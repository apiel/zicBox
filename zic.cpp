/** Description:
This C++ program serves as the core application engine, managing both background processes and the user interface (UI) display. It is structured using multiple threads, allowing different tasks to run simultaneously.

The program starts in the primary function, where it performs initial setup. This includes defining the visual style (like the color palette), loading necessary backend components (controllers), and reading application settings from a configuration file (typically `config.json`). Importantly, it sets up a special monitoring system to watch the configuration file; if the settings change while the program is running, it can automatically detect and handle the update. It then starts a separate "host" thread for background data management.

The visual component is handled by a dedicated UI thread. This thread initializes the drawing system, determines which screen or "view" should be displayed first (sometimes based on system settings), and renders the initial graphics.

The UI thread then enters a continuous loop, which is the heart of the application's responsiveness. In this loop, it constantly redraws the visible components and, if running on a desktop environment, checks for user events like mouse clicks. This loop ensures the screen is updated smoothly at a regular interval (about 80 milliseconds).

The code is flexible and supports two different rendering modes: one for desktop systems (which handles sophisticated user interaction) and one for embedded systems that draw directly to a screen buffer.

In summary, the program launches background services, initializes the display, and maintains a dedicated thread to continuously draw the interface and manage user interaction until the application is intentionally closed.

sha: 9b4b75cf72c101ad301595b99115aad5e0f96fa184394c49b5b3610033786736 
*/
#define ZIC_LOG_LEVEL ZIC_LOG_DEBUG

#include "config.h"
#include "draw/draw.h"
#include "helpers/configWatcher.h"
#include "helpers/getTicks.h"
#include "host.h"
#include "plugins/controllers/PixelController.h"
#include "styles.h"
#include "viewManager.h"

#include <cstdlib>
#include <pthread.h>

bool appRunning = true;

void* uiThread(void* = NULL)
{
    ViewManager& viewManager = ViewManager::get();
    viewManager.init();

    if (getenv("START_VIEW") && getenv("START_VIEW")[0] != '\0') {
        logInfo("Env variable starting view: %s", getenv("START_VIEW"));
        viewManager.setView(getenv("START_VIEW"));
    }

    viewManager.draw->fullClear();

    if (!viewManager.render()) {
        printf("No view were initialized to be rendered.");
        return NULL;
    }

    // int ms = 50;
    int ms = 80;
#ifdef DRAW_DESKTOP
    logInfo("Rendering with SDL.");
    unsigned long lastUpdate = getTicks();
    while (viewManager.draw->handleEvent(viewManager.view) && appRunning) {
        unsigned long now = getTicks();
        if (now - lastUpdate > ms) {
            lastUpdate = now;
            viewManager.draw->preRender(viewManager.view, now);
            viewManager.renderComponents(now);
        }
        usleep(1);
    }
    appRunning = false;
#else
    logDebug("Rendering framebuffer.");
    int us = ms * 1000;
    while (appRunning) {
        unsigned long now = getTicks();
        viewManager.renderComponents(now);
        usleep(us);
    }
#endif

    viewManager.draw->quit();
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
    controllers.push_back({ "Default", lastPluginControllerInstance });

    std::string configFilepath = argc >= 2 ? argv[1] : "config.json";
    loadJsonConfig(configFilepath);

    pthread_t watcherTid = configWatcher(configFilepath, &appRunning, []() {
        ViewManager& viewManager = ViewManager::get();
        Point pos = viewManager.draw->getWindowPosition();
        Size size = viewManager.draw->getWindowSize();
        printf("RESTART: %s %d,%d %d,%d\n", viewManager.view->name.c_str(), pos.x, pos.y, size.w, size.h);
        fflush(stdout);
    });

    showLogLevel();

    startHostThread();

    pthread_t ptid;
    pthread_create(&ptid, NULL, &uiThread, NULL);
    pthread_setname_np(ptid, "ui");

    // wait for uiThread to finish
    pthread_join(ptid, NULL);
    if (watcherTid != 0) {
        pthread_join(watcherTid, NULL);
    }

    return 0;
}
