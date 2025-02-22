#ifndef _HOST_H_
#define _HOST_H_

#include <dlfcn.h>
#include <pthread.h>
#include <stdexcept>
#include <vector>

#include "log.h"
#include "plugins/audio/audioPlugin.h"

#define USE_HOST_SO
#ifdef USE_HOST_SO
#include "plugins/config/LuaConfig.h"

struct Host {
    void (*hostScriptCallback)(char* key, char* value, const char* filename, std::vector<Var> variables);
    AudioPluginHandlerInterface* (*load)(const char* scriptPath);
    AudioPluginHandlerInterface* audioPluginHandler = NULL;
    void (*midiHandler)(std::vector<unsigned char>* message);
}* host = NULL;

void hostScriptCallback(char* key, char* value, const char* filename, std::vector<Var> variables)
{
    if (host) {
        host->hostScriptCallback(key, value, filename, variables);
    }
}

void midiHandler(std::vector<unsigned char>* message)
{
    if (host) {
        host->midiHandler(message);
    }
}

AudioPlugin& getPlugin(std::string name, int16_t track = -1)
{
    return host->audioPluginHandler->getPlugin(name, track);
}

void hostConfig(nlohmann::json& config)
{
    if (host) {
        host->audioPluginHandler->config(config);
    }
}

void sendAudioEvent(AudioEventType event)
{
    host->audioPluginHandler->sendEvent(event);
}

void* hostThread(void* = NULL)
{
    if (host) {
        host->audioPluginHandler->loop();
    }
    return NULL;
}


void loadHostPlugin()
{
    if (host) {
        logWarn("Host plugin already loaded");
        return;
    }

#ifdef IS_RPI
    void* handle = dlopen("host/libzicHost.arm.so", RTLD_LAZY);
#else
    void* handle = dlopen("host/libzicHost.x86.so", RTLD_LAZY);
#endif
    if (!handle) {
        logError("Cannot open host library libzicHost: %s", dlerror());
        return;
    }

    dlerror();

    host = new Host();
    host->hostScriptCallback = (void (*)(char* key, char* value, const char* filename, std::vector<Var> variables))dlsym(handle, "hostScriptCallback");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        logError("Host plugin, cannot load symbol: %s", dlsym_error);
        dlclose(handle);
        return;
    }

    host->load = (AudioPluginHandlerInterface * (*)(const char* scriptPath)) dlsym(handle, "load");
    dlsym_error = dlerror();
    if (dlsym_error) {
        logError("Host plugin, cannot load symbol: %s", dlsym_error);
        dlclose(handle);
        return;
    }
    host->audioPluginHandler = host->load(NULL);

    host->midiHandler = (void (*)(std::vector<unsigned char>* message))dlsym(handle, "midiHandler");
    dlsym_error = dlerror();
    if (dlsym_error) {
        logError("Host plugin, cannot load symbol: %s", dlsym_error);
        dlclose(handle);
        return;
    }
}
#else
#include "host/AudioPluginHandler.h"
#include "host/config.h"
#include "plugins/audio/valueInterface.h"

void* hostThread(void* = NULL)
{
    AudioPluginHandler::get().loop();
    return NULL;
}

AudioPlugin& getPlugin(std::string name, int16_t track = -1)
{
    return AudioPluginHandler::get().getPlugin(name, track);
}

void sendAudioEvent(AudioEventType event)
{
    AudioPluginHandler::get().sendEvent(event);
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

#endif