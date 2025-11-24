/** Description:
This component, named `AudioInputAlsa`, is a specialized software plugin designed to handle incoming audio on Linux operating systems.

Its primary function is to read sound data, typically from an input source like a microphone or a line-in jack, using the Advanced Linux Sound Architecture (ALSA). ALSA is the standard system for managing audio hardware on many Linux platforms.

**How it Works:**
The plugin acts as a bridge between the main application and the sound hardware. Upon initialization, it is configured specifically for "capture" (input recording). You can optionally tell it which specific device to use (e.g., a custom USB microphone name); otherwise, it defaults to the system’s standard input.

To ensure smooth operation, the plugin reads audio efficiently in larger segments, storing them in a temporary local buffer. When the main application requests the "next sample" of sound, the plugin retrieves it from this buffer. If the local buffer is emptied, the plugin automatically fetches a new chunk of data directly from the sound card, ensuring continuous recording without interruptions.

It also includes necessary safeguards to detect and report errors instantly if the connection to the sound hardware is lost or if there are issues during the data transfer process.

sha: e75cfb5f45d978026d823869374a0c56aa409f331a6ad00633c380cf90b3ae98 
*/
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
