#pragma once

#include <Arduino.h>
#if __has_include("driver/i2s_legacy.h")
#include "driver/i2s_legacy.h"
#elif __has_include("driver/i2s.h")
#include "driver/i2s.h"
#endif

#include "../../zicApp.h"

// Single Digital Pin Audio Output (GP0 / GPIO 0 using PDM TX mode)
#define I2S_NUM         I2S_NUM_0
#define AUDIO_PIN       0

inline void initAudioESP32()
{
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_PDM),
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
        .bck_io_num = I2S_PIN_NO_CHANGE,
        .ws_io_num = I2S_PIN_NO_CHANGE,
        .data_out_num = AUDIO_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);
}

inline void audioTaskESP32(void* parameter)
{
    ZicApp* app = (ZicApp*)parameter;
    static int16_t buffer[512]; // 256 frames * 2 channels (static to prevent stack overflow)

    while (true) {
        uint32_t nowMs = millis();
        if (app) app->updateMidiClockTimeout(nowMs);

        while (Serial1.available()) {
            uint8_t b = Serial1.read();
            if (app) app->handleMidiByte(b, nowMs);
        }

        for (int i = 0; i < 256; ++i) {
            float sample = (app && app->isPlaying) ? app->engine.sample() : 0.0f;
            int16_t val = (int16_t)(std::clamp(sample, -1.0f, 1.0f) * 32767.0f);
            buffer[i * 2] = val;     // Left
            buffer[i * 2 + 1] = val; // Right
        }
        size_t bytesWritten;
        i2s_write(I2S_NUM, buffer, sizeof(buffer), &bytesWritten, portMAX_DELAY);
        vTaskDelay(1);
    }
}
