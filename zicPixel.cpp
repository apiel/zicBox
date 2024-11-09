#define DRAW_SSD1306

#include "config.h"
#include "draw/SSD1306/draw.h"
#include "host.h"
#include "styles.h"
#include "timer.h"
#include "viewManager.h"

int main(int argc, char* argv[])
{
    styles.screen = { 128, 64 };
    loadUiConfig(argc >= 2 ? argv[1] : "pixel.ui", argc >= 3 ? argv[2] : NULL);

    ViewManager& viewManager = ViewManager::get();
    viewManager.draw.init();

    viewManager.init();
    if (!viewManager.render()) {
        printf("No view were initialized to be rendered.");
        return 1;
    }

    unsigned long lastUpdate = getTicks();
    while (true) {
        unsigned long now = getTicks();
        if (now - lastUpdate > 50) {
            lastUpdate = now;
            viewManager.renderComponents(now);
        }
        usleep(1);
    }

    return 0;
}
