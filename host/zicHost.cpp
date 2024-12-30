#include "AudioPluginHandler.h"
#include "config.h"
#include "def.h"

extern "C" {
AudioPluginHandlerInterface* init(const char* scriptPath =  "./config.lua")
{
    loadConfigPlugin(NULL, scriptPath, hostScriptCallback);
    return &AudioPluginHandler::get();
}

void* hostLoop(void* = NULL)
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

void midi(std::vector<unsigned char>* message)
{
    midiHandler(message);
}
}

int main()
{
    if (!init()) {
        return 1;
    }
    hostLoop();
    return 0;
}