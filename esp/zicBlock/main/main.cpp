#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio>
#include <cstring>

#include "uiManager.h"
#include <unistd.h>

#define I2S_PORT I2S_NUM_0
i2s_chan_handle_t tx_chan;

#define TAG "SH1107"
#define I2C_BUS_NUM I2C_NUM_0
#define I2C_SCL 16
#define I2C_SDA 21
#define I2C_FREQ_HZ 400000
#define SH1107_ADDR 0x3C

i2c_master_dev_handle_t sh1107_dev;
i2c_master_bus_handle_t i2c_bus;

UIManager ui;

// #define DISPLAY_PAGES 16 // 128 / 8 = 16 pages
const u_int8_t DISPLAY_PAGES = ui.height / 8;

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

        for (uint8_t col = 0; col < ui.width; col++) {
            uint8_t data = callback(page, col);
            sh1107_write_data(data);
        }
    }
}

uint8_t clear_cb(uint8_t, uint8_t) { return 0x00; }
uint8_t render_cb(uint8_t page, uint8_t col) { return ui.draw.screenBuffer[page * ui.width + col]; }

void audio_task(void* arg)
{
    Audio& audio = Audio::get();
    const int num_channels = audio.channels;
    const int buffer_samples = 512; // similar to SDL "want.samples"
    const int total_samples = buffer_samples * num_channels;
    int16_t buffer[total_samples];

    ESP_LOGI(TAG, "Audio task started on core %d", xPortGetCoreID());

    while (true) {
        for (int i = 0; i < total_samples; i++) {
            float s = ui.audio.sample();
            buffer[i] = (int16_t)(s * 32767.0f);
        }

        size_t bytes_written = 0;
        esp_err_t err = i2s_channel_write(tx_chan, buffer, sizeof(buffer), &bytes_written, portMAX_DELAY);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "I2S write failed: %d", err);
        }
    }
}

// // audio_task: produce audio, write to I2S with blocking write
// void audio_task(void* arg)
// {
//     Audio& audio = Audio::get();
//     const int num_channels = audio.channels;
//     const int buffer_samples = 512; // sample frames per callback-like chunk
//     const int total_samples = buffer_samples * num_channels;
//     // allocate on heap to avoid large stack usage
//     int16_t* buffer = (int16_t*)heap_caps_malloc(sizeof(int16_t) * total_samples, MALLOC_CAP_DMA);
//     if (!buffer) {
//         ESP_LOGE(TAG, "Failed to allocate audio buffer");
//         vTaskDelete(NULL);
//         return;
//     }

//     ESP_LOGI(TAG, "audio_task running on core %d", xPortGetCoreID());

//     while (true) {
//         // fill buffer with generated samples
//         for (int i = 0; i < total_samples; ++i) {
//             // IMPORTANT: ui.audio.sample() must return float (single precision)
//             float s = ui.audio.sample(); // ensure this is float, not double
//             buffer[i] = (int16_t)(s * 32767.0f);
//         }

//         // Blocking write to I2S. This will yield the CPU while waiting for DMA to accept more data.
//         size_t bytes_written = 0;
//         esp_err_t err = i2s_channel_write(tx_chan, buffer, sizeof(int16_t) * total_samples, &bytes_written, portMAX_DELAY);
//         if (err != ESP_OK) {
//             ESP_LOGE(TAG, "i2s write error: %d", err);
//             // short delay so we don't spin hard on errors
//             vTaskDelay(pdMS_TO_TICKS(1));
//         }

//         // Safety yield: if your audio loop is very expensive, a tiny delay helps the watchdog/idle run.
//         // Keep this small: 0 or 1 ms. If you already block on i2s_channel_write(portMAX_DELAY), this can be optional.
//         taskYIELD(); // or vTaskDelay(pdMS_TO_TICKS(0));
//     }

//     // never reached
//     heap_caps_free(buffer);
//     vTaskDelete(NULL);
// }

void i2s_init()
{
    esp_err_t ret;

    // 1. Create new TX channel config
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_PORT, I2S_ROLE_MASTER);
    ret = i2s_new_channel(&chan_cfg, &tx_chan, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2S channel: %d", ret);
        return;
    }

    Audio& audio = Audio::get();

    // 2. Standard (Philips I²S) slot & clock config
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(audio.sampleRate),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
            I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = (gpio_num_t)13,
            .ws = (gpio_num_t)11,
            .dout = (gpio_num_t)12,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    // 3. Initialize and enable the TX channel
    ret = i2s_channel_init_std_mode(tx_chan, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2s_channel_init_std_mode failed: %d", ret);
        return;
    }
    i2s_channel_enable(tx_chan);

    ESP_LOGI(TAG, "I2S TX started");
}

#define GPIO_KEY GPIO_NUM_1
void gpio_key_init()
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE; // no interrupts
    io_conf.mode = GPIO_MODE_INPUT; // input mode
    io_conf.pin_bit_mask = 1ULL << GPIO_KEY; // GPIO1
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE; // enable internal pull-up
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    ESP_LOGI(TAG, "GPIO%d initialized as input with pull-up", GPIO_KEY);
}

void key_poll_task(void* arg)
{
    bool lastState = true; // HIGH = not pressed (because of pull-up)

    while (true) {
        bool current = gpio_get_level(GPIO_KEY);

        if (current != lastState) {
            lastState = current;
            if (current == 0) {
                ui.onKey(0, 29, 1);
                ESP_LOGI(TAG, "Key pressed");
            } else {
                ui.onKey(0, 29, 0);
                ESP_LOGI(TAG, "Key released");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // 10 ms polling/debounce
    }
}

extern "C" void app_main()
{
    i2c_init();
    sh1107_init();
    sh1107_update_display(clear_cb);

    i2s_init();
    // Start audio task on core 1
    xTaskCreatePinnedToCore(audio_task, "audio_task", 4096, NULL, 5, NULL, 1);

    gpio_key_init();
    xTaskCreatePinnedToCore(key_poll_task, "key_poll_task", 2048, NULL, 4, NULL, 0);

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
