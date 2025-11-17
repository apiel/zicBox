#pragma once

#include "AudioPulse.h"

/*md
## AudioOutputPulse

AudioOutputPulse plugin is used to write audio output to PulseAudio.

**Value**:
- `DEVICE: name` to set output device name. If not defined, default device will be used.
*/
class AudioOutputPulse : public AudioPulse {
protected:
    pa_simple* newDevice(const pa_sample_spec streamFormat)
    {
        return pa_simple_new(NULL, NULL, PA_STREAM_PLAYBACK, deviceName, "zicAudioOutputPulse", &streamFormat, NULL, NULL, NULL);
    }

public:
    AudioOutputPulse(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : AudioPulse(props, config)
    {
        open();
    }

    void sample(float* buf)
    {
        if (bufferIndex >= audioChunk) {
            bufferIndex = 0;
            if (device) {
                pa_simple_write(device, buffer, bufferReadSize, NULL);
            }
        }
        // if (buf[track] < -1.0f || buf[track] > 1.0f) {
        //     printf("AudioOutputPulse out of range %f\n", buf[track]);
        // }
        buffer[bufferIndex++] = buf[track];
        if (props.channels == CHANNEL_STEREO) {
            buffer[bufferIndex++] = buf[track];
        }
    }
};
