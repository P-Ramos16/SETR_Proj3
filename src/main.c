#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>   /* Required for  I2C */
#include <zephyr/drivers/uart.h>  /* for UART API*/
#include <zephyr/sys/printk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rtdb.h"
#include "buttons.h"
#include "modules/cmdproc.h"

#define SUCCESS 0
#define ERR_FATAL -1

/**
 * @file main.c
 * @brief Módulo principal do sistema de controlo de temperatura.
 *
 * Inicializa os LEDs e arranca a tarefa `led_update_task`, que atualiza os LEDs com base nos valores
 * armazenados na base de dados em tempo real (RTDB). Esta tarefa reflete o estado do sistema,
 * indicando se está ligado, se a temperatura está normal, ou se há desvios acima ou abaixo do desejado.
 *
 * Também chama a função `buttons_init()` para configurar os botões físicos.
 *
 * A lógica do sistema está distribuída entre múltiplas tarefas e protegida por mecanismos de sincronização.
 */


/* ---------- LEDs ---------- */
/*  - Definition  */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)
/*  - Thread Properties  */
#define led_thread_period 500
K_TIMER_DEFINE(led_thread_timer, NULL, NULL);
/*  - Specs  */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);


/* ---------- Temperature Sensor ---------- */
/*  - Definition  */
#define TC74_CMD_RTR 0x00   /* Read temperature command */
#define TC74_CMD_RWCR 0x01  /* Read/write configuration register */
/*  - I2C device vars  */
#define I2C0_NID DT_NODELABEL(tc74sensor)
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NID);
/*  - Thread Properties  */
#define temp_read_thread_period 250
K_TIMER_DEFINE(temp_read_thread_timer, NULL, NULL);


/* ---------- Heater (or LED) FET Control ---------- */
/*  - Definition  */
#define FET_NODE DT_ALIAS(fetpin)
/*  - Specs  */
static const struct gpio_dt_spec fet = GPIO_DT_SPEC_GET(FET_NODE, gpios);


/* ---------- UART Reader ---------- */
/*  - Device Setup  */
#define UART_NODE DT_NODELABEL(uart0)   /* UART0 node ID*/
/*  - Buffer Properties  */
#define RXBUF_SIZE 60
#define TXBUF_SIZE 60
#define MSG_BUF_SIZE 100 
#define RX_TIMEOUT 1000   /* Inactivity period after the instant when last char was received that triggers an rx event (in us) */
/*  - UART Config  */
const struct uart_config uart_cfg = {
		.baudrate = 115200,
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits = UART_CFG_STOP_BITS_1,
		.data_bits = UART_CFG_DATA_BITS_8,
		.flow_ctrl = UART_CFG_FLOW_CTRL_NONE
};
const struct device *uart_dev = DEVICE_DT_GET(UART_NODE);
static uint8_t rx_buf[RXBUF_SIZE];      /* RX buffer, to store received data */
static uint8_t rx_chars[RXBUF_SIZE];    /* chars actually received  */
volatile int uart_rxbuf_nchar=0;        /* Number of chars currently on the rx buffer */
/*  - Callback Setup  */
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data);
/*  - Thread Properties  */
#define uart_thread_period 100
K_TIMER_DEFINE(uart_thread_timer, NULL, NULL);


/* ---------- Semaphores ---------- */
/*  - For executing the PID controller on the new value after a sensor read  */
struct k_sem sensor_to_controller_sem = Z_SEM_INITIALIZER(sensor_to_controller_sem, 0, 1);
/*  - For executing the heat control based on the on/off value from the PID  */
struct k_sem controller_to_heater_sem = Z_SEM_INITIALIZER(controller_to_heater_sem, 0, 1);
/*  - For executing the command processor when a complete message is received  */
struct k_sem uart_full_message_sem = Z_SEM_INITIALIZER(uart_full_message_sem, 0, 1);


/* ---------- Other Usefull Vars ---------- */
bool verboseMode = false

;

void led_update_task(void) {
    k_timer_start(&led_thread_timer, K_MSEC(led_thread_period), K_MSEC(led_thread_period));

    while (1) {
        /*  Wait for timer event  */
        k_timer_status_sync(&led_thread_timer);

        bool on = rtdb_get_system_on();
        int desired = rtdb_get_desired_temp();
        int current = rtdb_get_current_temp();

        gpio_pin_set_dt(&led0, on ? 1 : 0);

        if (!on) {
            gpio_pin_set_dt(&led1, 0);
            gpio_pin_set_dt(&led2, 0);
            gpio_pin_set_dt(&led3, 0);
        } else {
            int diff = current - desired;
            gpio_pin_set_dt(&led1, (diff >= -2 && diff <= 2) ? 1 : 0);
            gpio_pin_set_dt(&led2, (diff < -2) ? 1 : 0);
            gpio_pin_set_dt(&led3, (diff > 2) ? 1 : 0);
        }
    }
}
K_THREAD_DEFINE(led_task_id, 1024, led_update_task, NULL, NULL, NULL, 5, 0, 0);



