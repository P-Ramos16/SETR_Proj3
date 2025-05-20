#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

/*  Button and LED nodes  */
#define BTN1_NODE DT_ALIAS(sw0)
#define BTN2_NODE DT_ALIAS(sw1)
#define BTN4_NODE DT_ALIAS(sw3)
#define LED1_NODE DT_ALIAS(led0)
#define LED2_NODE DT_ALIAS(led1)
#define LED3_NODE DT_ALIAS(led2)
#define LED4_NODE DT_ALIAS(led3)

/*  GPIO specs  */
static const struct gpio_dt_spec btn1 = GPIO_DT_SPEC_GET(BTN1_NODE, gpios);
static const struct gpio_dt_spec btn2 = GPIO_DT_SPEC_GET(BTN2_NODE, gpios);
static const struct gpio_dt_spec btn4 = GPIO_DT_SPEC_GET(BTN4_NODE, gpios);

static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);
static const struct gpio_dt_spec led4 = GPIO_DT_SPEC_GET(LED4_NODE, gpios);

/*  Button callback structures  */
static struct gpio_callback cb_btn1, cb_btn2, cb_btn4;

/*  System state  */
static bool systemOn = false;
static int desiredTemp = 20; //  Start at 20°C
static int currentTemp = 20; //  TODO: replace with sensor

/* Update all LEDs based on system and temperature state */
void update_leds(void) {
    
    gpio_pin_set_dt(&led1, systemOn ? 1 : 0);

    if (!systemOn) {
        gpio_pin_set_dt(&led2, 0);
        gpio_pin_set_dt(&led3, 0);
        gpio_pin_set_dt(&led4, 0);
        return;
    }

    int diff = currentTemp - desiredTemp;
    gpio_pin_set_dt(&led2, (diff >= -2 && diff <= 2) ? 1 : 0);
    gpio_pin_set_dt(&led3, (diff < -2) ? 1 : 0);
    gpio_pin_set_dt(&led4, (diff > 2) ? 1 : 0);
}

/* Button callbacks */
void btn1_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    systemOn = !systemOn;
    printk("System turned %s\n", systemOn ? "ON" : "OFF");
    update_leds();
}

void btn2_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    if (systemOn) {
        desiredTemp++;
        printk("Desired temperature increased to %d\n", desiredTemp);
        update_leds();
    }
}

void btn4_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    if (systemOn) {
        desiredTemp--;
        printk("Desired temperature decreased to %d\n", desiredTemp);
        update_leds();
    }
}

int main(void) {
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led4, GPIO_OUTPUT_INACTIVE);

    gpio_pin_configure_dt(&btn1, GPIO_INPUT);
    gpio_pin_configure_dt(&btn2, GPIO_INPUT);
    gpio_pin_configure_dt(&btn4, GPIO_INPUT);

    gpio_pin_interrupt_configure_dt(&btn1, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure_dt(&btn2, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure_dt(&btn4, GPIO_INT_EDGE_TO_ACTIVE);

    gpio_init_callback(&cb_btn1, btn1_pressed, BIT(btn1.pin));
    gpio_init_callback(&cb_btn2, btn2_pressed, BIT(btn2.pin));
    gpio_init_callback(&cb_btn4, btn4_pressed, BIT(btn4.pin));

    gpio_add_callback(btn1.port, &cb_btn1);
    gpio_add_callback(btn2.port, &cb_btn2);
    gpio_add_callback(btn4.port, &cb_btn4);

    printk("System initialized with desired temp = 20°C\n");


    while (1) {
        k_sleep(K_SECONDS(5));
        // Simulate temp changes here, e.g.:
        // currentTemp += 1;
        // update_leds();
    }

    return 0;
}