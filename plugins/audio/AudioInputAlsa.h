#ifndef _AUDIO_INPUT_ALSA_H_
#define _AUDIO_INPUT_ALSA_H_

#include "AudioAlsa.h"

class AudioInputAlsa : public AudioAlsa {
protected:
    snd_pcm_stream_t stream()
    {
        return SND_PCM_STREAM_CAPTURE;
    }

public:
    AudioInputAlsa(AudioPlugin::Props& props, char* _name)
        : AudioAlsa(props, _name)
    {
        open();
    }

    void sample(float* buf)
    {
        if (bufferIndex >= audioChunk) {
            bufferIndex = 0;
            if (handle && snd_pcm_readi(handle, buffer, audioChunk) < 0) {
                // int err;
                // debug("Audio card read error: %s\n", snd_strerror(err));
                // should throw and exit!!
                return;
            }
        }
        buf[track] = buffer[bufferIndex++] / 2147483647.0f;
    }
};

#endif
