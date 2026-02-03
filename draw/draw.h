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

#include "draw/drawInterface.h"
#include "draw/drawPrimitives.h"
#include "fonts/fonts.h"
#include "helpers/clamp.h"
#include "log.h"
#include "plugins/components/utils/color.h"

#include <cmath>
#include <stdexcept>
#include <string.h>
#include <string>

class Draw : public DrawInterface, public DrawPrimitives {
public:
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
                    // pixel({ (int)(pos.x + col * scale), (int)(pos.y + (row + marginTop) * scale) }, { color });
                }
            }
        }
        return width * scale;
    }

    int getTextWidth(std::string text, const uint8_t** font, int spacing)
    {
        int width = 0;
        for (char c : text) {
            const uint8_t* charPtr = font[1 + (c - ' ')];
            width += charPtr[0] + spacing;
        }
        return width;
    }

    int getLineHeight(uint32_t size, const uint8_t** font)
    {
        uint8_t fontHeight = *font[0];
        float scale = size / (float)fontHeight;
        if (scale == 0) scale = 1;
        return fontHeight * scale;
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
        // , DrawPrimitives(screenSize)
        , DrawPrimitives(DEFAULT_FONT)
        , screenSizeOrginal(styles.screen)
        , screenSize(styles.screen)
    {
    }

    ~Draw()
    {
    }

    bool needRenderingState = false;
    void renderNext() override
    {
        needRenderingState = true;
    }

    bool needRendering() override
    {
        bool value = needRenderingState;
        needRenderingState = false;
        return value;
    }

    void clear() override
    {
        // logDebug("clearing framebuffer yo %dx%d", screenSize.w, screenSize.h);
        // Init buffer with background color
        for (int y = 0; y < screenSize.h; y++) {
            for (int x = 0; x < screenSize.w; x++) {
                screenBuffer[y][x] = styles.colors.background;
            }
        }
    }

    void fullClear()
    {
        // Init buffer with background color
        for (int i = 0; i < SCREEN_BUFFER_ROWS; i++) { // here we can do the whole buffer even if it is out of bound
            for (int j = 0; j < SCREEN_BUFFER_COLS; j++) {
                screenBuffer[j][i] = styles.colors.background;
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
        return DrawPrimitives::text(position, text, size, options);
    }

    int textCentered(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) override
    {
        return DrawPrimitives::textCentered(position, text, size, options);
    }

    int textRight(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) override
    {
        return DrawPrimitives::textRight(position, text, size, options);
    }

    uint8_t getDefaultFontSize(void* font) override
    {
        const uint8_t** fontData = getFont({ .font = font });
        uint8_t height = *fontData[0];
        return height;
    }

    int textBox(Point position, Size size, std::string text, uint32_t fontSize, DrawTextOptions options = {}) override
    {
        return DrawPrimitives::textBox(position, size, text, fontSize, options);
    }

    void filledRect(Point position, Size size, DrawOptions options = {}) override
    {
        return DrawPrimitives::filledRect(position, size, options);
    }

    void rect(Point position, Size size, DrawOptions options = {}) override
    {
        return DrawPrimitives::rect(position, size, options);
    }

    void filledRect(Point position, Size size, uint8_t radius, DrawOptions options = {}) override
    {
        return DrawPrimitives::filledRect(position, size, radius, options);
    }

    void rect(Point position, Size size, uint8_t radius, DrawOptions options = {}) override
    {
        return DrawPrimitives::rect(position, size, radius, options);
    }

    void filledPie(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {}) override
    {
        return DrawPrimitives::filledPie(position, radius, startAngle, endAngle, options);
    }

    void arc(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {}) override
    {
        return DrawPrimitives::arc(position, radius, startAngle, endAngle, options);
    }

    void circle(Point position, int radius, DrawOptions options = {}) override
    {
        return DrawPrimitives::circle(position, radius, options);
    }

    void filledCircle(Point position, int radius, DrawOptions options = {})
    {
        return DrawPrimitives::filledCircle(position, radius, options);
    }

    void line(Point start, Point end, DrawOptions options = {}) override
    {
        return DrawPrimitives::line(start, end, options);
    }

    void lines(std::vector<Point> points, DrawOptions options = {}) override
    {
        return DrawPrimitives::lines(points, options);
    }

    void filledPolygon(std::vector<Point> points, DrawOptions options = {}) override
    {
        return DrawPrimitives::filledPolygon(points, options);
    }

    void pixel(Point position, DrawOptions options = {}) override
    {
        pixel(position, options.color);
    }

    void pixel(Point position, Color color) override
    {
        if (position.x < 0 || position.x >= screenSize.w || position.y < 0 || position.y >= screenSize.h) {
            return;
        }
        if (color.a != 255) {
            screenBuffer[position.y][position.x] = applyAlphaColor(screenBuffer[position.y][position.x], color);
            screenBuffer[position.y][position.x].a = 255;
        } else {
            screenBuffer[position.y][position.x] = color;
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
                logDebug("----------> set screenSize: %dx%d", screenSize.w, screenSize.h);
            }
        } catch (const std::exception& e) {
            logError("screen config: %s", e.what());
        }
    }
};
