#pragma once

#include <cstdint>

enum FontType {
    COMPILED_FONT = 0,
    TTF_FONT
};

struct Font {
    void *data;
    FontType type = COMPILED_FONT;
};