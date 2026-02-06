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
        , DrawPrimitives(DEFAULT_FONT, styles.screen.w)
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

    int text(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) override
    {
        return DrawPrimitives::text2(position, text, size, screenSize.w, options);
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
        const uint8_t** fontData = DrawPrimitives::getFont({ .font = font });
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
