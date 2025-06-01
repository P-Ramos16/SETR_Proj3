#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include "modules/rtdb.h"

/**
 * @file buttons.c
 * @brief Gestão dos botões físicos do sistema.
 *
 * Configura os botões BTN1, BTN2 e BTN4 para operar por interrupção. Cada botão invoca um callback:
 * - BTN1: Liga/desliga o sistema.
 * - BTN2: Aumenta a temperatura desejada.
 * - BTN4: Diminui a temperatura desejada.
 *
 * Cada ação manipula a base de dados em tempo real (RTDB), assegurando a sincronização entre tarefas.
 * \author Pedro Ramos, n.º 107348
 * \author Rafael Morgado, n.º 104277
 * \date 01/06/2025
 */


/* GPIO config for buttons */
#define BTN1_NODE DT_ALIAS(sw0)  /**< Devicetree alias for BTN1 */
#define BTN2_NODE DT_ALIAS(sw1)  /**< Devicetree alias for BTN2 */
#define BTN4_NODE DT_ALIAS(sw3)  /**< Devicetree alias for BTN4 */

static const struct gpio_dt_spec btn1 = GPIO_DT_SPEC_GET(BTN1_NODE, gpios);  /**< BTN1 GPIO specification */
static const struct gpio_dt_spec btn2 = GPIO_DT_SPEC_GET(BTN2_NODE, gpios);  /**< BTN2 GPIO specification */
static const struct gpio_dt_spec btn4 = GPIO_DT_SPEC_GET(BTN4_NODE, gpios);  /**< BTN4 GPIO specification */

static struct gpio_callback cb_btn1, cb_btn2, cb_btn4;

/**
 * @brief Callback for Button 1 press.
 *
 * Toggles the system on/off state.
 */
static void btn1_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    rtdb_set_system_on(!rtdb_get_system_on());
}


/**
 * @brief Callback for Button 2 press.
 *
 * Increases the desired temperature by 1 °C, if the system is on.
 */
static void btn2_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    if (rtdb_get_system_on())
        rtdb_set_desired_temp(rtdb_get_desired_temp() + 1);
}


/**
 * @brief Callback for Button 4 press.
 *
 * Decreases the desired temperature by 1 °C, if the system is on.
 */
static void btn4_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    if (rtdb_get_system_on())
        rtdb_set_desired_temp(rtdb_get_desired_temp() - 1);
}


/**
 * @brief Initialize buttons and their interrupts.
 */
void buttons_init(void) {
    gpio_pin_configure_dt(&btn1, GPIO_INPUT);
    gpio_pin_configure_dt(&btn2, GPIO_INPUT);
    gpio_pin_configure_dt(&btn4, GPIO_INPUT);

    gpio_pin_interrupt_configure_dt(&btn1, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure_dt(&btn2, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure_dt(&btn4, GPIO_INT_EDGE_TO_ACTIVE);

    gpio_init_callback(&cb_btn1, btn1_handler, BIT(btn1.pin));
    gpio_add_callback(btn1.port, &cb_btn1);

    gpio_init_callback(&cb_btn2, btn2_handler, BIT(btn2.pin));
    gpio_add_callback(btn2.port, &cb_btn2);

    gpio_init_callback(&cb_btn4, btn4_handler, BIT(btn4.pin));
    gpio_add_callback(btn4.port, &cb_btn4);
}
