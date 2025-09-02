#pragma once

#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

#include "audioPlugin.h"
#include "log.h"

static void pa_context_state_callback(pa_context* context, void* userdata);

class AudioPulse : public AudioPlugin {
protected:
    static const uint32_t audioChunk = 128;
    const uint32_t bufferReadSize = (sizeof(float) / sizeof(uint8_t)) * audioChunk;

    float buffer[audioChunk];
    pa_simple* device = NULL;

    uint32_t bufferIndex = 0;

    AudioPlugin::Props& props;

    virtual pa_simple* newDevice(const pa_sample_spec streamFormat) = 0;

    int retry = 0;
    void open()
    {
        logDebug("AudioPulse::open");

        if (device) {
            pa_simple_free(device);
            device = NULL;
        }

        static const pa_sample_spec streamFormat = {
            .format = PA_SAMPLE_FLOAT32LE,
            .rate = (uint32_t)props.sampleRate,
            .channels = props.channels,
        };

        device = newDevice(streamFormat);
        if (!device) {
            logError("ERROR: pa_simple_new() failed: %s", pa_strerror(pa_context_errno(NULL)));
            if (retry < 2) {
                retry++;
                logInfo("AudioPulse::open retry (%d)", retry);
                system("/etc/init.d/S33pulseaudio restart");
                open();
            }
            return;
        }
        logDebug("AudioPulse::open done");
    }

    void search()
    {
        pa_mainloop* ml = NULL;
        pa_context* context = NULL;
        char* server = NULL;

        if (!(ml = pa_mainloop_new())) {
            logError("pa_mainloop_new() failed: %s", pa_strerror(pa_context_errno(NULL)));
            return freeListDevice(ml, context, server);
        }

        paMainLoopApi = pa_mainloop_get_api(ml);

        if (!(context = pa_context_new_with_proplist(paMainLoopApi, NULL, NULL))) {
            logError("pa_context_new() failed: %s", pa_strerror(pa_context_errno(NULL)));
            return freeListDevice(ml, context, server);
        }

        pa_context_set_state_callback(context, pa_context_state_callback, this);

        if (pa_context_connect(context, server, PA_CONTEXT_NOFLAGS, NULL) < 0) {
            logError("pa_context_connect() failed: %s", pa_strerror(pa_context_errno(context)));
            return freeListDevice(ml, context, server);
        }

        int ret = 1;
        if (pa_mainloop_run(ml, &ret) < 0) {
            logError("pa_mainloop_run() failed: %s", pa_strerror(pa_context_errno(context)));
        }
        freeListDevice(ml, context, server);
    }

    void freeListDevice(pa_mainloop* ml, pa_context* context, char* server = NULL)
    {
        if (context) {
            pa_context_unref(context);
        }

        if (ml) {
            pa_mainloop_free(ml);
        }

        pa_xfree(server);
    }

public:
    const char* deviceName = NULL;
    pa_mainloop_api* paMainLoopApi;

    AudioPulse(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : AudioPlugin(props, config)
        , props(props)
    {
        for (uint32_t i = 0; i < audioChunk; i++) {
            buffer[i] = 0.0f;
        }

        auto& json = config.json;
        if (json.contains("device")) {
            deviceName = (char *)json["device"].get<std::string>().c_str();
            logDebug("Load output device: %s\n", deviceName);
            search();
            open();
        }
    }

    virtual bool isSink()
    {
        return true;
    }
};

static void pa_set_info(int eol, void* userdata, const pa_proplist* proplist, const char* name)
{
    AudioPulse* api = (AudioPulse*)userdata;

    if (eol) {
        api->paMainLoopApi->quit(api->paMainLoopApi, 0);
        return;
    }

    const char* description = pa_proplist_gets(proplist, "device.description");
    logDebug("- %s [DEVICE=%s] or [DEVICE=%s]", description, description, name);

    if (strcmp(description, api->deviceName) == 0) {
        AudioPulse* api = (AudioPulse*)userdata;
        api->deviceName = name;
    }
}

static void pa_set_source_info(pa_context* c, const pa_source_info* i, int eol, void* userdata)
{
    if (eol) {
        pa_set_info(eol, userdata, NULL, NULL);
    } else {
        pa_set_info(eol, userdata, i->proplist, i->name);
    }
}

static void pa_set_sink_info(pa_context* c, const pa_sink_info* i, int eol, void* userdata)
{
    if (eol) {
        pa_set_info(eol, userdata, NULL, NULL);
    } else {
        pa_set_info(eol, userdata, i->proplist, i->name);
    }
}

static void pa_context_state_callback(pa_context* context, void* userdata)
{
    AudioPulse* api = (AudioPulse*)userdata;

    switch (pa_context_get_state(context)) {
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        break;

    case PA_CONTEXT_READY: {
        if (api->isSink()) {
            pa_context_get_sink_info_list(context, pa_set_sink_info, userdata);
        } else {
            pa_context_get_source_info_list(context, pa_set_source_info, userdata);
        }
        break;
    }

    case PA_CONTEXT_TERMINATED:
        api->paMainLoopApi->quit(api->paMainLoopApi, 0);
        break;

    case PA_CONTEXT_FAILED:
    default:
        logWarn("PA_CONTEXT_FAILED.");
        api->paMainLoopApi->quit(api->paMainLoopApi, 0);
    }
}
