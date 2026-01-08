#pragma once

#include <dlfcn.h>
#include <pthread.h>
#include <stdexcept>
#include <vector>

#include "log.h"
#include "plugins/audio/audioPlugin.h"
#include "helpers/getExecutableDirectory.h"

#define USE_HOST_SO
#ifdef USE_HOST_SO

struct Host {
    AudioPluginHandlerInterface* (*load)();
    AudioPluginHandlerInterface* audioPluginHandler = NULL;
}* host = NULL;

void hostConfig(nlohmann::json& config)
{
    if (host) {
        host->audioPluginHandler->config(config);
    }
}

void* hostThread(void* = NULL)
{
    if (host) {
        host->audioPluginHandler->loop();
    }
    return NULL;
}

AudioPluginHandlerInterface* getAudioPluginHandler()
{
    return host->audioPluginHandler;
}

void loadHostPlugin()
{
    if (host) {
        logWarn("Host plugin already loaded");
        return;
    }

    void* handle = dlopen((getExecutableDirectory() + "/libs/libzicHost.so").c_str(), RTLD_LAZY);
    if (!handle) {
        logError("Cannot open host library libzicHost: %s", dlerror());
        return;
    }

    dlerror();

    host = new Host();
    host->load = (AudioPluginHandlerInterface * (*)()) dlsym(handle, "load");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        logError("Host plugin, cannot load symbol: %s", dlsym_error);
        dlclose(handle);
        return;
    }
    host->audioPluginHandler = host->load();
}
#else
#include "host/AudioPluginHandler.h"
#include "plugins/audio/valueInterface.h"

void* hostThread(void* = NULL)
{
    AudioPluginHandler::get().loop();
    return NULL;
}

AudioPluginHandlerInterface* getAudioPluginHandler()
{
    return &AudioPluginHandler::get();
}

void loadHostPlugin()
{
}
#endif

void startHostThread()
{
    logDebug("Starting host in thread\n");
    pthread_t ptid;
    pthread_create(&ptid, NULL, &hostThread, NULL);
    pthread_setname_np(ptid, "host");
}