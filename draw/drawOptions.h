#pragma once

#include "draw/baseInterface.h"

struct DrawOptions {
    Color color = { 255, 255, 255, 255 };
    int thickness = 1;
};

struct DrawTextOptions {
    Color color = { 255, 255, 255, 255 };
    void* font = nullptr;
    int maxWidth = 0;
    int fontHeight = 0; // Compiled
    int fontSpacing = 1; // Ttf
    Color color2 = { 255, 255, 255, 255 };
};