int read_temperature_task(void) {
    int ret = 0;
    uint8_t temp = 0; 

    k_timer_start(&temp_read_thread_timer, K_MSEC(temp_read_thread_period), K_MSEC(temp_read_thread_period));
    
    if (!device_is_ready(dev_i2c.bus)) {
	    printk("I2C bus %s is not ready!\n\r",dev_i2c.bus->name);
	    return ERR_FATAL;
    }

    /* Write (command RTR) to set the read address to temperature */
    ret = i2c_write_dt(&dev_i2c, TC74_CMD_RTR, 1);

    while (1) {
        /*  Wait for timer event  */
        k_timer_status_sync(&temp_read_thread_timer);

        /* Read temperature register */       
        ret = i2c_read_dt(&dev_i2c, &temp, sizeof(temp));

        rtdb_set_current_temp((int8_t)temp);

        uint64_t time_ms = k_uptime_get();
        uint32_t time_s = time_ms / 1000;
        uint32_t time_ms_remainder = time_ms % 1000;

        if (verboseMode) {
            printk("Read temperature: %d at time %u.%03u s\n\r", temp, time_s, time_ms_remainder);
        }

        //  Tell the PID controller to start working with this new value
        k_sem_give(&sensor_to_controller_sem);
    }
    
    return SUCCESS;
}
K_THREAD_DEFINE(temp_read_task_id, 1024, read_temperature_task, NULL, NULL, NULL, 5, 0, 0);



float pid_calculate(float setpoint, float measured, float dt, 
                    float *last_error, float *integral) {
    const float Kp = 2.0f;
    const float Ki = 0.1f;
    const float Kd = 0.05f;

    float error = setpoint - measured;

    // Proportional term
    float Pout = Kp * error;

    // Integral term with maximum and minimum values
    *integral += error * dt;
    if (*integral > 20.0f) *integral = 20.0f;
    if (*integral < -20.0f) *integral = -20.0f;
    float Iout = Ki * (*integral);

    // Derivative term
    float derivative = (error - *last_error) / dt;
    float Dout = Kd * derivative;

    *last_error = error;

    return Pout + Iout + Dout;
}

void pid_controller_task(void) {
    // Initialize PID
    float integral = 0.0f;
    float last_error = 0.0f;
    
    const float dt = 0.25f; // time period is similar to the thread cycle of read_temperature_task BUT NOT EQUAL

    while (1) {
        // Wait for new sensor value
        k_sem_take(&sensor_to_controller_sem, K_FOREVER);

        // Read current and desired temperatures from RTDB
        float current_temp = (float)rtdb_get_current_temp();
        float desired_temp = (float)rtdb_get_desired_temp();

        float output = pid_calculate(desired_temp, current_temp, dt, &last_error, &integral);

        // Conversion
        rtdb_set_heat_on((output > 0.0f) && rtdb_get_system_on());

        if (verboseMode) {
            printk("PID decided heater state: %s (Current: %d°C, Desired: %d°C)\n\r", 
                (output > 0.0f) ? "ON" : "OFF", (int)current_temp, (int)desired_temp);
        }

        //  Tell the heater control to start working with this new value
        k_sem_give(&controller_to_heater_sem);
    }
}
K_THREAD_DEFINE(pid_task_id, 1024, pid_controller_task, NULL, NULL, NULL, 5, 0, 0);



void heat_control_task(void) {
    // Initialize PID
    bool last_heat_state = false;
    bool heater_state = false;

    while (1) {
        // Wait for new PID on/off value
        k_sem_take(&controller_to_heater_sem, K_FOREVER);

        // Only control if system is on
        if (!rtdb_get_system_on()) {
            if (last_heat_state) {
                heater_state = false;
                gpio_pin_set_dt(&fet, 0);

                if (verboseMode) {
                    printk("System off, heater off\n\r");
                }
            }
            continue;
        }
        // Conversion
        heater_state = rtdb_get_heat_on();

        if (last_heat_state != heater_state){
            gpio_pin_set_dt(&fet, heater_state);
            if (verboseMode) {
                printk("Heater turned: %d\n\r", heater_state);
            }
        }

        last_heat_state = heater_state;
    }
}
K_THREAD_DEFINE(heat_task_id, 1024, heat_control_task, NULL, NULL, NULL, 5, 0, 0);



