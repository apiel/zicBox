#define DRAW_SDD1306

#include "config.h"
#include "draw/drawSDL.h"
#include "event.h"
#include "host.h"
#include "styles.h"
#include "viewManager.h"

int main(int argc, char* argv[])
{
    loadUiConfig(argc >= 2 ? argv[1] : "pixel.ui", argc >= 3 ? argv[2] : NULL);

    ViewManager& viewManager = ViewManager::get();
    viewManager.draw.init();

    viewManager.init();
    if (!viewManager.render()) {
        printf("No view were initialized to be rendered.");
        return 1;
    }

    // EventHandler& event = EventHandler::get();
    // unsigned long lastUpdate = SDL_GetTicks();
    // while (event.handle()) {
    //     unsigned long now = SDL_GetTicks();
    //     if (now - lastUpdate > 50) {
    //         lastUpdate = now;
    //         viewManager.renderComponents(now);
    //     }
    //     SDL_Delay(1);
    // }

    return 0;
}
