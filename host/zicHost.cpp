#include "AudioPluginHandler.h"
#include "config.h"
#include "def.h"

extern "C" AudioPluginHandlerInterface* load(const char* scriptPath = NULL)
{
    if (scriptPath != NULL) {
        loadConfigPlugin(NULL, scriptPath, hostScriptCallback);
    }
    return &AudioPluginHandler::get();
}

int main()
{
    AudioPluginHandlerInterface* audioPluginHandler = load("./config.lua");
    audioPluginHandler->loop();
    return 0;
}