#ifndef _UI_DRAW_INTERFACE_H_
#define _UI_DRAW_INTERFACE_H_

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

struct Size {
    int w;
    int h;
};

struct Point {
    int x;
    int y;
};

struct Rect {
    Point position;
    Size size;
};

struct Xywh {
    int x;
    int y;
    int w;
    int h;
};

typedef struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a = 255;
} Color;

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
        Color overlay;
        Color darkOverlay;
        Color on;
        Color white;
        Color blue;
        Color red;
        Color turquoise;
        Color grey;
    } colors;
};

struct DrawOptions {
    Color color = { 255, 255, 255, 255 };
    // bool antiAliasing = false;
};

// struct DrawTextOptions {
//     Color color = { 255, 255, 255, 255 };
//     const char* fontPath = nullptr;
//     int maxWidth = -1;
// };

struct DrawTextOptions : DrawOptions {
    const char* fontPath = nullptr;
    int maxWidth = -1;
};

class DrawInterface {
public:
    Styles& styles;
    DrawInterface(Styles& styles)
        : styles(styles)
    {
    }

    virtual void init() = 0;
    virtual void render() = 0;
    virtual void renderNext() = 0;
    virtual void triggerRendering() = 0;

    DrawTextOptions getDefaultTextOptions(DrawTextOptions options)
    {
        return {
            options.color,
            options.fontPath == nullptr ? styles.font.regular : options.fontPath,
            options.maxWidth == -1 ? styles.screen.w : options.maxWidth
        };
    }

    virtual int textCentered(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) = 0;
    virtual int text(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) = 0;
    virtual int textRight(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) = 0;

    virtual void clear() = 0;
    virtual void line(Point start, Point end, DrawOptions options = {}) = 0;
    virtual void lines(std::vector<Point> points, DrawOptions options = {}) = 0;
    virtual void pixel(Point position, DrawOptions options = {}) = 0;
    virtual void filledRect(Point position, Size size, DrawOptions options = {}) = 0;
    virtual void rect(Point position, Size size, DrawOptions options = {}) = 0;

    // GFX
    virtual void filledRect(Point position, Size size, uint8_t radius, DrawOptions options = {}) = 0;
    virtual void rect(Point position, Size size, uint8_t radius, DrawOptions options = {}) = 0;
    virtual void filledPie(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {}) = 0;
    virtual void filledEllipse(Point position, int radiusX, int radiusY, DrawOptions options = {}) = 0;
    virtual void ellipse(Point position, int radiusX, int radiusY, DrawOptions options = {}) = 0;
    virtual void filledPolygon(std::vector<Point> points, DrawOptions options = {}) = 0;
    virtual void polygon(std::vector<Point> points, DrawOptions options = {}) = 0;
    virtual void aaline(Point start, Point end, DrawOptions options = {}) = 0;
    virtual void aalines(std::vector<Point> points, DrawOptions options = {}) = 0;
    virtual void arc(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {}) = 0;
    virtual void circle(Point position, int radius, DrawOptions options = {}) = 0;
    virtual void filledCircle(Point position, int radius, DrawOptions options = {}) = 0;

    virtual void* setTextureRenderer(Size size) = 0;
    virtual void setMainRenderer() = 0;
    virtual void destroyTexture(void* texture) = 0;
    virtual void applyTexture(void* texture, Rect dest) = 0;

    virtual Color getColor(char* color) = 0;
    virtual bool config(char* key, char* value) = 0;
};

#endif