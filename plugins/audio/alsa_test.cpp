/** Description:
This C++ program is essentially a simple digital audio player and synthesizer designed specifically for Linux systems. Its primary goal is to generate and play a pure musical tone for a set duration.

The code uses the **ALSA (Advanced Linux Sound Architecture)** library, which allows software to communicate directly with the computer's sound card.

**How the Code Works:**

1.  **Preparation:** The program defines the characteristics of the sound it will create: a duration of 5 seconds, stereo output (2 channels), and a frequency of 220 Hz (the musical note A3). It also specifies a high-quality sample rate (44,100 samples per second).

2.  **Audio Configuration (The Setup):** It first opens the computer's default audio device. Then, it meticulously configures the technical settings, telling the sound card exactly how the data will arrive—for instance, specifying that the data is continuous and formatted for stereo playback.

3.  **Sound Generation:** The program includes a mathematical function that calculates a smooth, repeating pattern known as a **sine wave**. This waveform is the digital representation of a pure, single-pitch tone. Instead of creating the entire 5 seconds of sound at once, it generates the wave data in small, continuous chunks.

4.  **Playback Loop:** The program enters a rapid loop where it calculates the next small chunk of sound data and immediately sends that data to the sound card. This process repeats hundreds of times per second, providing the continuous stream of information needed to produce the 5-second audible tone.

5.  **Cleanup:** Once the 5 seconds are complete, the program ensures the sound card finishes playing any data remaining in its buffer and cleanly closes the connection, freeing up the audio device for other programs.

sha: f8cd5ae080d581219fac588855e20a25ece263e2716e11364de92c258ee017b7 
*/
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