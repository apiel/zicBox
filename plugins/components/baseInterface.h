#pragma once

#include <cstdint>

struct Size {
    int w;
    int h;
};

struct Point {
    int x;
    int y;
};

struct Rect {
    Point position;
    Size size;
};

struct Xywh {
    int x;
    int y;
    int w;
    int h;
};

typedef struct Color {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
} Color;