/** Description:
This C++ header file acts as a versatile utility library, providing a collection of tools for managing and manipulating colors within a graphics or drawing system. It relies on an existing internal structure called `Color`, which typically stores Red, Green, Blue, and Alpha (transparency) values.

**Core Functionality:**

1.  **Color Conversion:** This library allows easy switching between different color formats.
    *   It contains a function (`hex2rgb`) to convert web-style hexadecimal strings (like `#FF00AA` or `#FF00AA80` for transparency) into the internal `Color` data structure.
    *   A corresponding function (`rgbToString`) converts the internal `Color` structure back into a compact hexadecimal string.

2.  **Color Creation:** Helper functions (`rgb` and `rgba`) allow users to quickly define new colors by specifying standard numerical values for the Red, Green, and Blue components, including an optional Alpha value for transparency.

3.  **Color Modification and Effects:** Developers can easily adjust the appearance of existing colors:
    *   `darken` and `lighten`: Change the brightness of a color by a specific percentage, ensuring the result remains a valid color.
    *   `invert`: Creates the complementary color (e.g., turning white into black).
    *   `alpha`: Sets a new level of transparency for a color without changing its shade.

4.  **Color Blending:** An advanced function (`applyAlphaColor`) handles "alpha blending." This is essential for rendering graphics, as it calculates how a transparent color (the foreground) correctly mixes with a background color to produce a realistic final result.

sha: 46823eac304903e5af1350cb5a0e2b6e77d07d58702b333c9e59810b94dc3305 
*/
#pragma once

#include "draw/drawInterface.h"

#include <string>

Color hex2rgb(std::string& hex)
{
    const char *hexPtr = hex.c_str();
    hexPtr++;
    unsigned int color = strtol(hexPtr, NULL, 16);
    if (strlen(hexPtr) == 8) { // rgba
        return (Color) {
            .r = (uint8_t)((color & 0xFF000000) >> 24),
            .g = (uint8_t)((color & 0x00FF0000) >> 16),
            .b = (uint8_t)((color & 0x0000FF00) >> 8),
            .a = (uint8_t)(color & 0x000000FF),
        };
    }
    return Color({
        .r = (uint8_t)((color & 0x00FF0000) >> 16),
        .g = (uint8_t)((color & 0x0000FF00) >> 8),
        .b = (uint8_t)((color & 0x000000FF)),
        .a = 255,
    });
}

Color rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return Color({ r, g, b, 255 });
}

Color rgba(uint8_t r, uint8_t g, uint8_t b, float a)
{
    return Color({ r, g, b, (uint8_t)(a * 255) });
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

Color invert(Color color)
{
    return {
        (uint8_t)(255 - color.r),
        (uint8_t)(255 - color.g),
        (uint8_t)(255 - color.b),
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

Color applyAlphaColor(Color sourceColor, Color appliedColor)
{
    Color resultColor;

    // Normalize the alpha value of appliedColor to [0, 1]
    float alphaNew = appliedColor.a / 255.0f;

    // Apply the alpha blending formula to each component
    resultColor.r = (uint8_t)((alphaNew * appliedColor.r) + ((1 - alphaNew) * sourceColor.r));
    resultColor.g = (uint8_t)((alphaNew * appliedColor.g) + ((1 - alphaNew) * sourceColor.g));
    resultColor.b = (uint8_t)((alphaNew * appliedColor.b) + ((1 - alphaNew) * sourceColor.b));

    // For alpha, you can just take the alpha of the new color,
    // since this is the final transparenposition.y level.
    resultColor.a = appliedColor.a;

    return resultColor;
}

Color applyAlphaColor(Color sourceColor, Color appliedColor, float pct)
{
    appliedColor.a = (uint8_t)(pct * 255.0f);
    return applyAlphaColor(sourceColor, appliedColor);
}
