#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h> /* Required for  I2C */
#include "rtdb.h"
#include "buttons.h"

#define SUCCESS 0
#define ERR_FATAL -1   /* If I2C fails ...*/

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
#define led_thread_period 1000
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
#define FET_NODE DT_ALIAS(fetpin)
static const struct gpio_dt_spec fet = GPIO_DT_SPEC_GET(FET_NODE, gpios);


/* ---------- Semaphores ---------- */
/*  - For executing the PID controller on the new value after a sensor read  */
struct k_sem sensor_to_controller_sem = Z_SEM_INITIALIZER(sensor_to_controller_sem, 0, 1);
/*  - For executing the heat control based on the on/off value from the PID  */
struct k_sem controller_to_heater_sem = Z_SEM_INITIALIZER(controller_to_heater_sem, 0, 1);



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
    } else {
        printk("I2C bus %s, device address %x ready\n\r",dev_i2c.bus->name, dev_i2c.addr);
    } 

    /* Write (command RTR) to set the read address to temperature */
    ret = i2c_write_dt(&dev_i2c, TC74_CMD_RTR, 1);
    if(ret != 0){
        printk("Failed to write to I2C device at address %x, register %x \n\r", dev_i2c.addr ,TC74_CMD_RTR);
    }

    while (1) {
        /*  Wait for timer event  */
        k_timer_status_sync(&temp_read_thread_timer);

        /* Read temperature register */       
        ret = i2c_read_dt(&dev_i2c, &temp, sizeof(temp));

        rtdb_set_current_temp((int8_t)temp);

        uint64_t time_ms = k_uptime_get();
        uint32_t time_s = time_ms / 1000;
        uint32_t time_ms_remainder = time_ms % 1000;

        printk("Read temperature: %d at time %u.%03u s\n\r", temp, time_s, time_ms_remainder);

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
    int heater_state = 0;

    float integral = 0.0f;
    float last_error = 0.0f;
    
    const float dt = 0.25f; // time period is similar to the thread cycle of read_temperature_task BUT NOT EQUAL

    while (1) {
        // Wait for new sensor value
        k_sem_take(&sensor_to_controller_sem, K_FOREVER);

        // Only control if system is on
        if (!rtdb_get_system_on()) {
            if (heater_state != 0) {
                heater_state = 0;
                printk("System off, heater off\n\r");
                gpio_pin_set_dt(&fet, 0);
            }
            continue;
        }

        // Read current and desired temperatures from RTDB
        float current_temp = (float)rtdb_get_current_temp();
        float desired_temp = (float)rtdb_get_desired_temp();

        float output = pid_calculate(desired_temp, current_temp, dt, &last_error, &integral);

        // Conversion
        heater_state = (output > 0.0f) ? 1 : 0;

        rtdb_set_heat_on(heater_state);

        gpio_pin_set_dt(&fet, heater_state);
        
        printk("PID decided heater state: %d (Current: %d°C, Desired: %d°C)\n\r", 
               heater_state, (int)current_temp, (int)desired_temp);

        //  Tell the heater control to start working with this new value
        k_sem_give(&controller_to_heater_sem);
    }
}
K_THREAD_DEFINE(pid_task_id, 1024, pid_controller_task, NULL, NULL, NULL, 5, 0, 0);



void heat_control_task(void) {
    // Initialize PID
    int last_heat_state = 0;

    while (1) {
        // Wait for new sensor value
        k_sem_take(&controller_to_heater_sem, K_FOREVER);

        // Only control if system is on
        if (!rtdb_get_system_on()) {
            if (heater_state != 0) {
                heater_state = 0;
                printk("System off, heater off\n\r");
                gpio_pin_set_dt(&fet, 0);
            }
            continue;
        }

        // Read current and desired temperatures from RTDB
        float current_temp = (float)rtdb_get_current_temp();
        float desired_temp = (float)rtdb_get_desired_temp();

        float output = pid_calculate(desired_temp, current_temp, dt, &last_error, &integral);

        // Conversion
        heater_state = (output > 0.0f) ? 1 : 0;

        rtdb_set_heat_on(heater_state);

        gpio_pin_set_dt(&fet, heater_state);
        
        printk("PID decided heater state: %d (Current: %d°C, Desired: %d°C)\n\r", 
               heater_state, (int)current_temp, (int)desired_temp);
    }
}
K_THREAD_DEFINE(heat_task_id, 1024, heat_control_task, NULL, NULL, NULL, 5, 0, 0);


int main(void) {
    //  Setup LEDs
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
    // Setup Heater FET
    gpio_pin_configure_dt(&fet, GPIO_OUTPUT_INACTIVE);
        

    rtdb_init();
    buttons_init();

    return SUCCESS;
}
