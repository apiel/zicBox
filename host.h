#ifndef _HOST_H_
#define _HOST_H_

#include <stdexcept>
#include <vector>
#include <pthread.h> 

#include "UiPlugin.h"
#include "host/zicHost.h"
#include "log.h"
#include "plugins/audio/valueInterface.h"

AudioPluginHandlerInterface* audioPluginHandler = NULL;

void* hostThread(void* data)
{
    AudioPluginHandler::get().loop();
    return NULL;
}

AudioPlugin& getPlugin(const char* name, int16_t track = -1)
{
    if (!audioPluginHandler) {
        throw std::runtime_error("Host not loaded");
    }

    if (strcmp(name, "UI") == 0) {
        return UiPlugin::get();
    }
    return audioPluginHandler->getPlugin(name, track);
}

void sendAudioEvent(AudioEventType event)
{
    if (!audioPluginHandler) {
        throw std::runtime_error("Host not loaded");
    }
    audioPluginHandler->sendEvent(event);
}

bool loadHost(std::string hostConfigPath, const char* pluginConfig)
{
    if (audioPluginHandler) {
        logDebug("Host already loaded\n");
        return true;
    }

    logDebug("Initializing host\n");
    audioPluginHandler = initHost(hostConfigPath.c_str(), pluginConfig);
    if (!audioPluginHandler) {
        logError("Error initializing host\n");
        return false;
    }

    logDebug("Starting host in thread\n");
    pthread_t ptid;
    pthread_create(&ptid, NULL, &hostThread, NULL);

    return true;
}

#endif