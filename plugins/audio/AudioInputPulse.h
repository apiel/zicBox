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
