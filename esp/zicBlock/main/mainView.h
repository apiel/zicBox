#pragma once

#include "view.h"

class MainView : public View {
public:
    MainView(DrawInterface& draw)
        : View(draw)
    {
    }

    void render() override
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
    void onEncoder(int id, int8_t direction, uint32_t tick) override
    {
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};