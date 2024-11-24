#define ZIC_LOG_LEVEL ZIC_LOG_DEBUG
#define DRAW_ST7789

#ifndef IS_RPI
#define USE_DRAW_WITH_SDL
#endif

#ifdef USE_DRAW_WITH_SDL
// Define before config else ADD_ZONE_ENCODER not working
#include "SDL_EventHandler.h"
#endif

#include "config.h"
#include "draw/ST7789/draw.h"
#include "host.h"
#include "plugins/components/Pixel/PixelComponents.h"
#include "styles.h"
#include "timer.h"
#include "viewManager.h"

// Make from scratch UI

int main(int argc, char* argv[])
{
    styles.screen = { 240, 240 };
    styles.colors.primary = { 0x28, 0x59, 0x5f };

    loadPixelComponents();

    loadUiConfig(argc >= 2 ? argv[1] : "pixel.ui", argc >= 3 ? argv[2] : NULL, styles);

    showLogLevel();

    startHostThread();

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

    viewManager.draw.quit();

    return 0;
}
