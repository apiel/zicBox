#define ZIC_LOG_LEVEL ZIC_LOG_DEBUG
#define DRAW_ST7789

#ifndef IS_RPI
#define DRAW_SDL
#endif

#include "config.h"
#include "draw/draw.h"
#include "helpers/getTicks.h"
#include "host.h"
#include "plugins/controllers/PixelController.h"
#include "styles.h"
#include "viewManager.h"

// Make from scratch UI

void* uiThread(void* = NULL)
{
    ViewManager& viewManager = ViewManager::get();
    viewManager.init();
    if (!viewManager.render()) {
        printf("No view were initialized to be rendered.");
        return NULL;
    }

    // int ms = 50;
    int ms = 80;
#ifdef DRAW_SDL
    logInfo("Rendering with SDL.");
    unsigned long lastUpdate = getTicks();
    while (viewManager.draw.handleEvent(viewManager.view)) {
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
    return NULL;
}

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

    lastPluginControllerInstance = new PixelController(controllerProps, 0);

    loadJsonConfig(argc >= 2 ? argv[1] : "data/config.json");

    showLogLevel();

    startHostThread();

    pthread_t ptid;
    pthread_create(&ptid, NULL, &uiThread, NULL);
    pthread_setname_np(ptid, "ui");

    // wait for uiThread to finish
    pthread_join(ptid, NULL);

    return 0;
}
