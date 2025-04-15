#pragma once

#include <alsa/asoundlib.h>

#include "audioPlugin.h"
#include "log.h"

#define ALSA_MAX_CHANNELS 2

class AudioAlsa : public AudioPlugin {
protected:
    char cardName[20];
    static const uint32_t audioChunk = 128;

    // int32_t buffer[audioChunk];
    float buffer[audioChunk * ALSA_MAX_CHANNELS];
    uint32_t bufferSize = audioChunk * ALSA_MAX_CHANNELS;

    uint32_t bufferIndex = 0;

    AudioPlugin::Props& props;

    snd_pcm_t* handle;

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

    void open()
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
            }
        }

        if ((err = snd_pcm_set_params(handle,
                 SND_PCM_FORMAT_FLOAT,
                 SND_PCM_ACCESS_RW_INTERLEAVED,
                 props.channels > ALSA_MAX_CHANNELS ? ALSA_MAX_CHANNELS : props.channels,
                 props.sampleRate, 1, 500000))
            < 0) {
            logError("Audio card params error: %s\n", snd_strerror(err));
            return;
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
