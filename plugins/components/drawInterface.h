#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "libs/nlohmann/json.hpp"

#include "plugins/components/EventInterface.h"

#include "baseInterface.h"

struct Styles {
    Size screen;
    const int margin;
    struct Font {
        const char* regular;
        const char* bold;
        const char* italic;
    } font;
    struct Colors {
        Color background;
        Color white;
        Color text;
        Color primary;
        Color secondary;
        Color tertiary;
        Color quaternary;
    } colors;
};

struct DrawOptions {
    Color color = { 255, 255, 255, 255 };
    int thickness = 1;
};

struct DrawTextOptions {
    Color color = { 255, 255, 255, 255 };
    void* font = NULL;
    int maxWidth = 0;
    int fontHeight = 0; // Compiled
    int fontSpacing = 1; // Ttf
};

class DrawInterface {
public:
    Styles& styles;
    DrawInterface(Styles& styles)
        : styles(styles)
    {
    }

    virtual void init() { }
    virtual void quit() { }
    virtual void render() = 0;
    virtual void renderNext() = 0;
    virtual void triggerRendering() = 0;

    virtual int textCentered(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) { return 0; }
    virtual int text(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) { return 0; }
    virtual int textRight(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) { return 0; }

    virtual void clear() { }
    virtual void clear(uint8_t page) { }
    virtual void line(Point start, Point end, DrawOptions options = {}) { }
    virtual void lines(std::vector<Point> points, DrawOptions options = {}) { }
    virtual void pixel(Point position, DrawOptions options = {}) { }
    virtual void filledRect(Point position, Size size, DrawOptions options = {}) { }
    virtual void rect(Point position, Size size, DrawOptions options = {}) { }

    // GFX
    virtual void filledRect(Point position, Size size, uint8_t radius, DrawOptions options = {}) { }
    virtual void rect(Point position, Size size, uint8_t radius, DrawOptions options = {}) { }
    virtual void filledPie(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {}) { }
    virtual void filledPolygon(std::vector<Point> points, DrawOptions options = {}) { }
    virtual void arc(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {}) { }
    virtual void circle(Point position, int radius, DrawOptions options = {}) { }
    virtual void filledCircle(Point position, int radius, DrawOptions options = {}) { }

    virtual void* setTextureRenderer(Size size) { return NULL; }
    virtual void setMainRenderer() { }
    virtual void destroyTexture(void* texture) { }
    virtual void applyTexture(void* texture, Rect dest) { }

    virtual void* getFont(std::string name = NULL, int size = -1) { return NULL; }
    virtual uint8_t getDefaultFontSize(void* font) { return 0; }
    virtual Color getColor(std::string color, Color defaultColor = { 0xFF, 0xFF, 0xFF }) { return defaultColor; }
    virtual Color getColor(const nlohmann::json& node, Color defaultColor) { return defaultColor; }
    virtual void config(nlohmann::json& config) { }

    virtual bool handleEvent(EventInterface* view) { return true; }

    virtual float getxFactor() = 0;
    virtual float getyFactor() = 0;

    virtual Point getWindowPosition() { return { -1, -1 }; }
    virtual Size getWindowSize() { return { -1, -1 }; }
};
