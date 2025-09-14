#pragma once
#include "AudioAlsa.h"
#include "helpers/range.h"

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
        float v = CLAMP(*buf, -1.0f, 1.0f);
        reinterpret_cast<int16_t*>(buffer.data())[sampleIndex++] = static_cast<int16_t>(v * 32767.0f);

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
