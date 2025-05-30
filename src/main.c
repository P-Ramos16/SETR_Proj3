#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include "rtdb.h"
#include "buttons.h"

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

/*  LEDs  */
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


/*  Temperature Sensor  */
/*  - Definition  */
#define TC74_CMD_RTR 0x00   /* Read temperature command */
#define TC74_CMD_RWCR 0x01  /* Read/write configuration register */
/*  - I2C device vars  */
#define I2C0_NID DT_NODELABEL(tc74sensor)
/*  - Thread Properties  */
#define temp_read_thread_period 250
K_TIMER_DEFINE(temp_read_thread_timer, NULL, NULL);

/*  Semaphores  */
/*  - For executing the PID controller on the new value after a sensor read  */
struct k_sem sensor_to_controller_sem = Z_SEM_INITIALIZER(sensor_to_controller_sem, 0, 1);



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



void read_temperature_task(void) {
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

        //  Tell the PID controller to start working with this new value
        k_sem_give(&sensor_to_controller_sem);
        //  TODO: for the controller -> k_sem_take(&sensor_to_controller_sem, K_FOREVER);  // Wait for signal
    }
}
K_THREAD_DEFINE(temp_read_task_id, 1024, read_temperature_task, NULL, NULL, NULL, 5, 0, 0);





int main(void) {
    //  Setup LEDs
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);

    rtdb_init();
    buttons_init();

    return 0;
}
