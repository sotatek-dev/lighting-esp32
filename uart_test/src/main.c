#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>

#define LED_PIN 4
#define LED_FLAGS GPIO_OUTPUT_ACTIVE
#define UART_NODE DT_NODELABEL(uart0)

void main(void)
{
	const struct device *led_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	const struct device *uart_dev = DEVICE_DT_GET(UART_NODE);

	if (!device_is_ready(led_dev)) {
		printk("GPIO device not ready!\n");
		return;
	}

	if (!device_is_ready(uart_dev)) {
		printk("UART device not ready!\n");
		return;
	}

	gpio_pin_configure(led_dev, LED_PIN, LED_FLAGS);

	printk("UART LED Control Ready. Send '1' or '0'\n");

	while (1) {
		uint8_t c;
		if (uart_poll_in(uart_dev, &c) == 0) {
			if (c == '1') {
				gpio_pin_set(led_dev, LED_PIN, 1);
				printk("LED ON\n");
			} else if (c == '0') {
				gpio_pin_set(led_dev, LED_PIN, 0);
				printk("LED OFF\n");
			}
		}
		k_sleep(K_MSEC(10));
	}
}