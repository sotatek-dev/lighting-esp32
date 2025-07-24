#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED_PIN 4
#define SLEEP_TIME_MS 500

void main(void)
{
	const struct device *gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (!device_is_ready(gpio_dev)) {
		printk("GPIO device not ready!\n");
		return;
	}

	gpio_pin_configure(gpio_dev, LED_PIN, GPIO_OUTPUT_ACTIVE);

	while (1) {
		gpio_pin_toggle(gpio_dev, LED_PIN);
		k_msleep(SLEEP_TIME_MS);
	}
}
