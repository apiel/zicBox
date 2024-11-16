#include "audioPluginHandler.h"
#include "config.h"
#include "def.h"

extern "C" {
AudioPluginHandlerInterface* init()
{
    loadHostConfig("./config.cfg", NULL);
    return &AudioPluginHandler::get();
}

int mainLoop()
{
    AudioPluginHandler::get().loop();
    return 0;
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
    return mainLoop();
}