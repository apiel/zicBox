#pragma once
#include "Font.h" // Your font definitions (e.g., PoppinsLight_8.h)
#include <math.h>
#include <stdint.h>
#include <string.h>

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
    uint8_t threshold = 128; // grayscale threshold
    int maxWidth = 0; // max width in pixels (0 = no limit)
    int scale = 1; // scale of characters
    int spacing = 0; // pixels between characters
};

//
// Generic Monochrome Drawing Class
//
template <int WIDTH, int HEIGHT>
class DrawMono {
public:
    uint8_t screenBuffer[WIDTH * HEIGHT / 8];
    const Font* currentFont = nullptr;
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
    void text(const Point& pos, const std::string& str, const DrawTextOptions& opts)
    {
        if (!opts.font)
            return;
        int cursorX = pos.x;
        int cursorY = pos.y;
        for (char c : str) {
            drawChar(cursorX, cursorY, c, opts);
            cursorX += opts.font->getCharWidth(c) * opts.scale + opts.spacing;
            if (opts.maxWidth && cursorX - pos.x >= opts.maxWidth)
                break;
        }
    }

    void textCentered(const Point& pos, const std::string& str, const DrawTextOptions& opts)
    {
        if (!opts.font)
            return;
        int width = getTextWidth(str, opts);
        Point startPos = { pos.x - width / 2, pos.y };
        text(startPos, str, opts);
    }

    void textRight(const Point& pos, const std::string& str, const DrawTextOptions& opts)
    {
        if (!opts.font)
            return;
        int width = getTextWidth(str, opts);
        Point startPos = { pos.x - width, pos.y };
        text(startPos, str, opts);
    }

    void drawChar(int x, int y, char c, const DrawTextOptions& opts)
    {
        const Font* f = opts.font;
        int w = f->getCharWidth(c);
        int h = f->getHeight();
        const uint8_t* bitmap = f->getCharBitmap(c);

        for (int row = 0; row < h; ++row) {
            for (int col = 0; col < w; ++col) {
                bool pixelOn = bitmap[row * w + col] > opts.threshold;
                if (pixelOn) {
                    for (int sx = 0; sx < opts.scale; ++sx)
                        for (int sy = 0; sy < opts.scale; ++sy)
                            setPixel(x + col * opts.scale + sx, y + row * opts.scale + sy, opts.color);
                }
            }
        }
    }

    int getTextWidth(const std::string& str, const DrawTextOptions& opts)
    {
        if (!opts.font)
            return 0;
        int width = 0;
        for (char c : str) {
            width += opts.font->getCharWidth(c) * opts.scale + opts.spacing;
        }
        return width;
    }
};
