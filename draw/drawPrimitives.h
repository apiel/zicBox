/**
 * drawPrimitives.h
 *
 * Platform-agnostic drawing primitives interface.
 * Works with both std::string/vector environments (Linux/RPi) and bare-metal (STM32H7).
 *
 * All functions accept raw pointers and const char* instead of std::string/std::vector
 * to ensure compatibility across platforms.
 */
#pragma once

#include "baseInterface.h"
#include "draw/drawOptions.h"
#include "plugins/components/utils/color.h"
#include "draw/fonts/Font.h"
#include "helpers/clamp.h"

#include <cstdint>
#include <string_view>

class DrawPrimitives {
public:
    virtual void pixel(Point position, Color color) = 0;

protected:
    void pixelWeight(Point position, DrawOptions options, int weight)
    {
        int ax = options.color.a;
        ax = ((ax * weight) >> 8);
        if (ax > 255) {
            options.color.a = 255;
        } else {
            options.color.a = (uint8_t)(ax & 0x000000ff);
        }
        // options.color.a = weight;
        pixel(position, options.color);
    }

    void lineVertical1px(Point start, Point end, DrawOptions options = {})
    {
        int y = start.y;
        int len = end.y;
        if (start.y > end.y) {
            y = end.y;
            len = start.y;
        }
        for (; y <= len; y++) {
            pixel({ start.x, y }, options.color);
        }
    }

    void lineHorizontal1px(Point start, Point end, DrawOptions options = {})
    {
        int x = start.x;
        int len = end.x;
        if (start.x > end.x) {
            x = end.x;
            len = start.x;
        }
        for (; x <= len; x++) {
            pixel({ x, start.y }, options.color);
        }
    }

    void lineDiagonal1px(Point start, Point end, DrawOptions options = {})
    {
        int32_t xx0 = start.x;
        int32_t yy0 = start.y;
        int32_t xx1 = end.x;
        int32_t yy1 = end.y;

        int dx, dy, tmp, xdir, y0p1, x0pxdir;
        if (yy0 > yy1) {
            tmp = yy0;
            yy0 = yy1;
            yy1 = tmp;
            tmp = xx0;
            xx0 = xx1;
            xx1 = tmp;
        }

        dx = xx1 - xx0;
        dy = yy1 - yy0;
        if (dx >= 0) {
            xdir = 1;
        } else {
            xdir = -1;
            dx = (-dx);
        }

#define AAbits 8

        uint32_t erracc = 0;
        uint32_t intshift = 32 - AAbits;

        pixel({ start.x, start.y }, options.color);

        if (dy > dx) {
            uint32_t erradj = ((dx << 16) / dy) << 16;
            x0pxdir = xx0 + xdir;
            while (--dy) {
                uint32_t erracctmp = erracc;
                erracc += erradj;
                if (erracc <= erracctmp) {
                    xx0 = x0pxdir;
                    x0pxdir += xdir;
                }
                yy0++;
                uint32_t wgt = (erracc >> intshift) & 255;
                pixelWeight({ xx0, yy0 }, options, 255 - wgt);
                pixelWeight({ x0pxdir, yy0 }, options, wgt);
            }

        } else {
            uint32_t erradj = ((dy << 16) / dx) << 16;
            y0p1 = yy0 + 1;
            while (--dx) {

                uint32_t erracctmp = erracc;
                erracc += erradj;
                if (erracc <= erracctmp) {
                    yy0 = y0p1;
                    y0p1++;
                }
                xx0 += xdir;
                uint32_t wgt = (erracc >> intshift) & 255;
                pixelWeight({ xx0, yy0 }, options, 255 - wgt);
                pixelWeight({ xx0, y0p1 }, options, wgt);
            }
        }
    }

    void lineVertical(Point start, Point end, DrawOptions options = {})
    {

        int startx = start.x - options.thickness * 0.5;
        int endx = end.x - options.thickness * 0.5;
        for (int i = 0; i < options.thickness; i++) {
            lineVertical1px({ startx + i, start.y }, { endx + i, end.y }, options);
        }
    }

