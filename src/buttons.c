#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include "rtdb.h"

/* GPIO config for buttons */
#define BTN1_NODE DT_ALIAS(sw0)
#define BTN2_NODE DT_ALIAS(sw1)
#define BTN4_NODE DT_ALIAS(sw3)

static const struct gpio_dt_spec btn1 = GPIO_DT_SPEC_GET(BTN1_NODE, gpios);
static const struct gpio_dt_spec btn2 = GPIO_DT_SPEC_GET(BTN2_NODE, gpios);
static const struct gpio_dt_spec btn4 = GPIO_DT_SPEC_GET(BTN4_NODE, gpios);

static struct gpio_callback cb_btn1, cb_btn2, cb_btn4;

static void btn1_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    rtdb_set_system_on(!rtdb_get_system_on());
}

static void btn2_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    if (rtdb_get_system_on())
        rtdb_set_desired_temp(rtdb_get_desired_temp() + 1);
}

static void btn4_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    if (rtdb_get_system_on())
        rtdb_set_desired_temp(rtdb_get_desired_temp() - 1);
}

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
