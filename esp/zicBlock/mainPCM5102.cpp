#include "driver/i2s_std.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>

static const char* TAG = "I2S_STD";

#define I2S_PORT I2S_NUM_0

extern "C" void app_main(void)
{
    esp_err_t ret;

    // 1. Create new TX channel config
    i2s_chan_handle_t tx_chan;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_PORT, I2S_ROLE_MASTER);
    ret = i2s_new_channel(&chan_cfg, &tx_chan, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2S channel: %d", ret);
        return;
    }

    // 2. Standard (Philips I²S) slot & clock config
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100),
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

    // 4. Generate simple sine wave
    const int sample_rate = 44100;
    const int freq = 440;
    const int buf_len = 256;
    int16_t samples[buf_len * 2]; // stereo

    float volume = 0.2f;
    float mod = 0.0f;
    float dir = 1.0f;
    while (true) {
        for (int i = 0; i < buf_len; i++) {
            float s = sinf(2 * M_PI * freq * mod * i / sample_rate);
            int16_t val = (int16_t)(s * 32767 * 0.5);
            samples[2 * i] = val * mod * volume;
            samples[2 * i + 1] = val * mod * volume;
            mod += 0.00001f * dir;
            if (mod > 1.00f) {
                dir = -1.0f;
            } else if (mod < 0.00f) {
                dir = 1.0f;
            }
        }
        size_t bytes_written;
        i2s_channel_write(tx_chan, samples, sizeof(samples), &bytes_written, portMAX_DELAY);
    }
}
