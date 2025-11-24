/** Description:
This code defines a specialized audio component named `AudioOutputAlsa_int16`. Its primary function is to serve as a bridge between a high-level audio processing engine and the computer's sound hardware, specifically using the ALSA audio system commonly found on Linux.

**How it Works (The Basic Idea):**

This component is dedicated to handling digital audio playback in a very specific format: signed 16-bit integers.

1.  **Setup for Playback:** Upon initialization, it configures the underlying ALSA system for audio output (playback) and sets the exact data type the sound card should expect—a 16-bit integer format, which corresponds to standard CD-quality audio.

2.  **Sample Conversion:** Audio processing systems typically use high-resolution floating-point numbers (like -1.0 to 1.0) to represent volume. When the component receives a new audio sample (via the `sample` method), it performs a critical conversion:
    *   It first ensures the volume level is safe (clamping), preventing distortion or errors.
    *   It then converts the floating-point value into a discrete 16-bit integer. This conversion is necessary because sound cards understand integers, not floats.

3.  **Buffering and Delivery:** The newly converted 16-bit sample is immediately stored in an internal memory area (a buffer). If the audio is stereo, the sample is duplicated for the second channel. Once this buffer is completely filled, the entire batch of sound data is efficiently "flushed" (sent) to the ALSA driver, ensuring a continuous and smooth flow of sound to the speakers.

In essence, this class is a specialized audio adapter, translating the high-precision internal audio signal into the standard 16-bit stream required by the hardware.

sha: a509cc11338ca7c8073dbfaf862f23156dcdcb367dfa3deaf77fda05252c6b79 
*/
#pragma once
#include "AudioAlsa.h"
#include "helpers/clamp.h"

class AudioOutputAlsa_int16 : public AudioAlsa {
public:
    AudioOutputAlsa_int16(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : AudioAlsa(props, config, SND_PCM_STREAM_PLAYBACK)
    {
        open(SND_PCM_FORMAT_S16_LE);
    }

    void sample(float* buf) override
    {
        if (!handle)
            return;

        // clamp and convert float → int16
        float v = CLAMP(buf[track], -1.0f, 1.0f);
        int16_t v16 = static_cast<int16_t>(v * 32767.0f);
        reinterpret_cast<int16_t*>(buffer.data())[sampleIndex++] = v16;

        if (channels == CHANNEL_STEREO) {
            reinterpret_cast<int16_t*>(buffer.data())[sampleIndex++] = v16;
        }

        const uint32_t samplesPerChunk = chunkFrames * channels;
        if (sampleIndex >= samplesPerChunk) {
            flushBuffer(buffer.data(), chunkFrames);
        }
    }

protected:
    void resizeBuffer() override
    {
        buffer.resize(chunkFrames * channels * sizeof(int16_t));
    }
};
