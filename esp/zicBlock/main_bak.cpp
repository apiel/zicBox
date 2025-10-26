#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_log.h"
#include "esp_timer.h"
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

// Framebuffer: 128 columns × 16 pages
uint8_t framebuffer[DISPLAY_PAGES][DISPLAY_WIDTH];

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

void sh1107_clear_buffer()
{
    memset(framebuffer, 0x00, sizeof(framebuffer));
}

void sh1107_update_display()
{
    for (uint8_t page = 0; page < DISPLAY_PAGES; page++) {
        sh1107_write_cmd(0xB0 | page); // Set page address
        sh1107_write_cmd(0x00); // Set lower column address
        sh1107_write_cmd(0x10); // Set higher column address

        for (uint8_t col = 0; col < DISPLAY_WIDTH; col++) {
            sh1107_write_data(framebuffer[page][col]);
        }
    }
}

void sh1107_draw_pixel(uint8_t x, uint8_t y, bool on)
{
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return;
    }

    uint8_t page = y / 8;
    uint8_t bit_position = y % 8;

    if (on) {
        framebuffer[page][x] |= (1 << bit_position);
    } else {
        framebuffer[page][x] &= ~(1 << bit_position);
    }
}

void render()
{
    // We might be able to optimize this!
    for (int i = 0; i < ui.width; i++) {
        for (int j = 0; j < ui.height; j++) {
            sh1107_draw_pixel(i, j, ui.draw.getPixel({ i, j }));
        }
    }
    sh1107_update_display();
}

// void render()
// {
//     // Direct memory copy instead of per-pixel set
//     memcpy(framebuffer, ui.draw.screenBuffer, sizeof(framebuffer));

//     // Send buffer to display
//     sh1107_update_display();
// }

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

// extern "C" void app_main()
// {
//     i2c_init();
//     sh1107_init();
//     sh1107_clear_buffer();
//     // sh1107_update_display();

//     const TickType_t interval = pdMS_TO_TICKS(80); // 80 ms
//     TickType_t lastWake = xTaskGetTickCount();

//     for (;;) {
//         if (ui.render()) {
//             render();
//         }
//         // Wait until next period and yield to other tasks (including idle)
//         vTaskDelayUntil(&lastWake, interval);
//     }
// }

void sh1107_draw_filled_square(uint8_t x0, uint8_t y0, uint8_t size)
{
    for (uint8_t x = x0; x < x0 + size && x < DISPLAY_WIDTH; x++) {
        for (uint8_t y = y0; y < y0 + size && y < DISPLAY_HEIGHT; y++) {
            sh1107_draw_pixel(x, y, true);
        }
    }
}

extern "C" void app_main()
{

    i2c_init();
    sh1107_init();
    sh1107_clear_buffer();

    uint8_t size = 10;

    // Draw 4 squares in corners
    sh1107_draw_filled_square(0, 0, size); // Top-left
    sh1107_draw_filled_square(DISPLAY_WIDTH - size, 0, size); // Top-right
    sh1107_draw_filled_square(0, DISPLAY_HEIGHT - size, size); // Bottom-left
    sh1107_draw_filled_square(DISPLAY_WIDTH - size, DISPLAY_HEIGHT - size, size); // Bottom-right

    for (int i = 0; i < 8; i++) {
        sh1107_draw_filled_square(i * 16, 64, 8);
    }

    // Update the display with the framebuffer content
    sh1107_update_display();

    ESP_LOGI(TAG, "SH1107 ready! Squares drawn in all 4 corners");
}