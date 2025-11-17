#pragma once

#include "AudioAlsa.h"

/*md
## AudioInputAlsa

AudioInputAlsa plugin is used to read audio input from ALSA.

**Value**:
- `DEVICE: name` to set input device name. If not defined, default device will be used.
*/
class AudioInputAlsa : public AudioAlsa {
public:
    AudioInputAlsa(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : AudioAlsa(props, config, SND_PCM_STREAM_CAPTURE)
    {
        open();
    }

    void sample(float* buf)
    {
        if (bufferIndex >= bufferSize) {
            bufferIndex = 0;
            int err;
            if (handle && (err = snd_pcm_readi(handle, buffer, audioChunk)) < 0) {
                logError("A problem occurred while reading from the audio card: %s", snd_strerror(err));
                return;
            }
        }
        buf[track] = buffer[bufferIndex++];
        if (props.channels == CHANNEL_STEREO) {
            bufferIndex++; // skip stereo
        }
    }
};
