#ifndef _UTILS_TOGGLE_COLOR_H_
#define _UTILS_TOGGLE_COLOR_H_

#include "../../drawInterface.h"
#include "../../utils/color.h"

class ToggleColor {
public:
    Color color;
    Color color1;
    Color color2;

    ToggleColor(Color color, Color color1, Color color2)
        : color(color)
        , color1(color1)
        , color2(color2)
    {
    }

    ToggleColor(Color color, float darkenRatio)
        : color(color)
        , color1(color)
        , color2(darken(color, darkenRatio))
    {
    }

    void toggle(uint8_t value)
    {
        color = value == 1 ? color1 : color2;
    }

    void darkness(float darkenRatio)
    {
        color2 = darken(color1, darkenRatio);
    }

    void setColors(Color color1, Color color2)
    {
        this->color1 = color1;
        this->color2 = color2;
    }

    void setColor(Color color, float darkenRatio)
    {
        this->color1 = color;
        this->color2 = darken(color, darkenRatio);
    }
};

#endif