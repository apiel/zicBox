#ifndef _AUDIO_INPUT_ALSA_H_
#define _AUDIO_INPUT_ALSA_H_

#include "AudioAlsa.h"

/*md
## AudioOutputAlsa

AudioOutputAlsa plugin is used to write audio output to ALSA.

**Value**:
- `DEVICE: name` to set output device name. If not defined, default device will be used.
*/
class AudioOutputAlsa : public AudioAlsa {
protected:
    snd_pcm_stream_t stream()
    {
        return SND_PCM_STREAM_CAPTURE;
    }

public:
    AudioOutputAlsa(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : AudioAlsa(props, config)
    {
        open();
    }

    void sample(float* buf)
    {
        // if (bufferIndex >= audioChunk * props.channels) { // ??
        if (bufferIndex >= audioChunk) {
            bufferIndex = 0;
            if (handle) {
                snd_pcm_sframes_t count = snd_pcm_writei(handle, buffer, audioChunk);
                // debug("write %ld (%d)(%d)\n", count, buffer[0], audioChunk);
            }
        }
        buffer[bufferIndex++] = buf[track] * 2147483647.0f;
    }
};

#endif
