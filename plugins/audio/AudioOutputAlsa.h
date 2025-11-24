/** Description:
This code defines a specialized component called `AudioOutputAlsa`. Its primary job is to manage the playback of sound through the ALSA system, which is the standard way Linux handles audio hardware.

This class acts as an adapter, translating generic audio data into the specific format required by the sound card hardware.

When the component is first set up, it automatically configures itself for **output** (playing sound) and specifies that all incoming audio data must be handled in a high-quality, floating-point format.

The most critical function manages the flow of sound data. It receives individual, tiny sound samples one by one. Instead of sending each piece immediately—which would cause performance issues—the class acts as a collector. It quickly stores these incoming samples into a temporary internal container (a buffer), ensuring correct handling for both mono and stereo channels.

Once this buffer is completely full, the component sends the entire chunk of collected audio data simultaneously to the sound card. This strategy ensures efficient, smooth, and continuous playback without interruptions or stuttering.

sha: 72582a818bd2359ee3991d7fd9ac17513c3a49dc73803d8be5fd69fcb326dd47 
*/
#pragma once
#include "AudioAlsa.h"

class AudioOutputAlsa : public AudioAlsa {
public:
    AudioOutputAlsa(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : AudioAlsa(props, config, SND_PCM_STREAM_PLAYBACK)
    {
        open(SND_PCM_FORMAT_FLOAT);
    }

    void sample(float* buf) override
    {
        if (!handle)
            return;

        // append one sample
        float v = *buf;
        reinterpret_cast<float*>(buffer.data())[sampleIndex++] = v;

        if (channels == CHANNEL_STEREO) {
            reinterpret_cast<float*>(buffer.data())[sampleIndex++] = v;
        }

        const uint32_t samplesPerChunk = chunkFrames * channels;
        if (sampleIndex >= samplesPerChunk) {
            flushBuffer(buffer.data(), chunkFrames);
        }
    }

protected:
    void resizeBuffer() override
    {
        buffer.resize(chunkFrames * channels * sizeof(float));
    }
};
