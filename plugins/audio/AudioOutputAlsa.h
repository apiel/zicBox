#pragma once

#include "AudioAlsa.h"

/*md
## AudioOutputAlsa

AudioOutputAlsa plugin is used to write audio output to ALSA.

**Value**:
- `DEVICE: name` to set output device name. If not defined, default device will be used.
*/
class AudioOutputAlsa : public AudioAlsa {
public:
    AudioOutputAlsa(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : AudioAlsa(props, config, SND_PCM_STREAM_PLAYBACK)
    {
        open();
    }

    void sample(float* buf) override
    {
        if (bufferIndex >= bufferSize) {
            bufferIndex = 0;
            if (handle) {
                snd_pcm_sframes_t count = snd_pcm_writei(handle, buffer, audioChunk);
            }
        }
        buffer[bufferIndex++] = buf[track];
    }
};
