#define USE_SDL_TIMER

#include "config.h"
#include "draw/SDL/draw.h"
#include "SDL_EventHandler.h"
#include "host.h"
#include "styles.h"
#include "viewManager.h"
#include "timer.h"

int main(int argc, char* argv[])
{
    styles.screen = { 800, 480 };
    loadUiConfig(argc >= 2 ? argv[1] : "config.ui", argc >= 3 ? argv[2] : NULL);

    // SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

    startHostThread();

    ViewManager& viewManager = ViewManager::get();
    viewManager.init();
    if (!viewManager.render()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No view were initialized to be rendered.");
        return 1;
    }

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

    // AudioPluginHandler::get().hydrate("serialized/track0.cfg", 0);

    EventHandler& event = EventHandler::get();
    unsigned long lastUpdate = getTicks();
    while (event.handle()) {
        unsigned long now = getTicks();
        if (now - lastUpdate > 50) {
            lastUpdate = now;
            viewManager.renderComponents(now);
        }
        SDL_Delay(1);
    }

    // AudioPluginHandler::get().serialize("serialized/track0.cfg", 0);

    SDL_Quit();

    return 0;
}
