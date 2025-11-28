/** Description:
This C++ program serves as the core startup engine for an application featuring a graphical user interface (GUI) and background services.

The main objective is to manage complex tasks simultaneously by utilizing parallel processing threads.

**Setup and Initialization:**
The program begins by defining the application's visual style, establishing a specific color palette. It then loads operational settings from a configuration file, typically `config.json`, and initializes specialized components, such as a dedicated data `Controller`. It also sets up the central `ViewManager`, which handles all screen rendering.

**Parallel Workers (Threads):**
The application relies on three main workers running concurrently:
1.  **The UI Worker:** This is the display engine. It initializes the screen system and enters a continuous loop that constantly redraws and updates all visible elements. Depending on the environment, this loop also captures all user input (like mouse clicks or keyboard presses) and keeps the application running until the user quits.
2.  **The Configuration Watcher:** This worker monitors the settings file. If the file is modified while the application is running, it alerts the system that a reload or configuration adjustment may be needed.
3.  **The Host Worker:** This worker manages background services and plugins required for the application's deeper functionality.

The main program sequence starts all these workers, then waits patiently for the primary UI Worker to complete (usually when the application window is closed). This controlled wait ensures all background services are properly shut down before the entire program terminates cleanly.

sha: 67aee60ee8ac5aed823ee363d15df607da94ee94867b0d0e1ef53db4d7ab1876 
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
