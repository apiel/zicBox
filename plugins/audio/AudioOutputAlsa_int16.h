#pragma once

#include "AudioAlsa.h"

class AudioOutputAlsa_int16 : public AudioAlsa {
public:
    AudioOutputAlsa_int16(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : AudioAlsa(props, config, SND_PCM_STREAM_PLAYBACK)
    {
        open(SND_PCM_FORMAT_S16_LE);
    }

    int16_t bufferInt16[audioChunk * ALSA_MAX_CHANNELS];
    void sample(float* buf) override
    {
        if (bufferIndex >= bufferSize) {
            bufferIndex = 0;
            if (handle) {
                snd_pcm_sframes_t count = snd_pcm_writei(handle, bufferInt16, audioChunk);
            }
        }
        // Convert the single float sample to a 16-bit integer
        bufferInt16[bufferIndex++] = static_cast<int16_t>(buf[track] * 32767.0f);
    }
};
