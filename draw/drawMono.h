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

//
// Generic Monochrome Drawing Class
//
template <int WIDTH, int HEIGHT>
class DrawMono {
public:
    uint8_t screenBuffer[WIDTH * HEIGHT / 8];
    uint8_t textThreshold = 128; // grayscale threshold (0–255)

    DrawMono() { clear(); }

    //-------------------------------
    // Framebuffer control
    //-------------------------------
    inline void clear() { memset(screenBuffer, 0x00, sizeof(screenBuffer)); }
    inline void fill() { memset(screenBuffer, 0xFF, sizeof(screenBuffer)); }

    inline void setPixel(Point p, bool color = true)
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

    inline void pixel(Point p, bool color = true) { setPixel(p, color); }

    bool getPixel(Point p) { return (screenBuffer[p.x + (p.y / 8) * WIDTH] & (1 << (p.y & 7))) != 0; }

    //-------------------------------
    // Primitives
    //-------------------------------
    void line(Point a, Point b, bool color = true)
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

    void lines(const Point* pts, int count, bool color = true)
    {
        for (int i = 0; i < count - 1; i++)
            line(pts[i], pts[i + 1], color);
    }

    void rect(Point pos, Size size, bool color = true)
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

    void filledRect(Point pos, Size size, bool color = true)
    {
        for (int y = pos.y; y < pos.y + size.h; y++)
            line({ pos.x, y }, { pos.x + size.w - 1, y }, color);
    }

    void circle(Point c, int r, bool color = true)
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

    void filledCircle(Point c, int r, bool color = true)
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

    void arc(Point c, int r, float startAngle, float endAngle, bool color = true)
    {
        int steps = std::max(4, (int)(r * fabsf(endAngle - startAngle)));
        for (int i = 0; i <= steps; i++) {
            float a = startAngle + (endAngle - startAngle) * (float)i / steps;
            int x = c.x + r * cosf(a);
            int y = c.y + r * sinf(a);
            setPixel({ x, y }, color);
        }
    }

    void filledPie(Point c, int r, float startAngle, float endAngle, bool color = true)
    {
        int steps = std::max(4, (int)(r * fabsf(endAngle - startAngle)));
        for (int i = 0; i <= steps; i++) {
            float a = startAngle + (endAngle - startAngle) * (float)i / steps;
            int x = c.x + r * cosf(a);
            int y = c.y + r * sinf(a);
            line(c, { x, y }, color);
        }
    }

    void filledPolygon(const Point* points, int count, bool color = true)
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
    int getTextWidth(const std::string& str, const DrawTextOptions& opts = {})
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
                    pixel({ pos.x + col * scale, pos.y + row * scale + marginTop }, color);
                }
            }
        }
        return width * scale;
    }

    int text(Point pos, const std::string& str, const DrawTextOptions& opts = {})
    {
        const uint8_t** font = getFont(opts);

        int x = pos.x;
        int maxX = opts.maxWidth ? opts.maxWidth : WIDTH;
        for (char c : str) {
            const uint8_t* charPtr = font[1 + (c - ' ')];
            uint8_t width = charPtr[0];

            if (x + width * opts.scale > maxX)
                break;

            x += drawChar({ x, pos.y }, charPtr, width, opts.color, opts.scale) + opts.fontSpacing;
        }
        return x;
    }

    void textCentered(Point pos, const std::string& str, const DrawTextOptions& opts = {})
    {
        int width = getTextWidth(str, opts);
        Point start = { pos.x - width / 2, pos.y };
        text(start, str, opts);
    }

    int textRight(Point pos, const std::string& str, const DrawTextOptions& opts = {})
    {
        int width = getTextWidth(str, opts);
        Point start = { pos.x - width, pos.y };
        text(start, str, opts);
        return start.x;
    }

    Font* DEFAULT_FONT = &PoppinsLight_8;
    void* font(std::string& name)
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
