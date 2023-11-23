#ifndef _HOST_H_
#define _HOST_H_

#include <SDL2/SDL.h> // SDL_Log

#include <vector>
#include <stdexcept>

#include "plugins/audio/valueInterface.h"
#include "UiPlugin.h"
#include "host/zicHost.h"

AudioPluginHandlerInterface* audioPluginHandler = NULL;
const char *configPath = "./config.cfg";

bool loadHost();

int hostThread(void *data)
{
    AudioPluginHandler::get().loop();
    return 0;
}

AudioPlugin &getPlugin(const char *name)
{
    if (!audioPluginHandler)
    {
        if (!loadHost())
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load host");
        }
    }
    
    if (strcmp(name, "UI") == 0)
    {
        return UiPlugin::get();
    }
    return audioPluginHandler->getPlugin(name);
}

bool loadHost()
{
    if (audioPluginHandler)
    {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Host already loaded\n");
        return true;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Initializing host\n");
    audioPluginHandler = initHost(configPath);
    if (!audioPluginHandler)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error initializing host\n");
        return false;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Starting host in SDL thread\n");
    SDL_Thread *thread = SDL_CreateThread(hostThread, "host", NULL);

    return true;
}

#endif