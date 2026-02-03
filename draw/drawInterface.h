/** Description:
This header file establishes a complete blueprint, or contract, for a generic graphics rendering system. Its purpose is to define a standard set of drawing commands, effectively separating the application's visual layout logic from the specific technology used to draw it (like a graphics card library).

The file is organized around style configuration and the drawing interface itself.

**Style and Configuration:**
The `Styles` structure acts as a central theme guide. It stores all fundamental visual settings: screen dimensions, margins, specific font file names (regular, bold, italic), and a comprehensive palette of standardized colors (such as background, primary accent, and text color).

Supporting structures, like `DrawOptions` and `DrawTextOptions`, hold specific instructions for individual actions, detailing temporary settings such as the exact color, line thickness for a shape, or font size for a piece of text.

**The Drawing Blueprint (`DrawInterface`):**
This class serves as the core contract, outlining every function a rendering engine must provide. It manages the global `Styles` and includes essential methods for the entire drawing process: starting the system, shutting it down, and executing the actual display updates (`render`).

The interface guarantees the ability to draw many geometric shapes—lines, circles, rectangles (including rounded corners and filled variations), polygons, and arcs. It also ensures precise text rendering with options for left, right, or center alignment. Additionally, it handles advanced features like managing and applying textures (images) and retrieving crucial screen configuration details like size and scaling factors.

This architecture allows developers to swap the underlying graphics engine without needing to rewrite their core drawing code.

sha: 7435e292761230dbbb5f836a6303d8d26e9aaa22fa2f72b138beafe7fa2fd014
*/
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "libs/nlohmann/json.hpp"

#include "plugins/components/EventInterface.h"

#include "draw/baseInterface.h"
#include "draw/drawOptions.h"

// Let's make a buffer bigger than necessary so we are sure any screen size can fit
// #define SCREEN_BUFFER_ROWS 2048
// #define SCREEN_BUFFER_COLS 2048
#define SCREEN_BUFFER_ROWS 4096
#define SCREEN_BUFFER_COLS 4096
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

class DrawInterface {
public:
    Styles& styles;
    Color screenBuffer[SCREEN_BUFFER_ROWS][SCREEN_BUFFER_COLS];

    DrawInterface(Styles& styles)
        : styles(styles)
    {
    }

    virtual void renderNext() = 0;
    virtual bool needRendering() = 0;

    virtual int textCentered(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) { return 0; }
    virtual int text(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) { return 0; }
    virtual int textRight(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) { return 0; }
    virtual int textBox(Point position, Size size, std::string text, uint32_t fontSize, DrawTextOptions options = {}) { return 0; }

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

    virtual float getxFactor() = 0;
    virtual float getyFactor() = 0;
    virtual Size& getScreenSize() = 0;
};
