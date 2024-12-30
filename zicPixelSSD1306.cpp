
/*
buildPixelSSD1306:
	@echo "\n------------------ build zicPixel SSD1306 ------------------\n"
	g++ -g -fms-extensions -o pixelSSD1306.$(BIN_SUFFIX) zicPixelSSD1306.cpp -ldl $(INC) $(RPI) $(RTMIDI) $(PIXEL_SDL) $(LUA) $(shell pkg-config --cflags --libs sndfile) $(shell pkg-config --cflags --libs libpulse-simple)

runPixelSSD1306:
	@echo "\n------------------ run zicPixel ------------------\n"
	./pixelSSD1306.$(BIN_SUFFIX)

*/

#define ZIC_LOG_LEVEL ZIC_LOG_DEBUG
#define DRAW_SSD1306

#ifndef IS_RPI
#define USE_DRAW_WITH_SDL
#endif

#ifdef USE_DRAW_WITH_SDL
// Define before config else ADD_ZONE_ENCODER not working
#include "SDL_EventHandler.h"
#endif

#include "config.h"
#include "draw/SSD1306/draw.h"
#include "host.h"
#include "plugins/components/PixelSSD1306/PixelComponents.h"
#include "styles.h"
#include "helpers/getTicks.h"
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

void loadAudioAliases()
{
    AudioPluginHandler& audioPluginHandler = AudioPluginHandler::get();
    audioPluginHandler.pluginAliases.push_back({ "SynthDrum23", [](AudioPlugin::Props& props, char* name) {
                                                    return new SynthDrum23(props, name);
                                                } });
    audioPluginHandler.pluginAliases.push_back({ "Tempo", [](AudioPlugin::Props& props, char* name) {
                                                    return new Tempo(props, name);
                                                } });
    audioPluginHandler.pluginAliases.push_back({ "Sequencer", [](AudioPlugin::Props& props, char* name) {
                                                    return new Sequencer(props, name);
                                                } });
    audioPluginHandler.pluginAliases.push_back({ "EffectDistortion", [](AudioPlugin::Props& props, char* name) {
                                                    return new EffectDistortion(props, name);
                                                } });
    audioPluginHandler.pluginAliases.push_back({ "EffectFilterMultiMode", [](AudioPlugin::Props& props, char* name) {
                                                    return new EffectFilterMultiMode(props, name);
                                                } });
    audioPluginHandler.pluginAliases.push_back({ "SerializeTrack", [](AudioPlugin::Props& props, char* name) {
                                                    return new SerializeTrack(props, name);
                                                } });
    audioPluginHandler.pluginAliases.push_back({ "EffectGainVolume", [](AudioPlugin::Props& props, char* name) {
                                                    return new EffectGainVolume(props, name);
                                                } });
    audioPluginHandler.pluginAliases.push_back({ "Mixer4", [](AudioPlugin::Props& props, char* name) {
                                                    return new Mixer4(props, name);
                                                } });
    audioPluginHandler.pluginAliases.push_back({ "Mixer2", [](AudioPlugin::Props& props, char* name) {
                                                    return new Mixer2(props, name);
                                                } });
    audioPluginHandler.pluginAliases.push_back({ "AudioOutputPulse", [](AudioPlugin::Props& props, char* name) {
                                                    return new AudioOutputPulse(props, name);
                                                } });
}

int main(int argc, char* argv[])
{
    loadPixelComponents();
    loadAudioAliases();

    styles.screen = { 128, 64 };
    loadUiConfig(argc >= 2 ? argv[1] : "pixelSSD1306.cfg", argc >= 3 ? argv[2] : NULL, styles);

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
