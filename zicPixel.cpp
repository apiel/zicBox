#define DRAW_SSD1306

#ifndef IS_RPI
#define USE_DRAW_WITH_SDL
#endif

#include "config.h"
#include "draw/SSD1306/draw.h"
#include "host.h"
#include "styles.h"
#include "timer.h"
#include "viewManager.h"

#ifdef USE_DRAW_WITH_SDL
#include "SDL_EventHandler.h"
#endif

int main(int argc, char* argv[])
{
    styles.screen = { 128, 64 };
    loadUiConfig(argc >= 2 ? argv[1] : "pixel.ui", argc >= 3 ? argv[2] : NULL);

    ViewManager& viewManager = ViewManager::get();
    viewManager.init();
    if (!viewManager.render()) {
        printf("No view were initialized to be rendered.");
        return 1;
    }

    unsigned long lastUpdate = getTicks();
#ifdef USE_DRAW_WITH_SDL
    EventHandler& event = EventHandler::get();
    while (event.handle()) {
#else
    while (true) {
#endif
        unsigned long now = getTicks();
        if (now - lastUpdate > 50) {
            lastUpdate = now;
            viewManager.renderComponents(now);
        }
        usleep(1);
    }

#ifdef USE_DRAW_WITH_SDL
    SDL_Quit();
#endif

    return 0;
}
