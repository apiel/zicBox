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
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
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
    int fontHeight = 0;
};

class DrawInterface {
public:
    Styles& styles;
    DrawInterface(Styles& styles)
        : styles(styles)
    {
    }

    virtual void init() { }
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

    virtual void* getFont(const char* name = NULL, int size = -1) { printf("yoooooooooo\n"); return NULL; }
    virtual Color getColor(char* color) { return styles.colors.white; }
    virtual bool config(char* key, char* value) { return false; }
};

#endif