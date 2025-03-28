#pragma once

#include "AudioAlsa.h"

/*md
## AudioInputAlsa

AudioInputAlsa plugin is used to read audio input from ALSA.

**Value**:
- `DEVICE: name` to set input device name. If not defined, default device will be used.
*/
class AudioInputAlsa : public AudioAlsa {
protected:
    snd_pcm_stream_t stream()
    {
        return SND_PCM_STREAM_CAPTURE;
    }

public:
    AudioInputAlsa(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : AudioAlsa(props, config)
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