    void lineHorizontal(Point start, Point end, DrawOptions options = {})
    {
        int starty = start.y - options.thickness * 0.5;
        int endy = end.y - options.thickness * 0.5;
        for (int i = 0; i < options.thickness; i++) {
            lineHorizontal1px({ start.x, starty + i }, { end.x, endy + i }, options);
        }
    }

    void lineDiagonal(Point start, Point end, DrawOptions options = {})
    {
        int startx = start.x - options.thickness * 0.5;
        int endx = end.x - options.thickness * 0.5;
        for (int i = 0; i < options.thickness; i++) {
            lineDiagonal1px({ startx + i, start.y }, { endx + i, end.y }, options);
        }
    }

    void line1px(Point start, Point end, DrawOptions options = {})
    {
        if (start.x == end.x) {
            lineVertical1px(start, end, options);
        } else if (start.y == end.y) {
            lineHorizontal1px(start, end, options);
        } else {
            lineDiagonal1px(start, end, options);
        }
    }

    void lineWithThickness(Point start, Point end, DrawOptions options = {})
    {
        if (start.x == end.x) {
            lineVertical(start, end, options);
        } else if (start.y == end.y) {
            lineHorizontal(start, end, options);
        } else {
            lineDiagonal(start, end, options);
        }
    }

    // void lines1px(std::span<const Point> points, DrawOptions options = {})
    // {
    //     for (int i = 0; i < points.size() - 1; i++) {
    //         line1px(points[i], points[i + 1], options);
    //     }
    // }

    // void linesWithThickness(std::span<const Point> points, DrawOptions options = {})
    // {
    //     for (int i = 0; i < points.size() - 1; i++) {
    //         lineWithThickness(points[i], points[i + 1], options);
    //     }
    // }

    template <typename PointCollection>
    void lines1px(const PointCollection& points, DrawOptions options = {})
    {
        if (points.size() < 2) return;
        for (size_t i = 0; i < points.size() - 1; i++) {
            line1px(points[i], points[i + 1], options);
        }
    }

    // 2. Thick Lines
    template <typename PointCollection>
    void linesWithThickness(const PointCollection& points, DrawOptions options = {})
    {
        if (points.size() < 2) return;
        for (size_t i = 0; i < points.size() - 1; i++) {
            lineWithThickness(points[i], points[i + 1], options);
        }
    }

    void fastLineDiagonal(Point start, Point end, DrawOptions options = {})
    {
        unsigned int dx = (end.x > start.x ? end.x - start.x : start.x - end.x);
        short xstep = end.x > start.x ? 1 : -1;
        unsigned int dy = (end.y > start.y ? end.y - start.y : start.y - end.y);
        short ystep = end.y > start.y ? 1 : -1;
        int col = start.x, row = start.y;

        if (dx < dy) {
            int t = -(dy >> 1);
            while (true) {
                pixel({ col, row }, options.color);
                if (row == end.y)
                    return;
                row += ystep;
                t += dx;
                if (t >= 0) {
                    col += xstep;
                    t -= dy;
                }
            }
        } else {
            int t = -(dx >> 1);
            while (true) {
                pixel({ col, row }, options.color);
                if (col == end.x)
                    return;
                col += xstep;
                t += dy;
                if (t >= 0) {
                    row += ystep;
                    t -= dx;
                }
            }
        }
    }

    int drawChar(Point pos, uint8_t* charPtr, int width, int marginTop, int rows, Color color, float scale = 1.00f)
    {
        float alpha = color.a / 255.0;
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < width; col++) {
                uint8_t a = charPtr[col + row * width];
                if (a) { // Only draw non-zero pixels
                    color.a = (uint8_t)(CLAMP(a * 2 * alpha, 0, 255));
                    pixel({ (int)(pos.x + col * scale), (int)(pos.y + row * scale + marginTop) }, color);
                }
            }
        }
        return width * scale;
    }

    int getTextWidth(std::string_view text, const uint8_t** font, int spacing)
    {
        int width = 0;
        for (char c : text) {
            const uint8_t* charPtr = font[1 + (c - ' ')];
            width += charPtr[0] + spacing;
        }
        return width;
    }

