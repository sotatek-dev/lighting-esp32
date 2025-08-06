#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "GPIO_TEST";

// List of GPIO pins to test, these should be connected to LEDs to monitor output
static const gpio_num_t test_pins[] = {
    GPIO_NUM_10,
    GPIO_NUM_11,
    GPIO_NUM_12,
    GPIO_NUM_13,
    GPIO_NUM_1,
    GPIO_NUM_2,
    GPIO_NUM_20,
    GPIO_NUM_21,
    GPIO_NUM_35,
    GPIO_NUM_36,
    GPIO_NUM_37,
    GPIO_NUM_38,
    GPIO_NUM_39,
    GPIO_NUM_45,
    GPIO_NUM_46,
    GPIO_NUM_47,
    GPIO_NUM_48,
    GPIO_NUM_40,
    GPIO_NUM_41,
    GPIO_NUM_42
};
static const int PIN_COUNT = sizeof(test_pins) / sizeof(test_pins[0]);

void app_main(void)
{
    // 1. Reset and configure GPIO pins
    for (int i = 0; i < PIN_COUNT; i++) {
        gpio_num_t pin = test_pins[i];
        // Reset the pin to ensure it's in a known state
        gpio_reset_pin(pin);
        // Configure the pin as output, no pull-up or pull-down, no interrupt
        gpio_config_t io_conf = {
            .pin_bit_mask = 1ULL << pin,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        esp_err_t ret = gpio_config(&io_conf);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to configure GPIO%d (%d)", pin, ret);
        }
    }

    // 2. Set all test pins to HIGH
    for (int i = 0; i < PIN_COUNT; i++) {
        gpio_set_level(test_pins[i], 1);
    }
    ESP_LOGI(TAG, "All test pins set HIGH. Check your LEDs.");

    // 3. Keep for the program for running
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
