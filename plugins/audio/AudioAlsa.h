#pragma once

#include <algorithm>
#include <alsa/asoundlib.h>
#include <cstring>
#include <string>
#include <vector>

#include "audioPlugin.h"
#include "log.h"

#define ALSA_MAX_CHANNELS 2
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
            logDebug("Load output device: %s\n", deviceName.c_str());
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

        channels = std::min<unsigned int>(props.channels, ALSA_MAX_CHANNELS);
        sampleRate = props.sampleRate;

        logDebug("AudioAlsa::open %s (rate %u, channels %u)\n", deviceName.c_str(), sampleRate, channels);

        int err = snd_pcm_open(&handle, deviceName.c_str(), stream, 0);
        if (err < 0) {
            logWarn("snd_pcm_open(%s) failed: %s — trying 'default'\n", deviceName.c_str(), snd_strerror(err));
            if ((err = snd_pcm_open(&handle, "default", stream, 0)) < 0) {
                logError("Default ALSA open failed: %s\n", snd_strerror(err));
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
            logError("snd_pcm_set_params failed: %s\n", snd_strerror(err));
            snd_pcm_close(handle);
            handle = nullptr;
            return;
        }

        // Query actual buffer/period sizes
        snd_pcm_uframes_t bufFrames, periodFrames;
        if ((err = snd_pcm_get_params(handle, &bufFrames, &periodFrames)) < 0) {
            logWarn("snd_pcm_get_params failed: %s\n", snd_strerror(err));
        } else {
            logDebug("ALSA actual buffer_frames=%lu, period_frames=%lu\n",
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
                logError("snd_pcm_recover failed: %s\n", snd_strerror(rc));
                sampleIndex = 0;
                return;
            } else {
                logDebug("Recovered from XRUN/suspend\n");
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
            logWarn("Partial ALSA write: wrote %zu/%zu frames\n", framesWritten, frameCount);
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
            logDebug("- %s [DEVICE=hw:%d,0]\n", name, cardNum);
            if (name == deviceName) {
                sprintf(cardName, "hw:%d,0", cardNum);
                deviceName = cardName;
            }
        }

        snd_config_update_free_global();
    }
};
