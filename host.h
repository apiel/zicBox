#ifndef _HOST_H_
#define _HOST_H_

#include <pthread.h>
#include <stdexcept>
#include <vector>

#include "host/audioPluginHandler.h"
#include "host/config.h"
#include "log.h"
#include "plugins/audio/valueInterface.h"

void* hostThread(void* data)
{
    AudioPluginHandler::get().loop();
    return NULL;
}

AudioPlugin& getPlugin(const char* name, int16_t track = -1)
{
    return AudioPluginHandler::get().getPlugin(name, track);
}

void sendAudioEvent(AudioEventType event)
{
    AudioPluginHandler::get().sendEvent(event);
}

void startHostThread()
{
    logDebug("Starting host in thread\n");
    pthread_t ptid;
    pthread_create(&ptid, NULL, &hostThread, NULL);
}

#endif