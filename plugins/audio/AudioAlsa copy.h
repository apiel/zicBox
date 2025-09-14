#pragma once

#include <alsa/asoundlib.h>

#include "audioPlugin.h"
#include "log.h"

#define ALSA_MAX_CHANNELS 2

class AudioAlsa : public AudioPlugin {
protected:
    char cardName[20];
    static const uint32_t audioChunk = 128;

    float buffer[audioChunk * ALSA_MAX_CHANNELS];
    snd_pcm_uframes_t bufferSize = audioChunk * ALSA_MAX_CHANNELS;

    snd_pcm_uframes_t bufferIndex = 0;

    AudioPlugin::Props& props;

    snd_pcm_t* handle = nullptr;

    void search()
    {
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

    snd_pcm_stream_t stream;

    void open(snd_pcm_format_t format = SND_PCM_FORMAT_FLOAT)
    {
        if (handle) {
            snd_pcm_close(handle);
        }

        logDebug("AudioAlsa::open %s (rate %d, channels %d)\n", deviceName.c_str(), props.sampleRate, props.channels);

        int err;
        if ((err = snd_pcm_open(&handle, deviceName.c_str(), stream, 0)) < 0) {
            logDebug("Playback open audio card \"%s\" error: %s.\nOpen default sound card\n", deviceName.c_str(), snd_strerror(err));
            if ((err = snd_pcm_open(&handle, "default", stream, 0)) < 0) {
                logError("Default playback audio card error: %s\n", snd_strerror(err));
                return;
            }
        }

        unsigned int channels = props.channels > ALSA_MAX_CHANNELS ? ALSA_MAX_CHANNELS : props.channels;
#ifdef IS_RPI
        snd_pcm_uframes_t periodSize = 150000;
#else
        snd_pcm_uframes_t periodSize = 50000; // 50ms // 100000
#endif
        if ((err = snd_pcm_set_params(handle,
                 format,
                 SND_PCM_ACCESS_RW_INTERLEAVED,
                 channels,
                 props.sampleRate, 1, periodSize))
            < 0) {
            logError("Audio card params error: %s\n", snd_strerror(err));
            return;
        }
        bufferSize = audioChunk * channels;

        // snd_pcm_uframes_t buffer_size = audioChunk;
        // unsigned int sample_rate = props.sampleRate;
        // snd_pcm_hw_params_t* hw_params;
        // snd_pcm_hw_params_alloca(&hw_params);
        // snd_pcm_hw_params_any(handle, hw_params);
        // snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
        // snd_pcm_hw_params_set_format(handle, hw_params, SND_PCM_FORMAT_FLOAT);
        // snd_pcm_hw_params_set_channels(handle, hw_params, channels);
        // snd_pcm_hw_params_set_rate_near(handle, hw_params, &sample_rate, nullptr);
        // snd_pcm_hw_params_set_period_size_near(handle, hw_params, &periodSize, nullptr);
        // snd_pcm_hw_params_set_buffer_size_near(handle, hw_params, &buffer_size);
        // if ((err = snd_pcm_hw_params(handle, hw_params)) < 0) {
        //     logError("Audio card params error: %s\n", snd_strerror(err));
        //     return;
        // }

        snd_pcm_uframes_t buffer_size;
        snd_pcm_hw_params_t* hw_params;
        snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size);
        logDebug("AudioAlsa::openned (periodSize %d, buffer_size %d)\n", periodSize, bufferSize);
        if (buffer_size != bufferSize) {
            logWarn("AudioAlsa::openned (buffer_size %d, expected %d)\n", buffer_size, bufferSize);
        }
    }

public:
    std::string deviceName = "default";

    AudioAlsa(AudioPlugin::Props& props, AudioPlugin::Config& config, snd_pcm_stream_t stream)
        : AudioPlugin(props, config)
        , props(props)
        , stream(stream)
    {
        for (uint32_t i = 0; i < bufferSize; i++) {
            buffer[i] = 0.0f;
        }

        auto& json = config.json;
        if (json.contains("device")) {
            deviceName = json["device"].get<std::string>();
            logDebug("Load output device: %s\n", deviceName.c_str());
            search();
            open();
        }
    }

    virtual bool isSink()
    {
        return true;
    }
};
