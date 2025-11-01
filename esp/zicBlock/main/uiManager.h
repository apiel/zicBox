#pragma once

#include "audio.h"
#include "draw/drawMono.h"
#include "helpers/enc.h"
#include "log.h"

#include "clapView.h"
#include "engineView.h"
#include "fxView.h"
#include "metalicView.h"
#include "snareHatView.h"
#include "stringView.h"
#include "toneView.h"

#include "uiManagerInterface.h"

class UIManager : public UIManagerInterface {
public:
    Audio& audio = Audio::get();

    static const uint8_t encoderCount = 9;
    static const int width = 128;
    static const int height = 128;

    DrawMono<width, height> draw;

    EngineView engineView;
    ToneView toneView;
    ClapView clapView;
    StringView stringView;
    MetalicView metalicView;
    SnareHatView snareHatView;
    FxView fxView;

    View* currentView = &toneView;

    static const uint8_t engineCount = 5;
    struct EngineAndView {
        View& view;
        Engine& engine;
    } engineAndViews[engineCount] = {
        { toneView, audio.tone },
        { clapView, audio.clap },
        { stringView, audio.drumString },
        { metalicView, audio.metalic },
        { snareHatView, audio.snareHat },
    };
    uint8_t selectedEngine = 0;
    View* selectedEngineView = &engineAndViews[selectedEngine].view;

    UIManager()
        : engineView(draw, *this)
        , toneView(draw)
        , clapView(draw)
        , stringView(draw)
        , metalicView(draw)
        , snareHatView(draw)
        , fxView(draw)
    {
        currentView->render();
        draw.renderNext();
        selectEngine(selectedEngine);
    }

    uint8_t getSelectedEngine() override { return selectedEngine; }
    uint8_t getEngineCount() override { return engineCount; }
    void selectEngine(uint8_t index) override
    {
        selectedEngine = index;
        selectedEngineView = &engineAndViews[selectedEngine].view;
        audio.setEngine(&engineAndViews[selectedEngine].engine);
    }

    bool render()
    {
        bool shouldRender = draw.shouldRender();
        if (shouldRender) {
            currentView->render();
        }
        return shouldRender;
    }

    uint64_t lastEncoderTick[encoderCount] = { 0 };
    void onEncoder(int id, int8_t direction, uint64_t tick)
    {
        int scaledDirection = encGetScaledDirection(direction, tick, lastEncoderTick[id]);
        lastEncoderTick[id] = tick;
        logDebug("onEncoder %d dir:%d", id, scaledDirection);
        currentView->onEncoder(id, scaledDirection, tick);
    }

    void onKey(uint16_t id, int key, int8_t state)
    {
        if (!currentView->onKey(id, key, state)) {
            logDebug("onKey id %d key %d state %d", id, key, state);
            if (key == 4) { // a
                if (state == 0) {
                    setView(toneView);
                } else {
                    setView(engineView);
                }
            } else if (key == 22) { // s
                if (state == 0) {
                    setView(clapView);
                }
            } else if (key == 7) { // d
                if (state == 0) {
                    setView(stringView);
                }
            } else if (key == 9) { // f
                if (state == 0) {
                    setView(fxView);
                }
            } else if (key == 29) { // z
                if (state == 1) {
                    audio.noteOn(60, 1.0f);
                } else {
                    audio.noteOff(60);
                }
            } else if (key == 27) { // x
                if (state == 0) {
                    setView(metalicView);
                }
            } else if (key == 6) { // c
                if (state == 0) {
                    setView(snareHatView);
                }
            }
        }
    }

    void setView(View& view) override
    {
        currentView = &view;
        draw.renderNext();
    }
};