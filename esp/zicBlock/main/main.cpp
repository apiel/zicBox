#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio>
#include <cstring>

#include "uiManager.h"
#include <unistd.h>

#define TAG "SH1107"
#define I2C_BUS_NUM I2C_NUM_0
#define I2C_SCL 16
#define I2C_SDA 21
#define I2C_FREQ_HZ 400000
#define SH1107_ADDR 0x3C

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 128
#define DISPLAY_PAGES 16 // 128 / 8 = 16 pages

i2c_master_dev_handle_t sh1107_dev;
i2c_master_bus_handle_t i2c_bus;

UIManager ui;

void i2c_init()
{
    // Create I2C master bus
    i2c_master_bus_config_t bus_cfg = {};
    bus_cfg.i2c_port = I2C_BUS_NUM;
    bus_cfg.sda_io_num = (gpio_num_t)I2C_SDA;
    bus_cfg.scl_io_num = (gpio_num_t)I2C_SCL;
    bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_cfg.flags.enable_internal_pullup = 1;

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &i2c_bus));
    ESP_LOGI(TAG, "I2C bus created");

    // Add SH1107 device to bus
    i2c_device_config_t dev_cfg = {};
    dev_cfg.device_address = SH1107_ADDR;
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.scl_speed_hz = I2C_FREQ_HZ;

    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus, &dev_cfg, &sh1107_dev));
    ESP_LOGI(TAG, "SH1107 device added to I2C bus");
}

esp_err_t sh1107_write_cmd(uint8_t cmd)
{
    uint8_t buf[2] = { 0x00, cmd }; // 0x00 = command
    return i2c_master_transmit(sh1107_dev, buf, 2, 100);
}

esp_err_t sh1107_write_data(uint8_t data)
{
    uint8_t buf[2] = { 0x40, data }; // 0x40 = data
    return i2c_master_transmit(sh1107_dev, buf, 2, 100);
}

void sh1107_init()
{
    sh1107_write_cmd(0xAE); // Display OFF
    sh1107_write_cmd(0xDC); // Set display start line
    sh1107_write_cmd(0x00);
    sh1107_write_cmd(0x81); // Set contrast
    sh1107_write_cmd(0x80); // Mid contrast
    sh1107_write_cmd(0xA1); // Segment remap (A1 = column 127 mapped to SEG0)
    sh1107_write_cmd(0xC8); // COM scan direction (C8 = remapped mode, scan from COM[N-1] to COM0)
    sh1107_write_cmd(0xA8); // Set multiplex ratio
    sh1107_write_cmd(0x7F); // 128 MUX
    sh1107_write_cmd(0xD3); // Set display offset
    sh1107_write_cmd(0x00); // No offset
    sh1107_write_cmd(0xD5); // Set display clock
    sh1107_write_cmd(0x50);
    sh1107_write_cmd(0xD9); // Set pre-charge period
    sh1107_write_cmd(0x22);
    sh1107_write_cmd(0xDB); // Set VCOM deselect
    sh1107_write_cmd(0x35);
    sh1107_write_cmd(0xAD); // Set DC-DC
    sh1107_write_cmd(0x8A); // Enable DC-DC
    sh1107_write_cmd(0xA4); // Display follows RAM
    sh1107_write_cmd(0xA6); // Normal display (not inverted)
    sh1107_write_cmd(0xAF); // Display ON

    vTaskDelay(100 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "SH1107 initialized");
}

typedef uint8_t (*sh1107_data_callback_t)(uint8_t page, uint8_t col);

void sh1107_update_display(sh1107_data_callback_t callback)
{
    for (uint8_t page = 0; page < DISPLAY_PAGES; page++) {
        sh1107_write_cmd(0xB0 | page); // Set page address
        sh1107_write_cmd(0x00); // Set lower column address
        sh1107_write_cmd(0x10); // Set higher column address

        for (uint8_t col = 0; col < DISPLAY_WIDTH; col++) {
            uint8_t data = callback(page, col);
            sh1107_write_data(data);
        }
    }
}

uint8_t clear_cb(uint8_t, uint8_t) { return 0x00; }
uint8_t render_cb(uint8_t page, uint8_t col) { return ui.draw.screenBuffer[page * DISPLAY_WIDTH + col]; }

extern "C" void app_main()
{
    i2c_init();
    sh1107_init();
    sh1107_update_display(clear_cb);

    const TickType_t interval = pdMS_TO_TICKS(80); // 80 ms
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        if (ui.render()) {
            sh1107_update_display(render_cb);
        }
        // Wait until next period and yield to other tasks (including idle)
        vTaskDelayUntil(&lastWake, interval);
    }
}

// #include "esp_timer.h" // need esp_timer in CmakeLists.txt

// static uint64_t getTicks()
// {
//     return esp_timer_get_time() / 1000ULL; // ms since boot
// }

// extern "C" void app_main()
// {
//     i2c_init();
//     sh1107_init();
//     sh1107_clear_buffer();

//     const int ms = 80;
//     uint64_t lastUpdate = getTicks();
//     while (true) {
//         uint64_t now = getTicks();
//         if (now - lastUpdate > ms) {
//             lastUpdate = now;
//             // ESP_LOGI(TAG, "render %" PRIu64, lastUpdate);
//             if (ui.render()) {
//                 render();
//             }
//         }
//         vTaskDelay(pdMS_TO_TICKS(1));
//     }
// }
