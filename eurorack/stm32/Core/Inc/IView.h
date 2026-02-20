#pragma once

#include "draw/drawPrimitives.h"

class IView {
public:
    virtual ~IView() { }
    virtual void onEncoder(int dir) = 0;
    virtual void onButton() = 0;
    virtual void render(DrawPrimitives& display) = 0;
};