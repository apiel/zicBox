#ifndef _AUDIO_API_INTERFACE_H_
#define _AUDIO_API_INTERFACE_H_

// old audio api...

#include <alsa/asoundlib.h>

#include "audioApi.h"
#include "audioPluginHandler.h"
#include "def.h"

class AudioApiAlsa : public AudioApi {
protected:
    AudioPluginHandler& audioHandler = AudioPluginHandler::get();

    static AudioApiAlsa* instance;
    AudioApiAlsa() { }

public:
    static AudioApiAlsa& get()
    {
        if (!instance) {
            instance = new AudioApiAlsa();
        }
        return *instance;
    }

    int open()
    {
        APP_PRINT("AudioApiAlsa::open\n");

        snd_pcm_t* handleOut;
        snd_pcm_t* handleIn;

        int err;
        if ((err = snd_pcm_open(&handleOut, audioOutputName, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
            logInfo("Playback open audio card \"%s\" error: %s.\nOpen default sound card\n", audioOutputName, snd_strerror(err));
            if ((err = snd_pcm_open(&handleOut, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
                logInfo("Default playback audio card error: %s\n", snd_strerror(err));
                return 1;
            }
        }

        if ((err = snd_pcm_open(&handleIn, audioInputName, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
            logInfo("Capture open audio card \"%s\" error: %s.\nOpen default sound card\n", audioInputName, snd_strerror(err));
            if ((err = snd_pcm_open(&handleIn, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
                logInfo("Default capture audio card error: %s\n", snd_strerror(err));
                return 1;
            }
        }

        // Float
        // if ((err = snd_pcm_set_params(handleOut, SND_PCM_FORMAT_FLOAT, SND_PCM_ACCESS_RW_INTERLEAVED,
        //          APP_CHANNELS, SAMPLE_RATE, 1, 500000))
        //     < 0) {
        //     logInfo("Audio card params error: %s\n", snd_strerror(err));
        //     return 1;
        // }
        // while (isRunning) {
        //     float outputBuffer[APP_AUDIO_CHUNK * APP_CHANNELS];
        //     audioHandler.samples((float*)outputBuffer, APP_AUDIO_CHUNK * APP_CHANNELS);
        //     snd_pcm_writei(handleOut, outputBuffer, APP_AUDIO_CHUNK);
        // }

        // snd_pcm_format_t format = SND_PCM_FORMAT_S32;
        snd_pcm_format_t format = SND_PCM_FORMAT_S16;
        if ((err = snd_pcm_set_params(handleOut, format, SND_PCM_ACCESS_RW_INTERLEAVED,
                 APP_CHANNELS, SAMPLE_RATE, 1, 500000))
            < 0) {
            logInfo("Audio card params error: %s\n", snd_strerror(err));
            return 1;
        }

        if ((err = snd_pcm_set_params(handleIn, format, SND_PCM_ACCESS_RW_INTERLEAVED,
                 APP_CHANNELS, SAMPLE_RATE, 1, 500000))
            < 0) {
            logInfo("Audio card params error: %s\n", snd_strerror(err));
            return 1;
        }

        while (isRunning) {
            // float inputBuffer[APP_AUDIO_CHUNK * APP_CHANNELS];
            // int32_t inputBuffer32[APP_AUDIO_CHUNK * APP_CHANNELS];
            // if (snd_pcm_readi(handleIn, inputBuffer32, APP_AUDIO_CHUNK) < 0) {
            //     logInfo("Audio card read error: %s\n", snd_strerror(err));
            //     return 1;
            // }
            // for (int i = 0; i < APP_AUDIO_CHUNK * APP_CHANNELS; i++) {
            //     inputBuffer[i] = (float)inputBuffer32[i] / 2147483647.0f;
            // }

            // float outputBuffer[APP_AUDIO_CHUNK * APP_CHANNELS];
            // audioHandler.samples(inputBuffer, (float*)outputBuffer, APP_AUDIO_CHUNK * APP_CHANNELS);
            // int32_t outputBuffer32[APP_AUDIO_CHUNK * APP_CHANNELS];
            // for (int i = 0; i < APP_AUDIO_CHUNK * APP_CHANNELS; i++) {
            //     outputBuffer32[i] = (int32_t)(outputBuffer[i] * 2147483647.0f);
            // }
            // snd_pcm_writei(handleOut, outputBuffer32, APP_AUDIO_CHUNK);

            int32_t inputBuffer32[APP_AUDIO_CHUNK * APP_CHANNELS];
            if (snd_pcm_readi(handleIn, inputBuffer32, APP_AUDIO_CHUNK) < 0) {
                logInfo("Audio card read error: %s\n", snd_strerror(err));
                return 1;
            }

            int32_t outputBuffer32[APP_AUDIO_CHUNK * APP_CHANNELS];
            audioHandler.samples(inputBuffer32, outputBuffer32, APP_AUDIO_CHUNK * APP_CHANNELS);
            snd_pcm_writei(handleOut, outputBuffer32, APP_AUDIO_CHUNK);
        }

        snd_pcm_close(handleOut);
        return 0;
    }

    void list()
    {
        int cardNum = -1;
        while (snd_card_next(&cardNum) > -1 && cardNum > -1) {
            char* name;
            snd_card_get_name(cardNum, &name);
            APP_PRINT("- %s [AUDIO_OUTPUT=hw:%d,0]\n", name, cardNum);
        }

        snd_config_update_free_global();
    }
};

AudioApiAlsa* AudioApiAlsa::instance = NULL;

#endif