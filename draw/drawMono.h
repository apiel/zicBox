/** Description:
This file serves as a comprehensive toolkit for drawing graphics and text, primarily designed for simple, typically monochrome (black and white) displays often found in embedded systems.

**Core Structure and Building Blocks**

The header defines fundamental concepts:
1.  **Coordinates:** `Point` structures define locations using X and Y coordinates.
2.  **Dimensions:** `Size` structures define width and height.
3.  **Drawing Options:** `DrawTextOptions` allows users to specify how text should look, including the font, color (on/off), scaling, and maximum width.
4.  **Font Library:** It integrates pre-defined font data (like "PoppinsLight") in various sizes, allowing text rendering from 6-pixel high to 24-pixel high.

**The Drawing Blueprint (`DrawInterface`)**

This section acts as a standardized contract. It lists every possible drawing command (like drawing lines, rectangles, circles, arcs, and text) that a display system must support. This blueprint ensures that different display implementations can use the same commands.

**The Monochrome Engine (`DrawMono`)**

This is the specific implementation that handles the actual pixel manipulation. The `DrawMono` class works by managing a memory array, known as the "screen buffer."

**How it Works**

The engine is highly efficient for monochrome screens because it treats every bit of data in the buffer as a single pixel (on or off). When a drawing command is issued (e.g., drawing a line), the engine calculates which exact pixels need to be lit up and then modifies the corresponding bits in the memory buffer.

It provides algorithms for drawing complex shapes like:
*   Basic shapes (lines, empty and filled rectangles, circles).
*   Advanced geometry (arcs, filled polygon shapes, and filled circular "pies").

It also includes robust functions for rendering text, calculating the size of a string, and automatically centering or aligning text on the screen using the preloaded fonts.

sha: 92a50cd4d635718e3c24f5879d569121c155a4cda8bb9798d1a1241314db6d39 
*/
#pragma once

#include "fonts/Font.h" // Your font definitions (e.g., PoppinsLight_8.h)
#include "fonts/PoppinsLight_12.h"
#include "fonts/PoppinsLight_16.h"
#include "fonts/PoppinsLight_24.h"
#include "fonts/PoppinsLight_6.h"
#include "fonts/PoppinsLight_8.h"

#include <cstring>
#include <math.h>
#include <stdint.h>
#include <string>
struct Point {
    int x;
    int y;
    Point()
        : x(0)
        , y(0)
    {
    }
    Point(int x, int y)
        : x(x)
        , y(y)
    {
    }
};

struct Size {
    int w;
    int h;
    Size()
        : w(0)
        , h(0)
    {
    }
    Size(int w, int h)
        : w(w)
        , h(h)
    {
    }
};

struct DrawTextOptions {
    const Font* font = nullptr; // font to use
    bool color = true; // true = set pixel, false = clear pixel
    int maxWidth = 0; // max width in pixels (0 = no limit)
    int scale = 1; // scale of characters
    int fontSpacing = 1; // pixels between characters
};

class DrawInterface {
public:
    virtual const Size& getSize() = 0;
    virtual void renderNext() = 0;
    virtual bool shouldRender() = 0;
    virtual void clear() = 0;
    virtual void setPixel(Point p, bool color = true) = 0;

    virtual void line(Point a, Point b, bool color = true) = 0;
    virtual void lines(const Point* pts, int count, bool color = true) = 0;
    virtual void rect(Point pos, Size size, bool color = true) = 0;
    virtual void filledRect(Point pos, Size size, bool color = true) = 0;
    virtual void circle(Point c, int r, bool color = true) = 0;
    virtual void filledCircle(Point c, int r, bool color = true) = 0;
    virtual void arc(Point c, int r, float startAngle, float endAngle, bool color = true) = 0;
    virtual void filledPie(Point c, int r, float startAngle, float endAngle, bool color = true) = 0;
    virtual void filledPolygon(const Point* points, int count, bool color = true) = 0;
    virtual int getTextWidth(const std::string& str, const DrawTextOptions& opts = {}) = 0;
    virtual int text(Point pos, const std::string& str, const DrawTextOptions& opts = {}) = 0;
    virtual void textCentered(Point pos, const std::string& str, const DrawTextOptions& opts = {}) = 0;
    virtual int textRight(Point pos, const std::string& str, const DrawTextOptions& opts = {}) = 0;
    virtual void* font(std::string& name) = 0;
};

