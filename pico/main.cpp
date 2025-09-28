#include <stdio.h>
#include <cmath>
#include <math.h>

#include "pico/stdlib.h"
#include "pico/audio_i2s.h"

// Configure pins
#define I2S_DATA_PIN 9
#define I2S_BCLK_PIN 10
#define I2S_LRCK_PIN 11

// Audio config: stereo, 44.1kHz, 16-bit
audio_format_t audio_format = {
    .sample_freq = 44100,
    .format = AUDIO_BUFFER_FORMAT_PCM_S16,
    .channel_count = 2
};

audio_buffer_format_t producer_format = {
    .format = &audio_format,
    .sample_stride = 4 // 16-bit * 2 channels
};

struct audio_i2s_config i2s_config = {
    .data_pin = I2S_DATA_PIN,
    .clock_pin_base = I2S_BCLK_PIN, // BCK=10, LRCK=11
    .dma_channel = 0,
    .pio_sm = 0
};

static audio_buffer_pool_t *pool;

int main() {
    stdio_init_all();

    // Initialize I2S
    pool = audio_new_producer_pool(&producer_format, 3, 256); // 3 buffers, 256 samples each
    bool ok = audio_i2s_setup(&audio_format, &i2s_config);
    if (!ok) {
        printf("I2S setup failed!\n");
        while (1);
    }
    audio_i2s_connect(pool);
    audio_i2s_set_enabled(true);

    // Generate test tone
    const float freq = 440.0f; // A4
    const float phase_inc = 2.0f * (float)M_PI * freq / audio_format.sample_freq;
    float phase = 0.0f;

    while (true) {
        audio_buffer_t *buffer = take_audio_buffer(pool, true);
        int16_t *samples = (int16_t *)buffer->buffer->bytes;
        for (int i = 0; i < buffer->max_sample_count; i++) {
            int16_t sample = (int16_t)(32767 * sinf(phase));
            samples[2 * i + 0] = sample; // Left
            samples[2 * i + 1] = sample; // Right
            phase += phase_inc;
            if (phase >= 2.0f * (float)M_PI) phase -= 2.0f * (float)M_PI;
        }
        buffer->sample_count = buffer->max_sample_count;
        give_audio_buffer(pool, buffer);
    }
}
