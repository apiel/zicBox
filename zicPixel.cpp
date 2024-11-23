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

#include "plugins/audio/AudioOutputPulse.h"
#include "plugins/audio/EffectDistortion.h"
#include "plugins/audio/EffectFilterMultiMode.h"
#include "plugins/audio/EffectGainVolume.h"
#include "plugins/audio/Mixer2.h"
#include "plugins/audio/Mixer4.h"
#include "plugins/audio/Sequencer.h"
#include "plugins/audio/SerializeTrack.h"
#include "plugins/audio/SynthDrum23.h"
#include "plugins/audio/Tempo.h"

// Make from scratch UI

int main(int argc, char* argv[])
{
    loadPixelComponents();

    styles.screen = { 240, 240 };
    loadUiConfig(argc >= 2 ? argv[1] : "pixel.ui", argc >= 3 ? argv[2] : NULL);

    showLogLevel();

    // might want to load host using so, in order to save compile time by removing $(shell pkg-config --cflags --libs libpulse-simple)
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
