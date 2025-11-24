/** Description:
This header file serves as a core definition library, providing standardized data structures essential for handling two-dimensional geometry, positioning, and color representation within a program, typically for graphics or user interfaces.

**Core Geometric Definitions:**

1.  **Point and Size:** These structures define the fundamental building blocks. A **Point** specifies a single location using horizontal (X) and vertical (Y) coordinates. **Size** defines the extent or dimension, using width (W) and height (H).

2.  **Rectangles (Rect and Xywh):** A rectangle defines a specific area. The file provides two ways to structure this data:
    *   **Rect** combines a defined **Point** (usually the top-left corner) and a **Size** structure.
    *   **Xywh** defines the rectangle using four separate numbers for X, Y, W, and H. Both achieve the same goal but offer flexibility in how the program manages the data.

**Color Definition:**

The **Color** structure standardizes how visual appearance is handled. It uses four components—Red, Green, Blue, and Alpha (transparency). These values are stored as highly efficient 8-bit integers. By default, a new color is initialized as solid white (all color components and alpha set to maximum).

In summary, this file ensures that every part of the application uses consistent terminology when dealing with screen positions, dimensions, and visual styles.

sha: a4215c89b6aea84ce5c1213eced42afac3a743e49a53b0d8f50660052b7a7202 
*/
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