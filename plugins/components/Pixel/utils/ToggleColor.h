#ifndef _UTILS_TOGGLE_COLOR_H_
#define _UTILS_TOGGLE_COLOR_H_

#include "../../drawInterface.h"
#include "../../utils/color.h"

class ToggleColor {
protected:
    uint8_t value = 1;

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

    void toggle(uint8_t _value)
    {
        value = _value;
        color = value == 1 ? color1 : color2;
    }

    void darkness(float darkenRatio)
    {
        color2 = darken(color1, darkenRatio);
        color = value == 1 ? color1 : color2;
    }

    void setColors(Color _color1, Color _color2)
    {
        color1 = _color1;
        color2 = _color2;
        color = value == 1 ? color1 : color2;
    }

    void setColor(Color _color, float darkenRatio)
    {
        color1 = _color;
        color2 = darken(_color, darkenRatio);
        color = value == 1 ? color1 : color2;
    }
};

#endif