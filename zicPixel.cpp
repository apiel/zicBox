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
#include "helpers/getTicks.h"
#include "host.h"
#include "plugins/controllers/PixelController.h"
#include "styles.h"
#include "viewManager.h"

// Make from scratch UI

int main(int argc, char* argv[])
{
    loadHostPlugin();

    // styles.colors.primary = { 0x3a, 0x7d, 0x80 }; // #3a7d80
    // styles.colors.secondary = { 0xff, 0xa0, 0xad }; // #ffa0ad
    // styles.colors.tertiary = { 0xa0, 0xb4, 0x97 }; // #A0B497
    // styles.colors.quaternary = { 0xe6, 0xc3, 0x4a }; // #e6c34a

    styles.colors.primary = { 0x4f, 0xbf, 0xc5 }; // #4fbfc5
    styles.colors.secondary = { 0xff, 0x8a, 0x94 }; // #ff8a94
    styles.colors.tertiary = { 0xc5, 0xd8, 0xb2 }; // #c5d8b2
    styles.colors.quaternary = { 0xf7, 0xda, 0x6d }; // #f7da6d

    // loadPixelComponents();

    // loadUiConfig(argc >= 2 ? argv[1] : "pixel.cfg", argc >= 3 ? argv[2] : NULL, styles);
    // loadUiConfig("config/pixel/main.lua", "lua", styles);
    loadUiConfig(argc >= 2 ? argv[1] : "pixel.lua", argc >= 3 ? argv[2] : NULL, styles);

    showLogLevel();

    startHostThread();

    // while(1) { usleep(10000); }

    ViewManager& viewManager = ViewManager::get();
    viewManager.init();
    if (!viewManager.render()) {
        printf("No view were initialized to be rendered.");
        return 1;
    }

    new PixelController(controllerProps, 0);

    // int ms = 50;
    int ms = 80;
#ifdef USE_DRAW_WITH_SDL
    logInfo("Rendering with SDL.");
    unsigned long lastUpdate = getTicks();
    EventHandler& event = EventHandler::get();
    while (event.handle()) {
        unsigned long now = getTicks();
        if (now - lastUpdate > ms) {
            lastUpdate = now;
            viewManager.renderComponents(now);
        }
        usleep(1);
    }
#else
    int us = ms * 1000;
    while (true) {
        unsigned long now = getTicks();
        viewManager.renderComponents(now);
        usleep(us);
    }
#endif

    viewManager.draw.quit();

    return 0;
}
