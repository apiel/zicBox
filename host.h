#ifndef _HOST_H_
#define _HOST_H_

#include <SDL2/SDL.h> // SDL_Log

#include <stdexcept>
#include <vector>

#include "UiPlugin.h"
#include "host/zicHost.h"
#include "plugins/audio/valueInterface.h"

AudioPluginHandlerInterface* audioPluginHandler = NULL;

int hostThread(void* data)
{
    AudioPluginHandler::get().loop();
    return 0;
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

bool loadHost(std::string hostConfigPath)
{
    if (audioPluginHandler) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Host already loaded\n");
        return true;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Initializing host\n");
    audioPluginHandler = initHost(hostConfigPath.c_str());
    if (!audioPluginHandler) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error initializing host\n");
        return false;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Starting host in SDL thread\n");
    SDL_Thread* thread = SDL_CreateThread(hostThread, "host", NULL);

    return true;
}

#endif