//
// Generic Monochrome Drawing Class
//
template <int WIDTH, int HEIGHT>
class DrawMono : public DrawInterface {
protected:
    bool needRendering = false;
    const Size size = { WIDTH, HEIGHT };

public:
    uint8_t screenBuffer[WIDTH * HEIGHT / 8];
    uint8_t textThreshold = 128; // grayscale threshold (0–255)

    DrawMono() { clear(); }

    const Size& getSize() override { return size; }
    void renderNext() override { needRendering = true; }

    bool shouldRender() override
    {
        if (needRendering) {
            needRendering = false;
            return true;
        }
        return false;
    }

    //-------------------------------
    // Framebuffer control
    //-------------------------------
    void clear() override { memset(screenBuffer, 0x00, sizeof(screenBuffer)); }

    void setPixel(Point p, bool color = true) override
    {
        if ((uint32_t)p.x >= WIDTH || (uint32_t)p.y >= HEIGHT)
            return;
        uint32_t index = p.x + (p.y / 8) * WIDTH;
        uint8_t bit = 1 << (p.y & 7);
        if (color)
            screenBuffer[index] |= bit;
        else
            screenBuffer[index] &= ~bit;
    }

    bool getPixel(Point p) { return (screenBuffer[p.x + (p.y / 8) * WIDTH] & (1 << (p.y & 7))) != 0; }

    //-------------------------------
    // Primitives
    //-------------------------------
    void line(Point a, Point b, bool color = true) override
    {
        int x0 = a.x, y0 = a.y, x1 = b.x, y1 = b.y;
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;

        while (true) {
            setPixel({ x0, y0 }, color);
            if (x0 == x1 && y0 == y1)
                break;
            e2 = 2 * err;
            if (e2 >= dy) {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y0 += sy;
            }
        }
    }

    void lines(const Point* pts, int count, bool color = true) override
    {
        for (int i = 0; i < count - 1; i++)
            line(pts[i], pts[i + 1], color);
    }

    void rect(Point pos, Size size, bool color = true) override
    {
        Point p1 = pos;
        Point p2 = { pos.x + size.w - 1, pos.y };
        Point p3 = { pos.x + size.w - 1, pos.y + size.h - 1 };
        Point p4 = { pos.x, pos.y + size.h - 1 };
        line(p1, p2, color);
        line(p2, p3, color);
        line(p3, p4, color);
        line(p4, p1, color);
    }

    void filledRect(Point pos, Size size, bool color = true) override
    {
        for (int y = pos.y; y < pos.y + size.h; y++)
            line({ pos.x, y }, { pos.x + size.w - 1, y }, color);
    }

    void circle(Point c, int r, bool color = true) override
    {
        int f = 1 - r;
        int dx = 1;
        int dy = -2 * r;
        int x = 0;
        int y = r;

        setPixel({ c.x, c.y + r }, color);
        setPixel({ c.x, c.y - r }, color);
        setPixel({ c.x + r, c.y }, color);
        setPixel({ c.x - r, c.y }, color);

        while (x < y) {
            if (f >= 0) {
                y--;
                dy += 2;
                f += dy;
            }
            x++;
            dx += 2;
            f += dx;
            setPixel({ c.x + x, c.y + y }, color);
            setPixel({ c.x - x, c.y + y }, color);
            setPixel({ c.x + x, c.y - y }, color);
            setPixel({ c.x - x, c.y - y }, color);
            setPixel({ c.x + y, c.y + x }, color);
            setPixel({ c.x - y, c.y + x }, color);
            setPixel({ c.x + y, c.y - x }, color);
            setPixel({ c.x - y, c.y - x }, color);
        }
    }

    void filledCircle(Point c, int r, bool color = true) override
    {
        int f = 1 - r;
        int dx = 1;
        int dy = -2 * r;
        int x = 0;
        int y = r;

        line({ c.x - r, c.y }, { c.x + r, c.y }, color);

        while (x < y) {
            if (f >= 0) {
                y--;
                dy += 2;
                f += dy;
            }
            x++;
            dx += 2;
            f += dx;
            line({ c.x - x, c.y + y }, { c.x + x, c.y + y }, color);
            line({ c.x - x, c.y - y }, { c.x + x, c.y - y }, color);
            line({ c.x - y, c.y + x }, { c.x + y, c.y + x }, color);
            line({ c.x - y, c.y - x }, { c.x + y, c.y - x }, color);
        }
    }

