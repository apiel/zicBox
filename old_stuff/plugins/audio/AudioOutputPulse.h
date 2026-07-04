/** Description:
This header defines a critical component called `AudioOutputPulse`. Its main responsibility is to act as the bridge between internal audio processing logic and the computer’s sound hardware, specifically utilizing the PulseAudio sound server, which is commonly found in Linux systems.

**Primary Goal:**
The module takes finalized digital audio data—the raw numbers representing sound waves—and outputs them so they can be heard through your speakers or headphones. It ensures that the digital sound data is correctly formatted and delivered to the operating system's sound system for playback.

**How It Works:**
1.  **Connection:** When the module starts, it establishes a dedicated connection to the PulseAudio server specifically for outputting sound.
2.  **Buffering:** It collects incoming audio data into a temporary storage area (a buffer). Instead of sending individual, tiny samples, it waits until this buffer is filled with a specific chunk of audio.
3.  **Transmission:** Once the chunk is ready, the module sends this entire packet of data to the PulseAudio server efficiently. The server then handles the final physical signal conversion and transmission to the active sound device. This buffering process is essential for maintaining smooth, uninterrupted audio.

**Configuration:**
Users can control which output device is used. By specifying a `DEVICE` name, the output can be directed to a specific sound card or headset instead of the system's default audio output.

sha: 3a5342f8b54448a35e96a599765451c0a8c47e87e8d30de75828b9d8328706d6 
*/
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
