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
