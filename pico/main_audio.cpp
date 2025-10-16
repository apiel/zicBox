#include <cmath>
#include <math.h>
#include <stdio.h>

#include "pico/audio_i2s.h"
#include "pico/stdlib.h"

// Configure pins
#define I2S_DATA_PIN 9
#define I2S_BCLK_PIN 10
#define I2S_LRCK_PIN 11

#define BUTTON_PIN 15
// #define BUTTON_PIN 29
#define LED_PIN 25 // Onboard LED

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

static audio_buffer_pool_t* pool;

const uint TOUCH_PIN = 14; // GPIO14
const uint TRIES = 10; // averaging

// // measure charge time in microloop counts (not exact us, but relative)
// static inline uint32_t measure_touch_once()
// {
//     // Drive low first to start from known state
//     gpio_set_dir(TOUCH_PIN, GPIO_OUT);
//     gpio_put(TOUCH_PIN, 0);
//     sleep_us(1);

//     // Charge: set output high for short time
//     gpio_put(TOUCH_PIN, 1);
//     sleep_us(1);

//     // Make pin input (hi-Z) so pad discharges through internal pull-down (or through finger)
//     gpio_set_dir(TOUCH_PIN, GPIO_IN);
//     gpio_pull_down(TOUCH_PIN); // use pulldown so input will go LOW after discharge; measure time to go HIGH

//     // Now measure how long until the pin actually reads 1 (charging through pad)
//     uint32_t start = time_us_32();
//     const uint32_t timeout_us = 5000;
//     while (time_us_32() - start < timeout_us) {
//         if (gpio_get(TOUCH_PIN)) {
//             return time_us_32() - start;
//         }
//     }
//     // timeout - return a large value
//     return timeout_us;
// }

// uint32_t measure_touch_average()
// {
//     uint64_t sum = 0;
//     for (uint i = 0; i < TRIES; ++i) {
//         sum += measure_touch_once();
//         sleep_ms(5);
//     }
//     return (uint32_t)(sum / TRIES);
// }

int main()
{
    stdio_init_all();

    // Initialize I2S
    pool = audio_new_producer_pool(&producer_format, 3, 256); // 3 buffers, 256 samples each
    bool ok = audio_i2s_setup(&audio_format, &i2s_config);
    if (!ok) {
        printf("I2S setup failed!\n");
        while (1)
            ;
    }
    audio_i2s_connect(pool);
    audio_i2s_set_enabled(true);

    // Init LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    // Init button with internal pull-up
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    // Generate test tone
    const float freq = 440.0f; // A4
    const float phase_inc = 2.0f * (float)M_PI * freq / audio_format.sample_freq;
    float phase = 0.0f;

    printf("Hello over USB!\n");

    // gpio_init(TOUCH_PIN);
    // // Measure baseline (no touch) — run a few readings to get baseline
    // uint32_t baseline = 0;
    // for (int i = 0; i < 20; ++i)
    //     baseline = (baseline + measure_touch_average()) / 2;
    // printf("Baseline: %u\n", baseline);

    bool led_state = false;
    while (true) {
        audio_buffer_t* buffer = take_audio_buffer(pool, true);
        int16_t* samples = (int16_t*)buffer->buffer->bytes;
        for (int i = 0; i < buffer->max_sample_count; i++) {
            int16_t sample = (int16_t)(32767 * sinf(phase));
            samples[2 * i + 0] = sample; // Left
            samples[2 * i + 1] = sample; // Right
            phase += phase_inc;
            if (phase >= 2.0f * (float)M_PI)
                phase -= 2.0f * (float)M_PI;
        }
        buffer->sample_count = buffer->max_sample_count;
        give_audio_buffer(pool, buffer);

        // Button is active LOW (pressed = 0)
        if (!gpio_get(BUTTON_PIN)) {
            // simple debounce delay
            sleep_ms(20);
            if (!gpio_get(BUTTON_PIN)) {
                led_state = !led_state;
                gpio_put(LED_PIN, led_state);
                printf("Button pressed\n");

                // wait until button released
                while (!gpio_get(BUTTON_PIN)) {
                    tight_loop_contents();
                }
            }
        }

        // uint32_t v = measure_touch_average();
        // // Simple threshold: touched if measurement > baseline + margin
        // bool touched = v > (baseline + 20); // tweak margin as needed
        // if (touched) {
        //     printf("TOUCHED t=%u\n", v);
        //     led_state = !led_state;
        //     gpio_put(LED_PIN, led_state);
        // }
    }
}
