/*
    Test SPI 2 and SPI 3
    Define the SPI host to use SPI2_HOST or SPI3_HOST
    This is required to have one jumper wire connected from MISO to MOSI
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "stdio.h"
#include "string.h"

static const char *TAG = "SPI_LOOP";
//#define HSPI_HOST SPI2_HOST  // Using HSPI (SPI2) for loopback
#define HSPI_HOST SPI3_HOST  // Using HSPI (SPI3) for loopback
void app_main(void)
{
    // 1. Configure bus SPI
    spi_bus_config_t buscfg = {
        .miso_io_num = 6,
        .mosi_io_num = 7,
        .sclk_io_num = 18,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &buscfg, 0));

    // 2. Configure device SPI (slave)
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000,  // 1 MHz
        .mode = 0,                           // SPI mode 0
        .spics_io_num = 5,                  // CS pin
        .queue_size = 1,
    };
    spi_device_handle_t handle;
    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &devcfg, &handle));

    // 3. Prepare to send data
    uint8_t tx_data[4] = {0xA5, 0x5A, 0xFF, 0x00};
    uint8_t rx_data[4] = {0};

    spi_transaction_t t = {
        .length = 4 * 8,      // number of bit for 4 bytes
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    // 4. Send and receive
    ESP_LOGI(TAG, "Gửi: 0x%02X 0x%02X 0x%02X 0x%02X",
             tx_data[0], tx_data[1], tx_data[2], tx_data[3]);
    ESP_ERROR_CHECK(spi_device_polling_transmit(handle, &t));

    // 5. Check the result
    ESP_LOGI(TAG, "Nhận: 0x%02X 0x%02X 0x%02X 0x%02X",
             rx_data[0], rx_data[1], rx_data[2], rx_data[3]);

    if (memcmp(tx_data, rx_data, sizeof(tx_data)) == 0) {
        ESP_LOGI(TAG, ">> SPI loopback successfully! <<");
    } else {
        ESP_LOGE(TAG, ">> SPI loopback failed. <<");
    }

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
