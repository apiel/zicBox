#pragma once

#include "draw/drawMono.h"
#include "helpers/enc.h"
#include "log.h"
#include "mainView.h"

class UIManager {
public:
    static const uint8_t encoderCount = 9;
    static const int width = 128;
    static const int height = 128;

    DrawMono<width, height> draw;

    MainView mainView;

    View& currentView = mainView;

    UIManager()
        : mainView(draw)
    {
        currentView.render();
    }

    bool shouldRender()
    {
        return draw.shouldRender();
    }

    uint32_t lastEncoderTick[encoderCount] = { 0 };
    void onEncoder(int id, int8_t direction, uint32_t tick)
    {
        int scaledDirection = encGetScaledDirection(direction, tick, lastEncoderTick[id]);
        lastEncoderTick[id] = tick;
        logDebug("onEncoder %d dir:%d", id, scaledDirection);
        currentView.onEncoder(id, scaledDirection, tick);
    }

    void onKey(uint16_t id, int key, int8_t state)
    {
        if (!currentView.onKey(id, key, state)) {
            logDebug("onKey id %d key %d state %d", id, key, state);
        }
    }
};