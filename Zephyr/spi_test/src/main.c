#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>

#define SPI_NODE  DT_NODELABEL(spi3)
#define LED_GPIO  DT_NODELABEL(gpio0)
#define LED_PIN   4

static const struct spi_config spi_cfg = {
    .frequency = 1000000,
    .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
    .slave = 0,
    .cs = NULL,
};

void main(void)
{
    const struct device *spi_dev = DEVICE_DT_GET(SPI_NODE);
    const struct device *gpio_dev = DEVICE_DT_GET(LED_GPIO);

    if (!device_is_ready(spi_dev) || !device_is_ready(gpio_dev)) {
        printk("SPI or GPIO device not ready!\n");
        return;
    }

    gpio_pin_configure(gpio_dev, LED_PIN, GPIO_OUTPUT_INACTIVE);

    uint8_t tx_buf[1];
    uint8_t rx_buf[1];

    struct spi_buf tx_bufs = {
        .buf = tx_buf,
        .len = sizeof(tx_buf)
    };
    struct spi_buf rx_bufs = {
        .buf = rx_buf,
        .len = sizeof(rx_buf)
    };
    struct spi_buf_set tx_set = { .buffers = &tx_bufs, .count = 1 };
    struct spi_buf_set rx_set = { .buffers = &rx_bufs, .count = 1 };

    bool led_state = false;

    while (1) {
        led_state = !led_state;
        tx_buf[0] = led_state ? 0x01 : 0x00;
        rx_buf[0] = 0xFF;  // reset buffer

        int ret = spi_transceive(spi_dev, &spi_cfg, &tx_set, &rx_set);

        if (ret == 0 && rx_buf[0] == tx_buf[0]) {
            gpio_pin_set(gpio_dev, LED_PIN, led_state);
            printk("LED %s (received 0x%02x)\n", led_state ? "ON" : "OFF", rx_buf[0]);
        } else {
            printk("SPI error or wrong data: received 0x%02x, expected 0x%02x\n", rx_buf[0], tx_buf[0]);
        }

        k_msleep(1000);
    }
}
