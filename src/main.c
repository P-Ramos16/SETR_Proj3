/**
 * @file main.c
 * @brief Main system controller for temperature regulation
 *
 * This module initializes all system components including:
 * - LED indicators
 * - Temperature sensor (TC74 via I2C)
 * - Heater control (via FET)
 * - UART communication interface
 * - Real-Time Database (RTDB)
 * - Button inputs
 *
 * The system implements a PID controller for temperature regulation
 * with multiple threads handling different aspects of the control system.
 * \author Pedro Ramos, n.º 107348
 * \author Rafael Morgado, n.º 104277
 * \date 01/06/2025
 */

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

#include "modules/rtdb.h"
#include "modules/buttons.h"
#include "modules/PID.h"
#include "modules/cmdproc.h"

#define SUCCESS 0     /**< Operation successful return code */
#define ERR_FATAL -1  /**< Fatal error return code */



/* ---------- LED Configuration ---------- */
#define LED0_NODE DT_ALIAS(led0)  /**< Devicetree alias for LED0 */
#define LED1_NODE DT_ALIAS(led1)  /**< Devicetree alias for LED1 */
#define LED2_NODE DT_ALIAS(led2)  /**< Devicetree alias for LED2 */
#define LED3_NODE DT_ALIAS(led3)  /**< Devicetree alias for LED3 */

#define led_thread_period 500  /**< LED update period in milliseconds */
K_TIMER_DEFINE(led_thread_timer, NULL, NULL);  /**< Timer for LED thread */

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);  /**< LED0 GPIO specification */
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);  /**< LED1 GPIO specification */
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);  /**< LED2 GPIO specification */
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);  /**< LED3 GPIO specification */


/* ---------- Temperature Sensor Configuration ---------- */
#define TC74_CMD_RTR 0x00   /**< Read temperature command */
#define TC74_CMD_RWCR 0x01  /**< Read/write configuration register */

#define I2C0_NID DT_NODELABEL(tc74sensor)  /**< Devicetree node identifier for TC74 sensor */
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NID);  /**< I2C device specification */

#define temp_read_thread_period 250  /**< Temperature reading period in milliseconds */
K_TIMER_DEFINE(temp_read_thread_timer, NULL, NULL);  /**< Timer for temperature reading thread */


/* ---------- Heater Control Configuration ---------- */
#define FET_NODE DT_ALIAS(fetpin)  /**< Devicetree alias for FET control pin */
static const struct gpio_dt_spec fet = GPIO_DT_SPEC_GET(FET_NODE, gpios);  /**< FET GPIO specification */


/* ---------- UART Configuration ---------- */
#define UART_NODE DT_NODELABEL(uart0)  /**< Devicetree node identifier for UART0 */

#define RXBUF_SIZE 60      /**< UART receive buffer size */
#define TXBUF_SIZE 60      /**< UART transmit buffer size */
#define MSG_BUF_SIZE 100   /**< Complete message buffer size */
#define RX_TIMEOUT 1000    /**< UART receive timeout in microseconds */

/** UART configuration structure */
const struct uart_config uart_cfg = {
		.baudrate = 115200,
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits = UART_CFG_STOP_BITS_1,
		.data_bits = UART_CFG_DATA_BITS_8,
		.flow_ctrl = UART_CFG_FLOW_CTRL_NONE
};

const struct device *uart_dev = DEVICE_DT_GET(UART_NODE);  /**< UART device instance */
static uint8_t rx_buf[RXBUF_SIZE];      /**< UART receive buffer */
static uint8_t rx_chars[RXBUF_SIZE];    /**< Buffer for received characters */
volatile int uart_rxbuf_nchar = 0;      /**< Number of characters in receive buffer */

/*  - Callback Setup  */
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data);
/*  - Thread Properties  */
#define uart_thread_period 100
K_TIMER_DEFINE(uart_thread_timer, NULL, NULL);


/* ---------- Semaphores ---------- */
struct k_sem sensor_to_controller_sem = Z_SEM_INITIALIZER(sensor_to_controller_sem, 0, 1); /**< For executing the PID controller on the new value after a sensor read  */
struct k_sem controller_to_heater_sem = Z_SEM_INITIALIZER(controller_to_heater_sem, 0, 1); /**< For executing the heat control based on the on/off value from the PID  */
struct k_sem uart_full_message_sem = Z_SEM_INITIALIZER(uart_full_message_sem, 0, 1); /**< For executing the command processor when a complete message is received  */



/**
 * @brief LED update task
 *
 * This thread periodically updates the LED indicators based on system state:
 * - LED0: System power state (on/off)
 * - LED1: Temperature within desired range (±2°C)
 * - LED2: Temperature below desired range
 * - LED3: Temperature above desired range
 */
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


