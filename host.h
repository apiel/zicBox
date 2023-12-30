#ifndef _HOST_H_
#define _HOST_H_

#include <SDL2/SDL.h> // SDL_Log

#include <stdexcept>
#include <vector>

#include "UiPlugin.h"
#include "host/zicHost.h"
#include "plugins/audio/valueInterface.h"

AudioPluginHandlerInterface* audioPluginHandler = NULL;
char hostConfigPath[512] = "./config.cfg";

bool loadHost();

int hostThread(void* data)
{
    AudioPluginHandler::get().loop();
    return 0;
}

AudioPlugin& getPlugin(GetPluginParams params)
{
    if (!audioPluginHandler) {
        if (!loadHost()) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load host");
        }
    }

    if (strcmp(params.name, "UI") == 0) {
        return UiPlugin::get();
    }
    return audioPluginHandler->getPlugin(params.name, params.track);
}

bool loadHost()
{
    if (audioPluginHandler) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Host already loaded\n");
        return true;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Initializing host\n");
    audioPluginHandler = initHost((const char*)hostConfigPath);
    if (!audioPluginHandler) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error initializing host\n");
        return false;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Starting host in SDL thread\n");
    SDL_Thread* thread = SDL_CreateThread(hostThread, "host", NULL);

    return true;
}

#endif