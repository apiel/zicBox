#pragma once

#include "draw/drawMono.h"

class View {
public:
    DrawInterface& draw;

    View(DrawInterface& draw)
        : draw(draw)
    {
    }

    virtual void render() = 0;
    virtual void onEncoder(int id, int8_t direction, uint64_t tick) = 0;
    virtual bool onKey(uint16_t id, int key, int8_t state) = 0;
};