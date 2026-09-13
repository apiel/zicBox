#pragma once

#include <Arduino.h>
#if __has_include("driver/i2s_legacy.h")
#include "driver/i2s_legacy.h"
#elif __has_include("driver/i2s.h")
#include "driver/i2s.h"
#endif

#include "../../zicApp.h"

// ESP32-S3 I2S Pin Definitions (Default configuration for PCM / I2S audio output)
#define I2S_NUM         I2S_NUM_0
#define I2S_BCK_PIN     1
#define I2S_WS_PIN      2
#define I2S_DATA_PIN    3

inline void initAudioESP32()
{
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = true
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_DATA_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);
}

inline void audioTaskESP32(void* parameter)
{
    ZicApp* app = (ZicApp*)parameter;
    int16_t buffer[512]; // 256 frames * 2 channels

    while (true) {
        for (int i = 0; i < 256; ++i) {
            float sample = app->renderMasterSample();
            int16_t val = (int16_t)(std::clamp(sample, -1.0f, 1.0f) * 32767.0f);
            buffer[i * 2] = val;     // Left
            buffer[i * 2 + 1] = val; // Right
        }
        size_t bytesWritten;
        i2s_write(I2S_NUM, buffer, sizeof(buffer), &bytesWritten, portMAX_DELAY);
        vTaskDelay(1);
    }
}
