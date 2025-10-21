#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/gpio.h"

extern "C" void app_main(void)
{
    printf("ESP32-S3 GPIO Test Program\n");

    // List of GPIOs to test
    int test_pins[] = {
        4, 5, 6, 7, 15, 16, 17, 18,
        8, 3, 46, 9, 10, 11, 12, 13,
        14, 21, 47, 48, 45, 0, 38, 39,
        40, 41, 42, 44, 43, 2, 1
    };
    const int num_pins = sizeof(test_pins) / sizeof(test_pins[0]);

    // Configure each GPIO as input with pull-up
    for (int i = 0; i < num_pins; i++) {
        gpio_reset_pin((gpio_num_t)test_pins[i]);
        gpio_set_direction((gpio_num_t)test_pins[i], GPIO_MODE_INPUT);
        gpio_pullup_en((gpio_num_t)test_pins[i]);
        gpio_pulldown_dis((gpio_num_t)test_pins[i]);
    }

    // Store previous states to detect changes
    int last_state[64] = {0}; // large enough for all pins
    for (int i = 0; i < num_pins; i++) {
        last_state[i] = gpio_get_level((gpio_num_t)test_pins[i]);
    }

    printf("Connect each GPIO to GND to test.\n\n");

    while (true) {
        for (int i = 0; i < num_pins; i++) {
            int level = gpio_get_level((gpio_num_t)test_pins[i]);
            if (level != last_state[i]) {
                if (level == 0) {
                    printf("GPIO %d connected (LOW)\n", test_pins[i]);
                } else {
                    printf("GPIO %d disconnected (HIGH)\n", test_pins[i]);
                }
                last_state[i] = level;
                fflush(stdout);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // 20 checks per second
    }
}