    void arc(Point c, int r, float startAngle, float endAngle, bool color = true) override
    {
        int steps = std::max(4, (int)(r * fabsf(endAngle - startAngle)));
        for (int i = 0; i <= steps; i++) {
            float a = startAngle + (endAngle - startAngle) * (float)i / steps;
            int x = c.x + r * cosf(a);
            int y = c.y + r * sinf(a);
            setPixel({ x, y }, color);
        }
    }

    void filledPie(Point c, int r, float startAngle, float endAngle, bool color = true) override
    {
        int steps = std::max(4, (int)(r * fabsf(endAngle - startAngle)));
        for (int i = 0; i <= steps; i++) {
            float a = startAngle + (endAngle - startAngle) * (float)i / steps;
            int x = c.x + r * cosf(a);
            int y = c.y + r * sinf(a);
            line(c, { x, y }, color);
        }
    }

    void filledPolygon(const Point* points, int count, bool color = true) override
    {
        for (int y = 0; y < HEIGHT; ++y) {
            int nodeX[64];
            int nodes = 0;
            for (int i = 0, j = count - 1; i < count; j = i++) {
                int xi = points[i].x, yi = points[i].y;
                int xj = points[j].x, yj = points[j].y;
                if ((yi < y && yj >= y) || (yj < y && yi >= y))
                    nodeX[nodes++] = xi + (y - yi) * (xj - xi) / (yj - yi);
            }
            // sort intersections
            for (int i = 0; i < nodes - 1; i++)
                for (int j = i + 1; j < nodes; j++)
                    if (nodeX[i] > nodeX[j])
                        std::swap(nodeX[i], nodeX[j]);
            for (int i = 0; i < nodes; i += 2)
                if (i + 1 < nodes)
                    line({ nodeX[i], y }, { nodeX[i + 1], y }, color);
        }
    }

    //-------------------------------
    // Text rendering
    //-------------------------------
    int getTextWidth(const std::string& str, const DrawTextOptions& opts = {}) override
    {
        const uint8_t** font = getFont(opts);
        int width = 0;
        for (char c : str) {
            const uint8_t* charPtr = font[1 + (c - ' ')];
            width += charPtr[0] + opts.fontSpacing;
        }
        return width;
    }

    int drawChar(Point pos, const uint8_t* charPtr, int width, bool color, int scale = 1)
    {
        uint8_t marginTop = charPtr[1] * scale;
        uint8_t rows = charPtr[2];
        charPtr += 3;
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < width; col++) {
                uint8_t a = charPtr[col + row * width];
                if (a > 64) { // threshold to draw pixels
                    setPixel({ pos.x + col * scale, pos.y + row * scale + marginTop }, color);
                }
            }
        }
        return width * scale;
    }

    int text(Point pos, const std::string& str, const DrawTextOptions& opts = {}) override
    {
        const uint8_t** font = getFont(opts);

        int x = pos.x;
        int maxX = opts.maxWidth ? opts.maxWidth + x : WIDTH;
        for (char c : str) {
            const uint8_t* charPtr = font[1 + (c - ' ')];
            uint8_t width = charPtr[0];

            if (x + width * opts.scale > maxX)
                break;

            x += drawChar({ x, pos.y }, charPtr, width, opts.color, opts.scale) + opts.fontSpacing;
        }
        return x;
    }

    void textCentered(Point pos, const std::string& str, const DrawTextOptions& opts = {}) override
    {
        int width = getTextWidth(str, opts);
        Point start = { pos.x - width / 2, pos.y };
        text(start, str, opts);
    }

    int textRight(Point pos, const std::string& str, const DrawTextOptions& opts = {}) override
    {
        int width = getTextWidth(str, opts);
        Point start = { pos.x - width, pos.y };
        text(start, str, opts);
        return start.x;
    }

    Font* DEFAULT_FONT = &PoppinsLight_8;
    void* font(std::string& name) override
    {
        if (name == "PoppinsLight_6") {
            return &PoppinsLight_6;
        } else if (name == "PoppinsLight_8") {
            return &PoppinsLight_8;
        } else if (name == "PoppinsLight_12") {
            return &PoppinsLight_12;
        } else if (name == "PoppinsLight_16") {
            return &PoppinsLight_16;
        } else if (name == "PoppinsLight_24") {
            return &PoppinsLight_24;
        }
        return DEFAULT_FONT;
    }

protected:
    const uint8_t** getFont(const DrawTextOptions& options)
    {
        if (options.font) {
            return (const uint8_t**)((Font*)options.font)->data;
        }
        return (const uint8_t**)DEFAULT_FONT->data;
    }
};
