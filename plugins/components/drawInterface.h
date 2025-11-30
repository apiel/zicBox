/** Description:
This C++ header file establishes the core system for all graphical rendering and styling within a software application. It acts as a universal blueprint, ensuring that different parts of the program can draw elements consistently, regardless of the underlying technology used to display them (like a graphics card driver or drawing library).

The file is structured around three main concepts:

1.  **Theming and Styles (`Styles`):** This defines the application's entire visual identity. It centrally manages screen dimensions, mandatory spacing (`margin`), and specifies which files to use for different font styles (regular, bold). Most importantly, it holds a comprehensive palette of predefined theme colors (e.g., background, primary, secondary), ensuring a unified look.

2.  **Drawing Instructions:** Separate structures define specific rules for drawing. `DrawOptions` dictates the color and thickness for shapes and lines. `DrawTextOptions` controls how text is rendered, including its color, the chosen font, and constraints like maximum width.

3.  **The Drawing Blueprint (`DrawInterface`):** This is an abstract interface—a set of rules that any real drawing system must implement. It guarantees standardized ways to:
    *   **Manage Rendering:** Control when the screen updates (`render`, `triggerRendering`).
    *   **Draw Primitives:** Create basic graphics like lines, circles, filled rectangles, and complex shapes (polygons, rounded corners).
    *   **Handle Text:** Draw text aligned left, centered, or right, adhering to the specified style rules.
    *   **Manage Images/Textures:** Create temporary drawing surfaces and apply images to the screen.
    *   **Scaling and Configuration:** Provide factors for adjusting graphics based on different screen sizes and integrate with external configuration data (like JSON) to customize colors and settings based on the defined `Styles`.

In essence, this file defines the *what* and the *how* of the visuals, allowing the drawing system itself to focus only on the mechanics of putting pixels on the screen.

sha: 1fde6f118b61536a1cf37de34a805406035bb46040994a316f2afe83e6ebaae2
*/
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
    Size screenSizeOrginal;
    Size screenSize;
    DrawInterface(Styles& styles)
        : styles(styles)
        , screenSizeOrginal(styles.screen)
        , screenSize(styles.screen)
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
