#include "AudioPluginHandler.h"
#include "config.h"
#include "def.h"

extern "C" {
AudioPluginHandlerInterface* load(const char* scriptPath =  "./config.lua")
{
    loadConfigPlugin(NULL, scriptPath, hostScriptCallback);
    return &AudioPluginHandler::get();
}

void midi(std::vector<unsigned char>* message)
{
    midiHandler(message);
}
}

int main()
{
    AudioPluginHandlerInterface* audioPluginHandler = load();
    audioPluginHandler->loop();
    return 0;
}