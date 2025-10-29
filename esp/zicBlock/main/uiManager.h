#pragma once

#include "audio.h"
#include "draw/drawMono.h"
#include "helpers/enc.h"
#include "log.h"
#include "clapView.h"
#include "toneView.h"
#include "stringView.h"
#include "fxView.h"
class UIManager {
public:
    Audio& audio = Audio::get();

    static const uint8_t encoderCount = 9;
    static const int width = 128;
    static const int height = 128;

    DrawMono<width, height> draw;

    ToneView toneView;
    ClapView clapView;
    StringView stringView;
    FxView fxView;

    View* currentView = &toneView;

    UIManager()
        : toneView(draw)
        , clapView(draw)
        , stringView(draw)
        , fxView(draw)
    {
        currentView->render();
        draw.renderNext();
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
                    currentView = &toneView;
                    draw.renderNext();
                }
            } else if (key == 22) { // s
                if (state == 0) {
                    currentView = &clapView;
                    draw.renderNext();
                }
            } else if (key == 7) { // d
                if (state == 0) {
                    currentView = &stringView;
                    draw.renderNext();
                }
            } else if (key == 9) { // f
                if (state == 0) {
                    currentView = &fxView;
                    draw.renderNext();
                }
            } else if (key == 29) { // z
                if (state == 1) {
                    audio.noteOn(60, 1.0f);
                } else {
                    audio.noteOff(60);
                }
            }
        }
    }
};