int uart_init(void) {
    int err=0; /* Generic error variable */
    uint8_t welcome_mesg[] = "\n\rUART COM: Hello user! Here is the list of possible commands:\n -> M: Set max temperature\n -> C: Get current temperature\n -> S: Set PID parameters\n\n"; 

    /* Check if uart device is open */
    if (!device_is_ready(uart_dev)) {
        printk("device_is_ready(uart) returned error! Aborting! \n\r");
        return ERR_FATAL;
    }

    /* Configure UART */
    err = uart_configure(uart_dev, &uart_cfg);
    if (err == -ENOSYS) { /* If invalid configuration */
        printk("uart_configure() error. Invalid configuration\n\r");
        return ERR_FATAL; 
    }

    /* Register callback */
    err = uart_callback_set(uart_dev, uart_cb, NULL);
    if (err) {
        printk("uart_callback_set() error. Error code:%d\n\r",err);
        return ERR_FATAL;
    }
		
    /* Enable data reception */
    err =  uart_rx_enable(uart_dev ,rx_buf,sizeof(rx_buf),RX_TIMEOUT);
    if (err) {
        printk("uart_rx_enable() error. Error code:%d\n\r",err);
        return ERR_FATAL;
    }

    /* Send a welcome message */ 
    printk("%s", welcome_mesg);

    return SUCCESS;
}

bool startingMessage = false;

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data) {
    int err;
    
    switch (evt->type) {
	
        case UART_TX_DONE:
            break;

    	case UART_TX_ABORTED:
	    	printk("UART_TX_ABORTED event \n\r");
		    break;
		
	    case UART_RX_RDY:
            // Process each received character
            for (int i = 0; i < evt->data.rx.len; i++) {
                uint8_t c = rx_buf[evt->data.rx.offset + i];
                
                // Start of new message
                if (c == '#') {
                    startingMessage = true;
                    uart_rxbuf_nchar = 0;  // Reset buffer index
                    rx_chars[uart_rxbuf_nchar++] = c;  // Store the start character
                    rxChar(c);
                    printk("%c", c);
                } 
                // In the middle of another message
                else if (startingMessage) {
                    // Only store if we're in a message
                    if (uart_rxbuf_nchar < (RXBUF_SIZE - 1)) {
                        rx_chars[uart_rxbuf_nchar++] = c;
                        rxChar(c);
                    } 
                    else {
                        // Buffer overflow - discard message
                        printk("Message too long, discarding\n");
                        startingMessage = false;
                    }
                    
                    printk("%c", c);

                    // Check for end character
                    if (c == '!') {
                        printk("\n");
                        rx_chars[uart_rxbuf_nchar] = '\0';  // Null-terminate
                        k_sem_give(&uart_full_message_sem);  // Notify processor
                        startingMessage = false;
                    }
                }
            }

		    break;

	    case UART_RX_BUF_REQUEST:
            printk("\n\rMessage too long, discarding\n");
            startingMessage = false;
		    break;

	    case UART_RX_BUF_RELEASED:
		    break;
		
	    case UART_RX_DISABLED: 
            /* When the RX_BUFF becomes full RX is disabled automaticaly.  */
            /* It must be re-enabled manually for continuous reception */
		    err =  uart_rx_enable(uart_dev ,rx_buf,sizeof(rx_buf),RX_TIMEOUT);
            if (err) {
                printk("uart_rx_enable() error. Error code:%d\n\r",err);
                exit(ERR_FATAL);                
            }
		    break;

	    case UART_RX_STOPPED:
		    printk("UART_RX_STOPPED event \n\r");
		    break;
		
	    default:
            printk("UART: unknown event \n\r");
		    break;
    }

}

int uart_command_task(void) {
    int err;
    uint8_t rep_mesg[MSG_BUF_SIZE];
	int len;
	unsigned char ans[256];

    while (1) {
        // Wait for new complete message
        k_sem_take(&uart_full_message_sem, K_FOREVER);

        sprintf(rep_mesg,"Final message: %s\n\r",rx_chars);    
        cmdProcessor();     
        getTxBuffer(ans, &len);
        printk("Response: ");    
        
        for (int i = 0; i < len; i++) {
            printf("%c", ans[i]);
        }
        printk("\n");
        
        err = uart_tx(uart_dev, rep_mesg, strlen(rep_mesg), SYS_FOREVER_MS);
        if (err) {
            printk("uart_tx() error. Error code:%d\n\r",err);
            return ERR_FATAL;
        }
    }
}
K_THREAD_DEFINE(uart_command_id, 1024, uart_command_task, NULL, NULL, NULL, 5, 0, 0);

int main(void) {
    //  Setup LEDs
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
    // Setup Heater FET
    gpio_pin_configure_dt(&fet, GPIO_OUTPUT_INACTIVE);
        

    uart_init();
    rtdb_init();
    buttons_init();

	/* Init UART RX and TX buffers */
	resetTxBuffer();
	resetRxBuffer();

    return SUCCESS;
}
