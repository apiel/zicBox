/** Description:
This C++ program serves as the core startup and execution engine for an application, likely one that involves a real-time graphical user interface (GUI). It is structured around handling continuous rendering, configuration, and concurrent operations using background processes.

The application begins by establishing its identity: loading necessary system plugins, defining the overall visual color palette, and initializing a controller for managing pixel output. Crucially, it loads its operational settings from a configuration file, typically named `config.json`.

Two main tasks are launched into parallel "threads" (independent background processes):

1.  **The Configuration Watcher:** A process that continuously monitors the settings file. If a user modifies this file while the application is running, the system detects the change and logs information about the current state, preparing for potential application restarts or reloads.
2.  **The User Interface Thread:** This is the heart of the application, responsible for everything the user sees. It initializes the graphical environment and decides which view or screen should be displayed first (sometimes based on environment settings). It enters a continuous rendering loop, repeatedly refreshing the display elements (components) and handling user input, such as mouse clicks or key presses. The rendering speed is optimized based on whether the application is running on a full desktop environment or a simple embedded display.

The main program waits for the user interface thread to finish—which happens when the application is properly closed—before gracefully shutting down all parallel processes and exiting.

sha: ecfc97f91fee10781220c535c19e5b6afe0c70576502222ffbe2810ac9c94a50 
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
            viewManager.draw->preRender(viewManager.view);
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
