#ifndef _HOST_H_
#define _HOST_H_

#include <dlfcn.h>
#include <vector>
#include <stdexcept>

#include "plugins/audio/valueInterface.h"
#include "UiPlugin.h"

AudioPluginHandlerInterface* audioPluginHandler = NULL;

AudioPluginHandlerInterface*(*initHost)() = NULL;
int (*mainLoopHost)() = NULL;
void (*midiHost)(std::vector<unsigned char> *message) = [](std::vector<unsigned char> *message) {};

bool loadHost();

int hostThread(void *data)
{
    return mainLoopHost();
}

void *linkHost(void *handle, const char *name)
{
    void *fn = dlsym(handle, name);
    const char *dlsym_error = dlerror();
    if (dlsym_error)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot load symbol: %s\n", dlsym_error);
        dlclose(handle);
        return NULL;
    }
    return fn;
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

    // FIXME
    const char *path = "../zicHost/zicHost.so";
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loading host from %s\n", path);
    void *handle = dlopen(path, RTLD_LAZY);

    if (!handle)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading host (%s): %s\n", path, dlerror());
        return false;
    }

    dlerror(); // clear previous error

    initHost = (AudioPluginHandlerInterface* (*)(void)) linkHost(handle, "init");
    if (!initHost)
    {
        return false;
    }

    mainLoopHost = (int (*)())linkHost(handle, "mainLoop");
    if (!mainLoopHost)
    {
        return false;
    }

    midiHost = (void (*)(std::vector<unsigned char> *))linkHost(handle, "midi");
    if (!midiHost)
    {
        return false;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Initializing host\n");
    audioPluginHandler = initHost();
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