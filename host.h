#ifndef _HOST_H_
#define _HOST_H_

#include <pthread.h>
#include <stdexcept>
#include <vector>

#include "UiPlugin.h"
#include "host/audioPluginHandler.h"
#include "host/config.h"
#include "log.h"
#include "plugins/audio/valueInterface.h"

AudioPluginHandlerInterface* audioPluginHandler = &AudioPluginHandler::get();

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

void startHostThread()
{
    logDebug("Starting host in thread\n");
    pthread_t ptid;
    pthread_create(&ptid, NULL, &hostThread, NULL);
}

#endif