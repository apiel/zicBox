/** Description:
This code defines a specialized drawing engine, encapsulated in a class named `Draw`, designed to manage all graphical output for an application.

**Core Mechanism:**
Instead of drawing directly to a physical screen, this engine uses a large internal memory space, often called a "screen buffer," which holds the color data for every potential pixel (up to 4096x4096). All drawing commands modify this internal memory map. When the content needs to be displayed, the system simply copies this buffer to the actual display device, a process managed by the built-in rendering functions.

**Key Drawing Capabilities:**
The class provides a comprehensive set of functions to create shapes and text:
1.  **Lines and Shapes:** It can draw horizontal, vertical, and diagonal lines, handling various thicknesses. It supports complex shapes like filled and outlined rectangles (with or without rounded corners), full circles, and partial circles (arcs and pies).
2.  **Polygons:** It can draw and fill complex, multi-sided shapes, automatically calculating which internal pixels to color in.
3.  **Text Rendering:** It integrates font handling, allowing text to be drawn at specified sizes and positions, with alignment options (left, center, right). It also incorporates anti-aliasing techniques to ensure smooth edges on lines and text.

**Utility and Management:**
The engine manages the current screen dimensions and calculates scaling factors, ensuring content looks correct even if the display size changes. It handles color by allowing users to specify exact values or use predefined names ("primary," "background"). It also manages transparency, blending new colors with existing ones on the buffer. This structure allows the application to queue up many drawing commands efficiently before refreshing the screen in a single update cycle.

sha: a2f584bdca85e7c5714fba25d6052ae9ac6d7a102ce0e9731bc3b270de454d1a 
*/
#pragma once

#include "fonts/fonts.h"
#include "helpers/clamp.h"
#include "log.h"
#include "plugins/components/drawInterface.h"
#include "plugins/components/utils/color.h"

#include <cmath>
#include <stdexcept>
#include <string.h>
#include <string>

// Let's make a buffer bigger than necessary so we are sure any screen size can fit
// #define SCREEN_BUFFER_ROWS 2048
// #define SCREEN_BUFFER_COLS 2048
#define SCREEN_BUFFER_ROWS 4096
#define SCREEN_BUFFER_COLS 4096

class Draw : public DrawInterface {
public:
    Color screenBuffer[SCREEN_BUFFER_ROWS][SCREEN_BUFFER_COLS];

    Size screenSizeOrginal;
    Size screenSize;

    Size& getScreenSize() override { return screenSize; }
    float getxFactor() override { return screenSize.w / float(screenSizeOrginal.w); }
    float getyFactor() override { return screenSize.h / float(screenSizeOrginal.h); }

    void setScreenSize(Size newSize)
    {
        screenSize = newSize;
    }

protected:
    bool needRendering = false;

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

    void lines1px(std::vector<Point> points, DrawOptions options = {})
    {
        for (int i = 0; i < points.size() - 1; i++) {
            line1px(points[i], points[i + 1], options);
        }
    }