public:
    Font& defaultFont;

    DrawPrimitives(Font& defaultFont)
    : defaultFont(defaultFont)
    {}

    const uint8_t** getFont(DrawTextOptions options)
    {
        if (options.font) {
            return (const uint8_t**)((Font*)options.font)->data;
        }
        return (const uint8_t**)defaultFont.data;
    }

    int text(Point position, std::string_view text, uint32_t size, int screenWidth, DrawTextOptions options = {})
    {
        float x = position.x;
        float maxX = x + (options.maxWidth ? options.maxWidth : (screenWidth - x));
        uint16_t len = text.length();

        const uint8_t** font = getFont(options);
        uint8_t height = *font[0];
        float scale = size / (float)height;
        scale = scale == 0 ? 1 : scale;
        int y = position.y;
        for (uint16_t i = 0; i < len && x < maxX; i++) {
            char c = text[i];
            const uint8_t* charPtr = font[1 + (c - ' ')]; // Get the glyph data for the character
            uint8_t width = charPtr[0];
            uint8_t marginTop = charPtr[1] * scale;
            uint8_t rows = charPtr[2];
            if (x + width * scale > maxX) {
                break;
            }
            x += drawChar({ (int)x, y }, (uint8_t*)charPtr + 3, width, marginTop, rows, options.color, scale) + options.fontSpacing;
        }
        return x;
    }

    int textCentered(Point position, std::string_view text, uint32_t size, DrawTextOptions options = {})
    {
        uint16_t len = text.length();

        const uint8_t** font = getFont(options);
        uint8_t height = *font[0];
        float scale = size / (float)height;
        scale = scale == 0 ? 1 : scale;
        int y = position.y;

        int textWidth = getTextWidth(text, font, options.fontSpacing) * scale;
        int x = position.x - textWidth / 2;

        // How to handle maxWidth?

        for (uint16_t i = 0; i < len; i++) {
            char c = text[i];
            const uint8_t* charPtr = font[1 + (c - ' ')]; // Get the glyph data for the character
            uint8_t width = charPtr[0];
            uint8_t marginTop = charPtr[1] * scale;
            uint8_t rows = charPtr[2];
            x += drawChar({ (int)x, y }, (uint8_t*)charPtr + 3, width, marginTop, rows, options.color, scale) + options.fontSpacing;
        }
        return x;
    }

    int textRight(Point position, std::string_view text, uint32_t size, DrawTextOptions options = {})
    {
        uint16_t len = text.length();
        float x = position.x;

        const uint8_t** font = getFont(options);
        uint8_t height = *font[0];
        float scale = size / (float)height;
        scale = scale == 0 ? 1 : scale;
        int y = position.y;

        for (uint16_t i = 0; i < len; i++) {
            char c = text[len - i - 1];
            const uint8_t* charPtr = font[1 + (c - ' ')]; // Get the glyph data for the character
            uint8_t width = charPtr[0];
            uint8_t marginTop = charPtr[1] * scale;
            uint8_t rows = charPtr[2];
            x -= width * scale;
            drawChar({ (int)x, y }, (uint8_t*)charPtr + 3, width, marginTop, rows, options.color, scale);
            x -= options.fontSpacing;
        }

        return x;
    }

    int textBox(Point position, Size size, std::string_view text, uint32_t fontSize, DrawTextOptions options = {})
    {
        const uint8_t** font = getFont(options);

        uint8_t fontHeight = *font[0];
        float scale = fontSize / (float)fontHeight;
        if (scale == 0) scale = 1;

        int lineHeight = fontHeight * scale;
        int y = position.y;
        int xStart = position.x;
        int maxX = position.x + size.w;
        int maxY = position.y + size.h;

        std::string line;
        std::string word;

        Color currentColor = options.color;
        bool useAltColor = false;

        auto lineWidth = [&](const std::string& s) {
            return getTextWidth(s, font, options.fontSpacing) * scale;
        };

        auto drawLine = [&]() {
            float x = xStart;

            for (char c : line) {
                if (c == '~') {
                    useAltColor = !useAltColor;
                    currentColor = useAltColor ? options.color2 : options.color;
                    continue; // safety (should never happen)
                }

                const uint8_t* charPtr = font[1 + (c - ' ')];
                uint8_t width = charPtr[0];
                uint8_t marginTop = charPtr[1];
                uint8_t rows = charPtr[2];

                if (x + width * scale > maxX)
                    break;

                x += drawChar(
                    { (int)x, y },
                    (uint8_t*)charPtr + 3,
                    width,
                    marginTop,
                    rows,
                    currentColor,
                    scale);

                x += options.fontSpacing;
            }

            line.clear();
            y += lineHeight;
        };

        for (size_t i = 0; i <= text.size(); i++) {
            char c = (i < text.size()) ? text[i] : '\n';

            if (c == ' ' || c == '\n') {
                if (!word.empty()) {
                    std::string testLine = line.empty() ? word : line + " " + word;

                    if (lineWidth(testLine) > size.w) {
                        drawLine();
                        if (y > maxY) return y;
                        line = word;
                    } else {
                        if (!line.empty()) line += " ";
                        line += word;
                    }
                    word.clear();
                }

                if (c == '\n') {
                    drawLine(); // blank lines supported
                    if (y > maxY) return y;
                }
            } else {
                word += c;
            }
        }

        return y;
    }

    void filledRect(Point position, Size size, DrawOptions options = {})
    {
        for (int y = position.y; y <= position.y + size.h; y++) {
            lineHorizontal1px({ position.x, y }, { position.x + size.w, y }, options);
        }
    }

    // FIXME thickness is not working properly
    void rect(Point position, Size size, DrawOptions options = {})
    {
        Point a = position;
        Point b = { position.x + size.w, position.y };
        Point c = { position.x + size.w, position.y + size.h };
        Point d = { position.x, position.y + size.h };
        line(a, b, options);
        line(b, c, options);
        line(c, d, options);
        line(d, a, options);
    }

    void filledRect(Point position, Size size, uint8_t radius, DrawOptions options = {})
    {
        filledRect({ position.x + radius, position.y }, { size.w - 2 * radius, size.h }, options);
        filledRect({ position.x, position.y + radius }, { size.w, size.h - 2 * radius }, options);
        filledPie({ position.x + radius, position.y + radius }, radius, 180, 270, options);
        filledPie({ position.x + size.w - radius + 1, position.y + radius }, radius, 270, 360, options);
        filledPie({ position.x + radius, position.y + size.h - radius }, radius, 90, 180, options);
        filledPie({ position.x + size.w - radius + 1, position.y + size.h - radius }, radius, 0, 90, options);
    }

    // FIXME thickness is not working properly
    void rect(Point position, Size size, uint8_t radius, DrawOptions options = {})
    {
        arc({ position.x + radius, position.y + radius }, radius, 180, 270, options);
        line({ position.x, position.y + radius }, { position.x, position.y + size.h - radius }, options);
        arc({ position.x + size.w - radius, position.y + radius }, radius, 270, 360, options);
        line({ position.x + radius, position.y }, { position.x + size.w - radius, position.y }, options);
        arc({ position.x + radius, position.y + size.h - radius }, radius, 90, 180, options);
        line({ position.x + radius, position.y + size.h }, { position.x + size.w - radius, position.y + size.h }, options);
        arc({ position.x + size.w - radius, position.y + size.h - radius }, radius, 0, 90, options);
        line({ position.x + size.w, position.y + size.h - radius }, { position.x + size.w, position.y + radius }, options);
    }

    void filledPie(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {})
    {
        int xi, yi;
        double s, v, x, y, dx, dy;

        // Normalize angles to range [0, 360)
        startAngle = (startAngle % 360 + 360) % 360;
        endAngle = (endAngle % 360 + 360) % 360;

        // Convert angles to radians for comparison
        double startRad = startAngle * M_PI / 180.0;
        double endRad = endAngle * M_PI / 180.0;

        // Ensure angles are in counterclockwise order
        bool wrapAround = (endRad < startRad);

        int n = radius + 1;
        for (yi = position.y - n - 1; yi <= position.y + n + 1; yi++) {
            if (yi < (position.y - 0.5))
                y = yi;
            else
                y = yi + 1;
            s = (y - position.y) / radius;
            s = s * s;
            x = 0.5;
            if (s < 1.0) {
                x = radius * sqrt(1.0 - s);
                if (x >= 0.5) {
                    // Draw only the visible part of the pie within the angle range
                    for (xi = (int)(position.x - x + 1); xi <= (int)(position.x + x - 1); xi++) {
                        // Convert (xi, yi) to polar coordinates (r, theta)
                        double dx = xi - position.x;
                        double dy = yi - position.y;
                        double angle = atan2(dy, dx);
                        if (angle < 0)
                            angle += 2 * M_PI; // Normalize to [0, 2π]

                        // Check if angle is within the start and end angles
                        if ((wrapAround && (angle >= startRad || angle <= endRad)) || (!wrapAround && angle >= startRad && angle <= endRad)) {
                            pixel({ xi, yi }, options.color);
                        }
                    }
                }
            }
            s = 8 * radius * radius;
            dy = fabs(y - position.y) - 1.0;
            xi = position.x - x; // left
            while (1) {
                dx = (position.x - xi - 1);
                v = s - 4 * (dx - dy) * (dx - dy);
                if (v < 0)
                    break;
                v = (sqrt(v) - 2 * (dx + dy)) / 4;
                if (v < 0)
                    break;
                if (v > 1.0)
                    v = 1.0;

                // Convert (xi, yi) to polar coordinates (r, theta)
                double dx = xi - position.x;
                double dy = yi - position.y;
                double angle = atan2(dy, dx);
                if (angle < 0)
                    angle += 2 * M_PI; // Normalize to [0, 2π]

                // Check if angle is within the start and end angles
                if ((wrapAround && (angle >= startRad || angle <= endRad)) || (!wrapAround && angle >= startRad && angle <= endRad)) {
                    pixel({ xi, yi }, { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v) });
                }

                xi -= 1;
            }
            xi = position.x + x; // right
            while (1) {
                dx = (xi - position.x);
                v = s - 4 * (dx - dy) * (dx - dy);
                if (v < 0)
                    break;
                v = (sqrt(v) - 2 * (dx + dy)) / 4;
                if (v < 0)
                    break;
                if (v > 1.0)
                    v = 1.0;

                // Convert (xi, yi) to polar coordinates (r, theta)
                double dx = xi - position.x;
                double dy = yi - position.y;
                double angle = atan2(dy, dx);
                if (angle < 0)
                    angle += 2 * M_PI; // Normalize to [0, 2π]

                // Check if angle is within the start and end angles
                if ((wrapAround && (angle >= startRad || angle <= endRad)) || (!wrapAround && angle >= startRad && angle <= endRad)) {
                    pixel({ xi, yi }, { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v) });
                }

                xi += 1;
            }
        }

        // Draw smooth radius lines for the start and end angles
        int r = radius - 1;
        int startX = position.x + (int)(r * cos(startRad));
        int startY = position.y + (int)(r * sin(startRad));
        int endX = position.x + (int)(r * cos(endRad));
        int endY = position.y + (int)(r * sin(endRad));
        line1px({ startX, startY }, position, { options.color });
        line1px({ endX, endY }, position, { options.color });
    }

    void arc(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {})
    {
        int xi, yi;
        double s, v, x, y, dx, dy;

        // Normalize angles to range [0, 360)
        startAngle = (startAngle % 360 + 360) % 360;
        endAngle = (endAngle % 360 + 360) % 360;

        // Convert angles to radians
        double startRad = startAngle * M_PI / 180.0;
        double endRad = endAngle * M_PI / 180.0;

        // Handle angle wrapping
        bool wrapAround = (endRad < startRad);

        // Calculate inner and outer radii
        double innerRadius = radius - options.thickness / 2.0;
        double outerRadius = radius + options.thickness / 2.0;

        int n = outerRadius + 1;
        for (yi = position.y - n - 1; yi <= position.y + n + 1; yi++) {
            if (yi < (position.y - 0.5))
                y = yi;
            else
                y = yi + 1;
            s = (y - position.y) / outerRadius;
            s = s * s;
            x = 0.5;
            if (s < 1.0) {
                x = outerRadius * sqrt(1.0 - s);
                if (x >= 0.5) {
                    for (xi = (int)(position.x - x + 1); xi <= (int)(position.x + x - 1); xi++) {
                        // Calculate distance to the center
                        double dx = xi - position.x;
                        double dy = yi - position.y;
                        double distance = sqrt(dx * dx + dy * dy);

                        // Determine if within the angle and thickness range
                        if (distance >= innerRadius && distance <= outerRadius) {
                            // Convert (xi, yi) to polar coordinates
                            double angle = atan2(dy, dx);
                            if (angle < 0)
                                angle += 2 * M_PI; // Normalize to [0, 2π]

                            // Check angle constraints
                            if ((wrapAround && (angle >= startRad || angle <= endRad)) || (!wrapAround && angle >= startRad && angle <= endRad)) {
                                // Anti-aliasing based on distance to inner and outer radii
                                double alpha = 1.0;
                                if (distance < innerRadius + 1.0) {
                                    alpha = distance - innerRadius;
                                } else if (distance > outerRadius - 1.0) {
                                    alpha = outerRadius - distance;
                                }
                                if (alpha > 0) {
                                    pixel({ xi, yi }, { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * alpha) });
                                }
                            }
                        }
                    }
                }
            }
            s = 8 * outerRadius * outerRadius;
            dy = fabs(y - position.y) - 1.0;
            xi = position.x - x; // left
            while (1) {
                dx = (position.x - xi - 1);
                v = s - 4 * (dx - dy) * (dx - dy);
                if (v < 0)
                    break;
                v = (sqrt(v) - 2 * (dx + dy)) / 4;
                if (v < 0)
                    break;
                if (v > 1.0)
                    v = 1.0;

                // Calculate distance to the center
                double dx = xi - position.x;
                double dy = yi - position.y;
                double distance = sqrt(dx * dx + dy * dy);

                // Determine if within the angle and thickness range
                if (distance >= innerRadius && distance <= outerRadius) {
                    double angle = atan2(dy, dx);
                    if (angle < 0)
                        angle += 2 * M_PI; // Normalize to [0, 2π]

                    // Check angle constraints
                    if ((wrapAround && (angle >= startRad || angle <= endRad)) || (!wrapAround && angle >= startRad && angle <= endRad)) {
                        double alpha = 1.0;
                        if (distance < innerRadius + 1.0) {
                            alpha = distance - innerRadius;
                        } else if (distance > outerRadius - 1.0) {
                            alpha = outerRadius - distance;
                        }
                        if (alpha > 0) {
                            pixel({ xi, yi }, { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v * alpha) });
                        }
                    }
                }

                xi -= 1;
            }
            xi = position.x + x; // right
            while (1) {
                dx = (xi - position.x);
                v = s - 4 * (dx - dy) * (dx - dy);
                if (v < 0)
                    break;
                v = (sqrt(v) - 2 * (dx + dy)) / 4;
                if (v < 0)
                    break;
                if (v > 1.0)
                    v = 1.0;

                // Calculate distance to the center
                double dx = xi - position.x;
                double dy = yi - position.y;
                double distance = sqrt(dx * dx + dy * dy);

                // Determine if within the angle and thickness range
                if (distance >= innerRadius && distance <= outerRadius) {
                    double angle = atan2(dy, dx);
                    if (angle < 0)
                        angle += 2 * M_PI; // Normalize to [0, 2π]

                    // Check angle constraints
                    if ((wrapAround && (angle >= startRad || angle <= endRad)) || (!wrapAround && angle >= startRad && angle <= endRad)) {
                        double alpha = 1.0;
                        if (distance < innerRadius + 1.0) {
                            alpha = distance - innerRadius;
                        } else if (distance > outerRadius - 1.0) {
                            alpha = outerRadius - distance;
                        }
                        if (alpha > 0) {
                            pixel({ xi, yi }, { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v * alpha) });
                        }
                    }
                }

                xi += 1;
            }
        }
    }

    void circle(Point position, int radius, DrawOptions options = {})
    {
        arc(position, radius, 0, 359, options);
    }

    void filledCircle(Point position, int radius, DrawOptions options = {})
    {
        int xi, yi;
        double s, v, x, y, dx, dy;

        int n = radius + 1;
        for (yi = position.y - n - 1; yi <= position.y + n + 1; yi++) {
            if (yi < (position.y - 0.5))
                y = yi;
            else
                y = yi + 1;
            s = (y - position.y) / radius;
            s = s * s;
            x = 0.5;
            if (s < 1.0) {
                x = radius * sqrt(1.0 - s);
                if (x >= 0.5) {
                    line1px({ (int)(position.x - x + 1), yi }, { (int)(position.x + x - 1), yi }, options);
                }
            }
            s = 8 * radius * radius;
            dy = fabs(y - position.y) - 1.0;
            xi = position.x - x; // left
            while (1) {
                dx = (position.x - xi - 1);
                v = s - 4 * (dx - dy) * (dx - dy);
                if (v < 0)
                    break;
                v = (sqrt(v) - 2 * (dx + dy)) / 4;
                if (v < 0)
                    break;
                if (v > 1.0)
                    v = 1.0;
                pixel({ xi, yi }, { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v) });
                xi -= 1;
            }
            xi = position.x + x; // right
            while (1) {
                dx = (xi - position.x);
                v = s - 4 * (dx - dy) * (dx - dy);
                if (v < 0)
                    break;
                v = (sqrt(v) - 2 * (dx + dy)) / 4;
                if (v < 0)
                    break;
                if (v > 1.0)
                    v = 1.0;
                pixel({ xi, yi }, { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v) });
                xi += 1;
            }
        }
    }

    void line(Point start, Point end, DrawOptions options = {})
    {
        if (options.thickness == 1) {
            line1px(start, end, options);
        } else {
            lineWithThickness(start, end, options);
        }
    }

    template <typename PointCollection>
    void lines(const PointCollection& points, DrawOptions options = {})
    {
        if (options.thickness <= 1) {
            lines1px(points, options);
        } else {
            linesWithThickness(points, options);
        }
    }

    template <typename PointCollection>
    void filledPolygon(const PointCollection& points, DrawOptions options = {})
    {
        const size_t numPoints = points.size();
        if (numPoints < 3) return;

        // Draw the closed outline
        lines1px(points, options);
        line1px(points[0], points[numPoints - 1], options);

        // Compute bounding box
        int minY = points[0].y;
        int maxY = points[0].y;
        for (size_t i = 1; i < numPoints; ++i) {
            if (points[i].y < minY) minY = points[i].y;
            if (points[i].y > maxY) maxY = points[i].y;
        }

        // Fixed-size stack array for intersections (Max 32 intersections per scanline)
        // This replaces std::vector to avoid MCU heap allocation
        int intersections[32];

        for (int y = minY; y <= maxY; ++y) {
            size_t intersectCount = 0;

            for (size_t i = 0; i < numPoints; ++i) {
                Point p1 = points[i];
                Point p2 = points[(i + 1) % numPoints];

                // Scan-line intersection logic
                if ((p1.y <= y && p2.y > y) || (p2.y <= y && p1.y > y)) {
                    if (intersectCount < 32) {
                        float t = (float)(y - p1.y) / (float)(p2.y - p1.y);
                        intersections[intersectCount++] = p1.x + (int)(t * (p2.x - p1.x));
                    }
                }
            }

            // Sort intersections on the stack
            std::sort(intersections, intersections + intersectCount);

            for (size_t i = 0; i < intersectCount; i += 2) {
                if (i + 1 < intersectCount) {
                    int xStart = intersections[i] + 1;
                    int xEnd = intersections[i + 1];

                    // Using a horizontal line helper is much faster than individual pixel calls
                    for (int x = xStart; x <= xEnd; ++x) {
                        pixel({ x, y }, options.color);
                    }
                }
            }
        }
    }
};
