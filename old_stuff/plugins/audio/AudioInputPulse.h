/** Description:
This code defines a specialized component, `AudioInputPulse`, designed to capture live audio input, typically from a microphone or line-in, using the **PulseAudio** sound system common in Linux environments.

### 1. Purpose and Role

The primary function of `AudioInputPulse` is to act as an audio recorder, or "source." It takes audio data originating from the operating system and makes it available to the main application’s audio pipeline. Because it captures sound rather than plays it, the system explicitly recognizes it as an audio input component.

### 2. How it Works

The component establishes a direct, streamlined connection to the PulseAudio server upon creation. This connection is configured specifically for recording.

1.  **Device Setup:** It attempts to open the audio input device. If the user specifies a particular device name (using the optional `DEVICE: name` setting), it uses that input; otherwise, it defaults to the system’s primary input source (like the default microphone).
2.  **Chunk Reading:** To ensure efficient performance, the component does not read audio one tiny sample at a time. Instead, it reads audio in fixed, small batches, or "chunks," storing them temporarily in an internal buffer.
3.  **Sample Delivery:** The main process iterates through this buffer, delivering one audio sample at a time to the rest of the application. Once the internal buffer is depleted, it automatically reaches out to PulseAudio to fetch the next fresh chunk of sound data. It also correctly handles whether the incoming sound is mono (single track) or stereo (two tracks).

sha: 5c861e5574d8549f62644e7d70197dfd6c366c4937ae1ebb1d9c1dfbaba8d71f 
*/
#pragma once

#include "AudioPulse.h"

/*md
## AudioInputPulse

AudioInputPulse plugin is used to read audio input from PulseAudio.

**Value**:
- `DEVICE: name` to set input device name. If not defined, default device will be used.
*/
class AudioInputPulse : public AudioPulse {
protected:
    pa_simple* newDevice(const pa_sample_spec streamFormat)
    {
        pa_buffer_attr bufferAttr;
        bufferAttr.fragsize = audioChunk * sizeof(float);
        bufferAttr.maxlength = -1;

        return pa_simple_new(NULL, NULL, PA_STREAM_RECORD, deviceName, "zicAudioInputPulse", &streamFormat, NULL, &bufferAttr, NULL);
    }

public:
    AudioInputPulse(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : AudioPulse(props, config)
    {
        open();
    }

    void sample(float* buf)
    {
        if (bufferIndex >= audioChunk) {
            bufferIndex = 0;
            if (device) {
                pa_simple_read(device, buffer, bufferReadSize, NULL);
            }
        }
        buf[track] = buffer[bufferIndex++];
        if (props.channels == CHANNEL_STEREO) {
            bufferIndex++; // skip stereo
        }
    }

    bool isSink() override
    {
        return false;
    }
};