    void linesWithThickness(std::vector<Point> points, DrawOptions options = {})
    {
        for (int i = 0; i < points.size() - 1; i++) {
            lineWithThickness(points[i], points[i + 1], options);
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

    void lineVertical1px(Point start, Point end, DrawOptions options = {})
    {
        int y = start.y;
        int len = end.y;
        if (start.y > end.y) {
            y = end.y;
            len = start.y;
        }
        for (; y <= len; y++) {
            pixel({ start.x, y }, options);
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

    void lineHorizontal1px(Point start, Point end, DrawOptions options = {})
    {
        int x = start.x;
        int len = end.x;
        if (start.x > end.x) {
            x = end.x;
            len = start.x;
        }
        for (; x <= len; x++) {
            pixel({ x, start.y }, options);
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

        pixel({ start.x, start.y }, options);

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
                pixel({ col, row }, options);
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
                pixel({ col, row }, options);
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
        pixel(position, options);
    }

#ifdef FT_FREETYPE_H
    int drawChar(Point position, char character, FT_Face& face, int lineHeight, DrawOptions options = {}, int maxX = 0)
    {
        if (FT_Load_Char(face, character, FT_LOAD_RENDER)) {
            return 0;
        }

        FT_Bitmap* bitmap = &face->glyph->bitmap;
        if (maxX && (position.x + bitmap->width > maxX)) {
            return position.x + bitmap->width;
        }

        int marginTop = lineHeight - face->glyph->bitmap_top;
        return drawChar(position, bitmap->buffer, bitmap->width, marginTop, bitmap->rows, options.color);
    }
#endif

    int drawChar(Point pos, uint8_t* charPtr, int width, int marginTop, int rows, Color color, float scale = 1.00f)
    {
        float alpha = color.a / 255.0;
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < width; col++) {
                uint8_t a = charPtr[col + row * width];
                if (a) { // Only draw non-zero pixels
                    color.a = (uint8_t)(CLAMP(a * 2 * alpha, 0, 255));
                    pixel({ (int)(pos.x + col * scale), (int)(pos.y + row * scale + marginTop) }, { color });
                }
            }
        }
        return width * scale;
    }

    int getTextWidth(std::string text, const uint8_t** font, int spacing)
    {
        int width = 0;
        for (uint16_t i = 0; i < text.length(); i++) {
            char c = text[i];
            const uint8_t* charPtr = font[1 + (c - ' ')];
            width += charPtr[0] + spacing;
        }
        return width;
    }

    Color* getStyleColor(std::string& color)
    {
        if (color == "background") {
            return &styles.colors.background;
        }

        if (color == "white") {
            return &styles.colors.white;
        }

        if (color == "primary") {
            return &styles.colors.primary;
        }

        if (color == "secondary") {
            return &styles.colors.secondary;
        }

        if (color == "tertiary") {
            return &styles.colors.tertiary;
        }

        if (color == "quaternary") {
            return &styles.colors.quaternary;
        }

        if (color == "text") {
            return &styles.colors.text;
        }

        return NULL;
    }

public:
    Draw(Styles& styles)
        : DrawInterface(styles)
        , screenSizeOrginal(styles.screen)
        , screenSize(styles.screen)
    {
    }

    ~Draw()
    {
    }

    void init() override
    {
        logWarn("Initializing draw without Renderer");
    }

    void renderNext() override
    {
        needRendering = true;
    }

    void triggerRendering() override
    {
        if (needRendering) {
            render();
            needRendering = false;
        }
    }

    void render() override
    {
    }

    virtual void preRender(EventInterface* view, uint64_t now) { }

    void clear() override
    {
        // Init buffer with background color
        for (int i = 0; i < screenSize.w; i++) { // here we can do the whole buffer even if it is out of bound
            for (int j = 0; j < screenSize.h; j++) {
                screenBuffer[i][j] = styles.colors.background;
            }
        }
    }

    void fullClear()
    {
        // Init buffer with background color
        for (int i = 0; i < SCREEN_BUFFER_ROWS; i++) { // here we can do the whole buffer even if it is out of bound
            for (int j = 0; j < SCREEN_BUFFER_COLS; j++) {
                screenBuffer[i][j] = styles.colors.background;
            }
        }
    }

    void* getFont(std::string name = NULL, int size = -1) override
    {
        void* font = getFontPtr(name);
        if (!font) {
            throw std::runtime_error("Unknown font " + std::string(name));
        }
        return font;
    }

#ifdef FT_FREETYPE_H
    TtfFont* getTtfFont(DrawTextOptions options)
    {
        if (options.font && ((TtfFont*)options.font)->type == TTF_FONT) {
            return (TtfFont*)options.font;
        }
        return nullptr;
    }
#endif

    const uint8_t** getFont(DrawTextOptions options)
    {
        if (options.font) {
            return (const uint8_t**)((Font*)options.font)->data;
        }
        return (const uint8_t**)DEFAULT_FONT.data;
    }

    int text(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) override
    {
        float x = position.x;
        float maxX = x + (options.maxWidth ? options.maxWidth : (screenSize.w - x));
        uint16_t len = text.length();

#ifdef FT_FREETYPE_H
        TtfFont* ttfFont = getTtfFont(options);
        if (ttfFont) {
            FT_Set_Pixel_Sizes(ttfFont->face, 0, size);
            for (uint16_t i = 0; i < len && x < maxX; i++) {
                x += drawChar({ (int)x, position.y }, text[i], ttfFont->face, size, { .color = { options.color } }, maxX);
                x += options.fontSpacing;
            }
            return x;
        }
#endif

        const uint8_t** font = getFont(options);
        uint8_t height = *font[0];
        float scale = size / (float)height;
        scale = scale == 0 ? 1 : scale;
        int heightRatio = options.fontHeight == 0 ? 1 : (options.fontHeight / height);
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

    int textCentered(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) override
    {
        uint16_t len = text.length();

#ifdef FT_FREETYPE_H
        TtfFont* ttfFont = getTtfFont(options);
        if (ttfFont) {
            FT_Set_Pixel_Sizes(ttfFont->face, 0, size);

            int w = ttfFont->getWidth(text, options.fontSpacing);
            if (options.maxWidth && w > options.maxWidth) {
                w = options.maxWidth;
            }
            float x = position.x - w / 2;
            for (uint16_t i = 0; i < len && x < size.w; i++) {
                if (x > 0) {
                    x += drawChar({ (int)x, position.y }, text[i], ttfFont->face, size, { .color = { options.color } });
                }
                x += options.fontSpacing;
            }
            return x;
        }
#endif

        const uint8_t** font = getFont(options);
        uint8_t height = *font[0];
        float scale = size / (float)height;
        scale = scale == 0 ? 1 : scale;
        int heightRatio = options.fontHeight == 0 ? 1 : (options.fontHeight / height);
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

    int textRight(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) override
    {
        uint16_t len = text.length();
        float x = position.x;

#ifdef FT_FREETYPE_H
        TtfFont* ttfFont = getTtfFont(options);
        if (ttfFont) {
            FT_Set_Pixel_Sizes(ttfFont->face, 0, size);
            for (uint16_t i = 0; i < len && x > 0; i++) {
                char c = text[len - i - 1];
                x -= ttfFont->getWidth(c);
                if (x < 0) {
                    break;
                }
                drawChar({ (int)x, position.y }, c, ttfFont->face, size, { .color = { options.color } });
                x -= options.fontSpacing;
            }
            return x;
        }
#endif

        const uint8_t** font = getFont(options);
        uint8_t height = *font[0];
        float scale = size / (float)height;
        scale = scale == 0 ? 1 : scale;
        int heightRatio = options.fontHeight == 0 ? 1 : (options.fontHeight / height);
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

    uint8_t getDefaultFontSize(void* font) override
    {
#ifdef FT_FREETYPE_H
        TtfFont* ttfFont = getTtfFont(options);
        if (ttfFont) {
            return 0;
        }
#endif

        const uint8_t** fontData = getFont({ .font = font });
        uint8_t height = *fontData[0];
        return height;
    }

    void filledRect(Point position, Size size, DrawOptions options = {}) override
    {
        for (int y = position.y; y < position.y + size.h; y++) {
            lineHorizontal1px({ position.x, y }, { position.x + size.w, y }, options);
        }
    }

    // FIXME thickness is not working properly
    void rect(Point position, Size size, DrawOptions options = {}) override
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

    void filledRect(Point position, Size size, uint8_t radius, DrawOptions options = {}) override
    {
        filledRect({ position.x + radius, position.y }, { size.w - 2 * radius, size.h }, options);
        filledRect({ position.x, position.y + radius }, { size.w, size.h - 2 * radius }, options);
        filledPie({ position.x + radius, position.y + radius }, radius, 180, 270, options);
        filledPie({ position.x + size.w - radius + 1, position.y + radius }, radius, 270, 360, options);
        filledPie({ position.x + radius, position.y + size.h - radius }, radius, 90, 180, options);
        filledPie({ position.x + size.w - radius + 1, position.y + size.h - radius }, radius, 0, 90, options);
    }

    // FIXME thickness is not working properly
    void rect(Point position, Size size, uint8_t radius, DrawOptions options = {}) override
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

    void filledPie(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {}) override
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
                            pixel({ xi, yi }, { options.color });
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
                    pixel({ xi, yi }, { { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v) } });
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
                    pixel({ xi, yi }, { { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v) } });
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

    void arc(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {}) override
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
                                    pixel({ xi, yi }, { { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * alpha) } });
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
                            pixel({ xi, yi }, { { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v * alpha) } });
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
                            pixel({ xi, yi }, { { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v * alpha) } });
                        }
                    }
                }

                xi += 1;
            }
        }
    }

    void circle(Point position, int radius, DrawOptions options = {}) override
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
                pixel({ xi, yi }, { { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v) } });
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
                pixel({ xi, yi }, { { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v) } });
                xi += 1;
            }
        }
    }

    void line(Point start, Point end, DrawOptions options = {}) override
    {
        if (options.thickness == 1) {
            line1px(start, end, options);
        } else {
            lineWithThickness(start, end, options);
        }
    }

    void lines(std::vector<Point> points, DrawOptions options = {}) override
    {
        if (options.thickness == 1) {
            lines1px(points, options);
        } else {
            linesWithThickness(points, options);
        }
    }

    void filledPolygon(std::vector<Point> points, DrawOptions options = {}) override
    {
        if (points.size() < 3)
            return; // A polygon must have at least 3 points

        lines1px(points, options);
        line1px(points[0], points[points.size() - 1], options);

        // Compute the bounding box of the polygon
        int minY = points[0].y, maxY = points[0].y;
        for (const auto& p : points) {
            minY = std::min(minY, p.y);
            maxY = std::max(maxY, p.y);
        }

        // Use a scan-line approach to fill the polygon
        for (int y = minY; y <= maxY; ++y) {
            std::vector<int> intersections;

            // Find intersections of the polygon with the current scan line
            for (size_t i = 0; i < points.size(); ++i) {
                Point p1 = points[i];
                Point p2 = points[(i + 1) % points.size()]; // Wrap around to the first point

                // Check if the scan line crosses the edge
                if ((p1.y <= y && p2.y > y) || (p2.y <= y && p1.y > y)) {
                    // Compute the x-coordinate of the intersection
                    float t = (y - p1.y) / float(p2.y - p1.y);
                    int x = p1.x + t * (p2.x - p1.x);
                    intersections.push_back(x);
                }
            }

            // Sort the intersections to form spans
            std::sort(intersections.begin(), intersections.end());

            // Fill the spans between pairs of intersections
            for (size_t i = 0; i < intersections.size(); i += 2) {
                if (i + 1 < intersections.size()) {
                    int xStart = intersections[i] + 1;
                    int xEnd = intersections[i + 1];

                    for (int x = xStart; x <= xEnd; ++x) {
                        pixel({ x, y }, options);
                    }
                }
            }
        }
    }

    void pixel(Point position, DrawOptions options = {}) override
    {
        if (position.x < 0 || position.x >= screenSize.w || position.y < 0 || position.y >= screenSize.h) {
            return;
        }
        if (options.color.a != 255) {
            screenBuffer[position.y][position.x] = applyAlphaColor(screenBuffer[position.y][position.x], options.color);
            screenBuffer[position.y][position.x].a = 255;
        } else {
            screenBuffer[position.y][position.x] = options.color;
        }
    }

    Color getColor(std::string color, Color defaultColor = { 0xFF, 0xFF, 0xFF }) override
    {
        // if first char is # then call hex2rgb
        if (color[0] == '#') {
            return hex2rgb(color);
        }

        Color* styleColor = getStyleColor(color);
        if (styleColor != NULL) {
            return *styleColor;
        }

        return defaultColor;
    }

    Color getColor(const nlohmann::json& color, Color defaultColor) override
    {
        if (color.is_null()) {
            return defaultColor;
        }

        return getColor(color.get<std::string>(), defaultColor);
    }

    void config(nlohmann::json& config) override
    {
        try {
            if (config.contains("colors") && config["colors"].is_array()) {
                for (auto& color : config["colors"]) {
                    std::string name = color["name"].get<std::string>();
                    Color* styleColor = getStyleColor(name);
                    if (styleColor != NULL) {
                        Color newColor = getColor(color["color"]);
                        styleColor->r = newColor.r;
                        styleColor->g = newColor.g;
                        styleColor->b = newColor.b;
                    }
                }
            }
            if (config.contains("screenSize")) {
                styles.screen.w = config["screenSize"]["width"].get<int>();
                styles.screen.h = config["screenSize"]["height"].get<int>();
                screenSizeOrginal = styles.screen;
                screenSize = styles.screen;
            }
        } catch (const std::exception& e) {
            logError("screen config: %s", e.what());
        }
    }
};
