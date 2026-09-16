#pragma once

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include "draw/utils/color.h"

inline Color makeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
{
    Color c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    return c;
}

enum class NodeType {
    OSC,
    FX
};

enum class OscType {
    SINE = 0,
    SQUARE,
    SAW,
    TRIANGLE,
    NOISE
};

enum class FxType {
    FILTER_LP = 0,
    FILTER_HP,
    DELAY,
    DISTORTION,
    BITCRUSHER
};

struct Position {
    float x = 0.0f;
    float y = 0.0f;
};

struct ThemeColors {
    Color oscStart;
    Color oscEnd;
    Color fxStart;
    Color fxEnd;
    Color connStart;
    Color connEnd;
    Color bgGlow;
    Color sidebarBg;
    Color accent;
};

struct Theme {
    std::string id;
    std::string name;
    ThemeColors colors;
};

static const Theme THEMES[4] = {
    {
        "deep-space",
        "Deep Space",
        {
            { 79, 70, 229, 255 },  // oscStart
            { 49, 46, 129, 255 },  // oscEnd
            { 16, 185, 129, 255 }, // fxStart
            { 19, 78, 74, 255 },   // fxEnd
            { 129, 140, 248, 255 },// connStart
            { 52, 211, 153, 255 }, // connEnd
            { 10, 10, 26, 255 },   // bgGlow
            { 15, 18, 28, 255 },   // sidebarBg
            { 79, 70, 229, 255 }   // accent
        }
    },
    {
        "cyberpunk",
        "Cyberpunk",
        {
            { 255, 0, 255, 255 },  // oscStart
            { 102, 0, 102, 255 },  // oscEnd
            { 0, 255, 255, 255 },  // fxStart
            { 0, 68, 68, 255 },    // fxEnd
            { 255, 0, 255, 255 },  // connStart
            { 0, 255, 255, 255 },  // connEnd
            { 20, 0, 20, 255 },    // bgGlow
            { 12, 12, 16, 255 },   // sidebarBg
            { 255, 0, 255, 255 }   // accent
        }
    },
    {
        "monochrome",
        "Monochrome",
        {
            { 240, 240, 240, 255 },// oscStart
            { 80, 80, 80, 255 },   // oscEnd
            { 170, 170, 170, 255 },// fxStart
            { 40, 40, 40, 255 },   // fxEnd
            { 255, 255, 255, 255 },// connStart
            { 100, 100, 100, 255 },// connEnd
            { 12, 12, 14, 255 },   // bgGlow
            { 20, 20, 24, 255 },   // sidebarBg
            { 255, 255, 255, 255 } // accent
        }
    },
    {
        "heatwave",
        "Heatwave",
        {
            { 249, 115, 22, 255 }, // oscStart
            { 124, 45, 18, 255 },  // oscEnd
            { 225, 29, 72, 255 },  // fxStart
            { 76, 5, 25, 255 },    // fxEnd
            { 251, 146, 60, 255 }, // connStart
            { 244, 63, 94, 255 },  // connEnd
            { 25, 6, 6, 255 },     // bgGlow
            { 30, 10, 10, 255 },   // sidebarBg
            { 249, 115, 22, 255 }  // accent
        }
    }
};

struct SynthNode {
    std::string id;
    NodeType type = NodeType::OSC;
    int subType = 0;
    Position pos;
    float size = 70.0f;
    float frequency = 440.0f;
    float paramA = 0.5f;
    float paramB = 0.5f;
    bool isAudible = true;
    Color color;

    float pulsePhase = 0.0f;
};

struct Connection {
    std::string id;
    std::string fromId;
    std::string toId;
};

struct CatalystParticle {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float size = 2.0f;
    Color color;
};