/**
 * @brief Temperature reading task
 *
 * This thread periodically reads the temperature from the TC74 sensor
 * and updates the RTDB with the current temperature value.
 *
 * @return int SUCCESS on success, ERR_FATAL on failure
 */
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

        bool verboseMode = rtdb_get_verbose();

        if (verboseMode) {
            printk("Read temperature: %d at time %u.%03u s\n\r", temp, time_s, time_ms_remainder);
        }

        //  Tell the PID controller to start working with this new value
        k_sem_give(&sensor_to_controller_sem);
    }
    
    return SUCCESS;
}
K_THREAD_DEFINE(temp_read_task_id, 1024, read_temperature_task, NULL, NULL, NULL, 5, 0, 0);


/**
 * @brief PID controller task.
 *
 * This thread waits for new sensor data from the temperature reading task
 * and calculates the PID output to decide whether the heater should be on.
 * It then signals the heat control task to work on the value saved on the
 * rtdb.
 */
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

        bool verboseMode = rtdb_get_verbose();

        if (verboseMode) {
            printk("PID decided heater state: %s (Current: %d°C, Desired: %d°C)\n\r", 
                (output > 0.0f) ? "ON" : "OFF", (int)current_temp, (int)desired_temp);
        }

        //  Tell the heater control to start working with this new value
        k_sem_give(&controller_to_heater_sem);
    }
}
K_THREAD_DEFINE(pid_task_id, 1024, pid_controller_task, NULL, NULL, NULL, 5, 0, 0);



/**
 * @brief Heater control task.
 *
 * This thread waits for the PID controller task to signal it and then 
 * toggles the heater state based on the PID output and system state.
 * It ensures the heater only operates when the system is on.
 */
void heat_control_task(void) {
    // Initialize PID
    bool last_heat_state = false;
    bool heater_state = false;

    while (1) {
        // Wait for new PID on/off value
        k_sem_take(&controller_to_heater_sem, K_FOREVER);

        bool verboseMode = rtdb_get_verbose();

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


/**
 * @brief Initializes the UART peripheral.
 *
 * This function initializes the UART device, sets its configuration,
 * registers the callback, enables data reception, and prints a welcome 
 * message with instructions for the user.
 *
 * @return int SUCCESS on success, ERR_FATAL on failure
 */
int uart_init(void) {
    int err=0; /* Generic error variable */
    uint8_t welcome_mesg[] = "\n\rUART COM: Hello user! Here is the list of possible commands:\n -> M (#M+30219!):   Set desired temperature\n -> D (#D068!):      Get desired temperature\n -> C (#C067!):      Get current temperature\n -> S (#Sp1.23135!): Set PID parameters\n -> V (#V086!):      Toggle verbose mode\n\r\n\r"; 

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
/**
 * @brief UART callback function.
 *
 * This callback handles various UART events, including TX done, RX ready, and buffer requests.
 * It accumulates incoming characters into a message buffer when a message starts with '#'
 * and ends with '!'. It also handles buffer overflow and restarts reception as needed.
 *
 * @param dev Pointer to the UART device structure.
 * @param evt Pointer to the UART event structure.
 * @param user_data Unused pointer to user data.
 */
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
                        uart_rxbuf_nchar = 0;
                        memset(rx_buf, 0, sizeof(rx_buf));
                    }
                }
            }

		    break;

	    case UART_RX_BUF_REQUEST:
            printk("\n\rERR: Message too long, discarding\n");
            startingMessage = false;
            uart_rxbuf_nchar = 0;
            memset(rx_buf, 0, sizeof(rx_buf));
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


/**
 * @brief UART command processing task.
 *
 * This thread waits for complete UART messages, processes the command, and sends
 * a response back via UART. It coordinates with the UART callback to handle incoming messages.
 *
 * @return int SUCCESS on success, ERR_FATAL on failure.
 */
int uart_command_task(void) {
    int err;
    uint8_t rep_mesg[MSG_BUF_SIZE];
	int len;
	unsigned char ans[64];

    while (1) {
		resetRxBuffer();
		resetTxBuffer();
        // Wait for new complete message
        k_sem_take(&uart_full_message_sem, K_FOREVER);

        cmdProcessor();     
        getTxBuffer(ans, &len);
        ans[len] = 0; /* Terminate the string */

        sprintf(rep_mesg,"Response: %s\n\r",ans);            
        
        err = uart_tx(uart_dev, rep_mesg, strlen(rep_mesg), SYS_FOREVER_MS);
        if (err) {
            printk("uart_tx() error. Error code:%d\n\r",err);
            return ERR_FATAL;
        }
    }
}
K_THREAD_DEFINE(uart_command_id, 1024, uart_command_task, NULL, NULL, NULL, 5, 0, 0);


/**
 * @brief Main function.
 *
 * Initializes system peripherals including LEDs, heater FET, UART, RTDB, and buttons.
 * Configures UART receive/transmit buffers and starts the system.
 *
 * @return int SUCCESS on successful initialization.
 */
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
