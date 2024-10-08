#ifndef _UI_DRAW_INTERFACE_H_
#define _UI_DRAW_INTERFACE_H_

#include <stdint.h>
#include <stdio.h>
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
    uint8_t a;
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

struct DrawTextOptions {
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

    virtual void render() = 0;
    virtual void renderNext() = 0;
    virtual void triggerRendering() = 0;

    DrawTextOptions getDefaultTextOptions(DrawTextOptions options)
    {
        return {
            options.fontPath == nullptr ? styles.font.regular : options.fontPath,
            options.maxWidth == -1 ? styles.screen.w : options.maxWidth
        };
    }

    virtual int textCentered(Point position, std::string text, Color color, uint32_t size, DrawTextOptions options = {}) = 0;
    virtual int text(Point position, std::string text, Color color, uint32_t size, DrawTextOptions options = {}) = 0;
    virtual int textRight(Point position, std::string text, Color color, uint32_t size, DrawTextOptions options = {}) = 0;

    virtual void clear() = 0;
    virtual void filledRect(Point position, Size size, Color color) = 0;
    virtual void rect(Point position, Size size, Color color) = 0;
    virtual void line(Point start, Point end, Color color) = 0;
    virtual void lines(std::vector<Point> points, Color color) = 0;
    virtual void pixel(Point position, Color color) = 0;

    // GFX
    virtual void filledPie(Point position, int radius, int startAngle, int endAngle, Color color) = 0;
    virtual void filledEllipse(Point position, int radiusX, int radiusY, Color color) = 0;
    virtual void ellipse(Point position, int radiusX, int radiusY, Color color) = 0;
    virtual void filledPolygon(std::vector<Point> points, Color color) = 0;
    virtual void polygon(std::vector<Point> points, Color color) = 0;
    virtual void aaline(Point start, Point end, Color color) = 0;
    virtual void aalines(std::vector<Point> points, Color color) = 0;
    virtual void arc(Point position, int radius, int startAngle, int endAngle, Color color) = 0;
    virtual void filledRect(Point position, Size size, Color color, uint8_t radius) = 0;
    virtual void rect(Point position, Size size, Color color, uint8_t radius) = 0;

    virtual void* setTextureRenderer(Size size) = 0;
    virtual void setMainRenderer() = 0;
    virtual void destroyTexture(void* texture) = 0;
    virtual void applyTexture(void* texture, Rect dest) = 0;

    virtual Color darken(Color color, float amount) = 0;
    virtual Color lighten(Color color, float amount) = 0;
    virtual Color alpha(Color color, float amount) = 0;
    virtual Color getColor(char* color) = 0;
    virtual void setColor(char* name, char* color) = 0;
};

#endif