// g++ -o alsa_test alsa_test.cpp -lasound

// #include <alsa/asoundlib.h>
// #include <cmath>
// #include <cstdio>
// #include <cstdlib>
// #include <cstring>
// #include <unistd.h>

// #define SAMPLE_RATE 44100
// #define CHANNELS 2
// #define FREQUENCY 220  // A4 note
// #define DURATION 5     // Duration in seconds

// void generateSineWave(float* buffer, size_t samples) {
//     for (size_t i = 0; i < samples; ++i) {
//         double t = (double)i / SAMPLE_RATE;
//         float sample = sin(2 * M_PI * FREQUENCY * t);
//         for (int ch = 0; ch < CHANNELS; ++ch) {
//             buffer[i * CHANNELS + ch] = sample;
//         }
//     }
// }

// int main() {
//     snd_pcm_t* handle;
//     snd_pcm_hw_params_t* params;
//     snd_pcm_uframes_t frames;
//     int err;

//     // Open PCM device for playback
//     if ((err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
//         fprintf(stderr, "Playback open error: %s\n", snd_strerror(err));
//         return 1;
//     }

//     // Allocate a hardware parameters object
//     snd_pcm_hw_params_alloca(&params);

//     // Fill it in with default values
//     snd_pcm_hw_params_any(handle, params);

//     // Set the desired hardware parameters
//     snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
//     snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_FLOAT);
//     snd_pcm_hw_params_set_channels(handle, params, CHANNELS);
//     unsigned int rate = SAMPLE_RATE;
//     snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0);

//     // Write the parameters to the driver
//     if ((err = snd_pcm_hw_params(handle, params)) < 0) {
//         fprintf(stderr, "Setting hwparams error: %s\n", snd_strerror(err));
//         snd_pcm_close(handle);
//         return 1;
//     }

//     // Get the buffer size
//     snd_pcm_hw_params_get_period_size(params, &frames, 0);
//     size_t buffer_size = frames * CHANNELS * sizeof(float);
//     float* buffer = (float*)malloc(buffer_size);

//     // Generate sine wave
//     generateSineWave(buffer, frames);

//     // Play the sine wave
//     for (int i = 0; i < DURATION * SAMPLE_RATE / frames; ++i) {
//         if ((err = snd_pcm_writei(handle, buffer, frames)) != frames) {
//             fprintf(stderr, "Write to audio interface failed (%s)\n", snd_strerror(err));
//             break;
//         }
//     }

//     // Clean up
//     free(buffer);
//     snd_pcm_drain(handle);
//     snd_pcm_close(handle);

//     return 0;
// }


#include <alsa/asoundlib.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#define SAMPLE_RATE 44100
#define CHANNELS 2
#define FREQUENCY 220  // A4 note
#define DURATION 5     // Duration in seconds
#define AUDIO_CHUNK 128  // Fixed buffer size

void generateSineWave(float* buffer, size_t samples) {
    static double phase = 0.0;
    double phaseIncrement = 2.0 * M_PI * FREQUENCY / SAMPLE_RATE;
    for (size_t i = 0; i < samples; ++i) {
        float sample = sin(phase);
        phase += phaseIncrement;
        if (phase >= 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
        for (int ch = 0; ch < CHANNELS; ++ch) {
            buffer[i * CHANNELS + ch] = sample;
        }
    }
}

int main() {
    snd_pcm_t* handle;
    snd_pcm_hw_params_t* params;
    snd_pcm_uframes_t frames = AUDIO_CHUNK;
    int err;

    // Open PCM device for playback
    if ((err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf(stderr, "Playback open error: %s\n", snd_strerror(err));
        return 1;
    }

    // Allocate a hardware parameters object
    snd_pcm_hw_params_alloca(&params);

    // Fill it in with default values
    snd_pcm_hw_params_any(handle, params);

    // Set the desired hardware parameters
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_FLOAT);
    snd_pcm_hw_params_set_channels(handle, params, CHANNELS);
    unsigned int rate = 44100;
    snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0);

    // Write the parameters to the driver
    if ((err = snd_pcm_hw_params(handle, params)) < 0) {
        fprintf(stderr, "Setting hwparams error: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return 1;
    }

    // Fixed-size buffer
    float buffer[AUDIO_CHUNK * CHANNELS];

    // Play the sine wave
    for (int i = 0; i < DURATION * SAMPLE_RATE / AUDIO_CHUNK; ++i) {
        generateSineWave(buffer, AUDIO_CHUNK);
        if ((err = snd_pcm_writei(handle, buffer, AUDIO_CHUNK)) != AUDIO_CHUNK) {
            fprintf(stderr, "Write to audio interface failed (%s)\n", snd_strerror(err));
            break;
        }
    }

    // Clean up
    snd_pcm_drain(handle);
    snd_pcm_close(handle);

    return 0;
}