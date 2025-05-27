#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include "rtdb.h"

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


/* LEDs */
#define LED1_NODE DT_ALIAS(led0)
#define LED2_NODE DT_ALIAS(led1)
#define LED3_NODE DT_ALIAS(led2)
#define LED4_NODE DT_ALIAS(led3)

static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);
static const struct gpio_dt_spec led4 = GPIO_DT_SPEC_GET(LED4_NODE, gpios);

extern void buttons_init(void);

void led_update_task(void) {
    while (1) {
        bool on = rtdb_get_system_on();
        int desired = rtdb_get_desired_temp();
        int current = rtdb_get_current_temp();  // simulate if needed

        gpio_pin_set_dt(&led1, on ? 1 : 0);

        if (!on) {
            gpio_pin_set_dt(&led2, 0);
            gpio_pin_set_dt(&led3, 0);
            gpio_pin_set_dt(&led4, 0);
        } else {
            int diff = current - desired;
            gpio_pin_set_dt(&led2, (diff >= -2 && diff <= 2) ? 1 : 0);
            gpio_pin_set_dt(&led3, (diff < -2) ? 1 : 0);
            gpio_pin_set_dt(&led4, (diff > 2) ? 1 : 0);
        }

        k_sleep(K_MSEC(100));  //  TODO: change to millis like arduino
    }
}

K_THREAD_DEFINE(led_task_id, 1024, led_update_task, NULL, NULL, NULL, 5, 0, 0);

int main(void) {
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led4, GPIO_OUTPUT_INACTIVE);

    rtdb_init();
    buttons_init();

    return 0;
}
