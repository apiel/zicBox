/** Description:
This comprehensive software component, named `AudioAlsa`, serves as the fundamental manager for handling audio input and output within a Linux environment, leveraging the standard Advanced Linux Sound Architecture (ALSA).

**Core Functionality**

The class operates as an audio plugin that establishes and maintains a direct connection to a specific sound card or audio device. It manages real-time data streaming to ensure smooth, continuous playback or recording.

**Setup and Configuration**

1.  **Device Connection:** The internal `open` function initializes the connection, attempting to link to a specified device (like "hw:0,0") or falling back to the system's "default" audio output. It also includes logic to search for available audio devices.
2.  **Audio Settings:** It configures critical audio properties inherited from the larger system, including the number of audio channels (e.g., stereo) and the sample rate (the quality of the sound, typically 48,000 measurements per second).
3.  **Latency Control:** A key configuration setting is the latency—the maximum acceptable delay before the audio is processed. This ensures the output remains responsive.

**Data Handling**

Audio data is not sent to the sound card sample-by-sample, but in small, efficient blocks called "chunks" (128 frames at a time).

*   **Buffering:** An internal temporary storage area (a buffer) collects this audio data.
*   **Flush Mechanism:** The crucial `flushBuffer` function handles the transfer of the full chunk from the buffer to the ALSA system. This mechanism is robust, including automatic recovery logic to address common streaming errors, such as timing lapses or buffer under-runs (known as "XRUNs"), which could otherwise cause glitches or silence.

In essence, `AudioAlsa` is the reliable link between an application’s audio data and the physical sound hardware on a Linux system.

sha: 74e4532d3d04563b2e2605ba26d5d8f8bddce8d3dda413dfcd8cff11dcb1b293 
*/
#pragma once

#include <algorithm>
#include <alsa/asoundlib.h>
#include <cstring>
#include <string>
#include <vector>

#include "host/constants.h"
#include "audioPlugin.h"
#include "log.h"

static constexpr uint32_t AUDIO_CHUNK_FRAMES = 128; // frames per ALSA write

class AudioAlsa : public AudioPlugin {
protected:
    AudioPlugin::Props& props;
    snd_pcm_t* handle = nullptr;
    snd_pcm_stream_t stream;

    std::string deviceName = "default";
    unsigned int channels = 1;
    unsigned int sampleRate = 48000;

    const snd_pcm_uframes_t chunkFrames = AUDIO_CHUNK_FRAMES;
    uint32_t sampleIndex = 0; // index into interleaved buffer (in samples)

    // interleaved buffer (samples = frames * channels)
    std::vector<char> buffer; // actual type set by subclass

    // unsigned int latencyUs = 150000;
    unsigned int latencyUs = 200000;

public:
    AudioAlsa(AudioPlugin::Props& props, AudioPlugin::Config& config, snd_pcm_stream_t stream)
        : AudioPlugin(props, config)
        , props(props)
        , stream(stream)
    {
        auto& json = config.json;
        if (json.contains("device")) {
            deviceName = json["device"].get<std::string>();
            logDebug("Load output device: %s", deviceName.c_str());
            search();
        }

        latencyUs = json.value("latency", latencyUs);
    }

    virtual ~AudioAlsa()
    {
        if (handle) {
            snd_pcm_close(handle);
        }
    }

    virtual bool isSink() { return true; }

protected:
    void open(snd_pcm_format_t format = SND_PCM_FORMAT_FLOAT)
    {
        if (handle) {
            snd_pcm_close(handle);
            handle = nullptr;
        }

        channels = std::min<unsigned int>(props.channels, CHANNEL_STEREO);
        sampleRate = props.sampleRate;

        logDebug("AudioAlsa::open %s (rate %u, channels %u)", deviceName.c_str(), sampleRate, channels);

        int err = snd_pcm_open(&handle, deviceName.c_str(), stream, 0);
        if (err < 0) {
            logWarn("snd_pcm_open(%s) failed: %s — trying 'default'", deviceName.c_str(), snd_strerror(err));
            if ((err = snd_pcm_open(&handle, "default", stream, 0)) < 0) {
                logError("Default ALSA open failed: %s", snd_strerror(err));
                handle = nullptr;
                return;
            }
        }

        if ((err = snd_pcm_set_params(handle,
                 format,
                 SND_PCM_ACCESS_RW_INTERLEAVED,
                 channels,
                 sampleRate,
                 1, // soft_resample
                 latencyUs))
            < 0) {
            logError("snd_pcm_set_params failed: %s", snd_strerror(err));
            snd_pcm_close(handle);
            handle = nullptr;
            return;
        }

        // Query actual buffer/period sizes
        snd_pcm_uframes_t bufFrames, periodFrames;
        if ((err = snd_pcm_get_params(handle, &bufFrames, &periodFrames)) < 0) {
            logWarn("snd_pcm_get_params failed: %s", snd_strerror(err));
        } else {
            logDebug("ALSA actual buffer_frames=%lu, period_frames=%lu",
                (unsigned long)bufFrames, (unsigned long)periodFrames);
        }

        sampleIndex = 0;
        resizeBuffer();
    }

    // must be implemented by derived class to set buffer type/size
    virtual void resizeBuffer() = 0;

    // Called by derived classes when buffer is full
    void flushBuffer(void* rawBuf, size_t frameCount)
    {
        if (!handle)
            return;

        snd_pcm_sframes_t ret = snd_pcm_writei(handle, rawBuf, frameCount);
        if (ret < 0) {
            int rc = snd_pcm_recover(handle, static_cast<int>(ret), 0);
            if (rc < 0) {
                logError("snd_pcm_recover failed: %s", snd_strerror(rc));
                sampleIndex = 0;
                return;
            } else {
                logDebug("Recovered from XRUN/suspend");
                sampleIndex = 0;
                return;
            }
        } else if (ret != (snd_pcm_sframes_t)frameCount) {
            // partial write
            size_t framesWritten = (size_t)ret;
            size_t samplesWritten = framesWritten * channels;
            size_t samplesTotal = frameCount * channels;
            size_t samplesLeft = samplesTotal - samplesWritten;

            if (samplesLeft > 0) {
                // move leftover samples to front of buffer
                size_t sampleSize = buffer.size() / (chunkFrames * channels);
                memmove(buffer.data(),
                    (char*)buffer.data() + samplesWritten * sampleSize,
                    samplesLeft * sampleSize);
                sampleIndex = samplesLeft;
            } else {
                sampleIndex = 0;
            }
            logWarn("Partial ALSA write: wrote %zu/%zu frames", framesWritten, frameCount);
        } else {
            sampleIndex = 0; // all good
        }
    }

    void search()
    {
        char cardName[20];
        int cardNum = -1;
        while (snd_card_next(&cardNum) > -1 && cardNum > -1) {
            char* name;
            snd_card_get_name(cardNum, &name);
            logDebug("- %s [DEVICE=hw:%d,0]", name, cardNum);
            if (name == deviceName) {
                sprintf(cardName, "hw:%d,0", cardNum);
                deviceName = cardName;
            }
        }

        snd_config_update_free_global();
    }
};
