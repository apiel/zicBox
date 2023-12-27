// TODO make it configurable
#define ENCODER_COUNT 8

#include "draw.h"
#include "event.h"
#include "viewManager.h"
#include "host.h"
#include "config.h"
#include "styles.h"

int main( int argc, char* argv[] )
{
    loadUiConfig(argc == 2 ? argv[1] : "config.ui");

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "SDL video driver: %s", SDL_GetCurrentVideoDriver());

#ifdef IS_RPI
    SDL_ShowCursor(SDL_DISABLE);
#endif

    ViewManager &viewManager = ViewManager::get();
    viewManager.draw.init();

    if (!loadHost())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load host");
    }

    viewManager.init();
    if (!viewManager.render())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No view were initialized to be rendered.");
        return 1;
    }

    EventHandler &event = EventHandler::get();
    unsigned long lastUpdate = SDL_GetTicks();
    while (event.handle())
    {
        unsigned long now = SDL_GetTicks();
        if (now - lastUpdate > 50)
        {
            lastUpdate = now;
            viewManager.renderComponents(now);
        }
        SDL_Delay(1);
    }

    SDL_Quit();

    return 0;
}
