#pragma once

#include "draw/drawMono.h"
#include "helpers/enc.h"
#include "log.h"

class UIManager {
public:
    static const uint8_t encoderCount = 9;
    static const int width = 128;
    static const int height = 128;

    DrawMono<width, height> draw;

    UIManager()
    {
        draw.line({ 0, 0 }, { 50, 50 });
        draw.text({ 20, 0 }, "Hello World");
        draw.textRight({ 128, 10 }, "Right");
        draw.textCentered({ 64, 20 }, "Centered");
        draw.rect({ 50, 50 }, { 10, 10 });
        draw.filledRect({ 60, 60 }, { 10, 10 });
        draw.circle({ 75, 75 }, 5);
        draw.filledCircle({ 85, 85 }, 5);
        draw.renderNext();
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
    }

    void onKey(uint16_t id, int key, int8_t state)
    {
        logDebug("onKey id %d key %d state %d", id, key, state);
    }
};