#ifndef _COLOR_H_
#define _COLOR_H_

#include "../drawInterface.h"

#include <string>

Color hex2rgb(char* hex)
{

    hex++;
    unsigned int color = strtol(hex, NULL, 16);
    return Color({
        .r = (uint8_t)((color & 0x00FF0000) >> 16),
        .g = (uint8_t)((color & 0x0000FF00) >> 8),
        .b = (uint8_t)((color & 0x000000FF)),
        .a = 255,
    });
}

std::string rgbToString(Color color)
{
    char buffer[8];
    sprintf(buffer, "#%02X%02X%02X", color.r, color.g, color.b);
    return buffer;
}

Color darken(Color color, float amount)
{
    return {
        (uint8_t)(color.r * (1.0f - amount)),
        (uint8_t)(color.g * (1.0f - amount)),
        (uint8_t)(color.b * (1.0f - amount)),
        color.a
    };
}

float min(float a, float b)
{
    return a < b ? a : b;
}

Color lighten(Color color, float amount)
{
    return {
        (uint8_t)min(color.r * (1.0f + amount), 255.0f),
        (uint8_t)min(color.g * (1.0f + amount), 255.0f),
        (uint8_t)min(color.b * (1.0f + amount), 255.0f),
        color.a
    };
}

Color alpha(Color color, float amount)
{
    return {
        color.r,
        color.g,
        color.b,
        (uint8_t)(amount * 255.0f),
    };
}

#endif
