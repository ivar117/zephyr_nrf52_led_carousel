#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>

#include <inttypes.h>

#define LEN(arr) (sizeof(arr) / sizeof(arr)[0])

#define BLINK_TIME 500 /* LED blink time */

static const struct gpio_dt_spec gpio_buttons[] = {
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw1), gpios, {0}),
};

static struct gpio_dt_spec gpio_leds[] = {
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led3), gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios, {0}),
};

static struct gpio_callback button_cb_data;

void
button_pressed(const struct device *dev, struct gpio_callback *cb,
              uint32_t pins)
{
    if (pins & BIT(gpio_buttons[0].pin)) {
        printk("Button 1 pressed\n");
    }
    else {
        printk("Button 2 pressed\n");
    }
}

int
main(void)
{
    int ret;
    int i;

    // Go through each button and perform checks on them.
    for (int i = 0; i < LEN(gpio_buttons); i++) {
        if (!gpio_is_ready_dt(&gpio_buttons[i])) {
            printk("Error: button device %s is not ready\n",
                gpio_buttons[i].port->name);
            return 0;
        }

        ret = gpio_pin_configure_dt(&gpio_buttons[i], GPIO_INPUT);
        if (ret != 0) {
            printk("Error %d: failed to configure %s pin %d\n",
                ret, gpio_buttons[i].port->name, gpio_buttons[i].pin);
            return 0;
        }

        ret = gpio_pin_interrupt_configure_dt(&gpio_buttons[i],
                        GPIO_INT_EDGE_TO_ACTIVE);
        if (ret != 0) {
            printk("Error %d: failed to configure interrupt on %s pin %d\n",
                ret, gpio_buttons[i].port->name, gpio_buttons[i].pin);
            return 0;
        }
    }

    gpio_init_callback(&button_cb_data, button_pressed,
               BIT(gpio_buttons[0].pin) | BIT(gpio_buttons[1].pin));

    // Go through each LED and perform checks on them.
    for (int i = 0; i < LEN(gpio_leds); i++) {
        if (gpio_leds[i].port && !gpio_is_ready_dt(&gpio_leds[i])) {
            printk("Error %d: LED device %s is not ready; ignoring it\n",
                ret, gpio_leds[i].port->name);
            gpio_leds[i].port = NULL;
	    }

        if (gpio_leds[i].port) {
            ret = gpio_pin_configure_dt(&gpio_leds[i], GPIO_OUTPUT);
            if (ret != 0) {
                printk("Error %d: failed to configure LED device %s pin %d\n",
                    ret, gpio_leds[i].port->name, gpio_leds[i].pin);
                gpio_leds[i].port = NULL;
            } else {
                printk("Set up LED at %s pin %d\n", gpio_leds[i].port->name, gpio_leds[i].pin);
            }
        }
    }

    // Continuously cycle through and turn on/off the LEDs.
    for (;;) {
        for (int i = 0; i < LEN(gpio_leds); i++) {
            gpio_pin_set_dt(&gpio_leds[i], 1);
            k_msleep(BLINK_TIME);
            gpio_pin_set_dt(&gpio_leds[i], 0);
        }
    }

    return 0;
}