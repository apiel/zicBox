#ifndef _UI_DRAW_H_
#define _UI_DRAW_H_

#include "../plugins/components/drawInterface.h"
#include <string>

class Draw : public DrawInterface {
protected:


public:
    Draw(Styles& styles)
        : DrawInterface(styles)
    {
    }

    ~Draw()
    {
    }

    void init()
    {
    }

    void renderNext()
    {
    }

    void triggerRendering()
    {
    }

    void render()
    {
    }

    int textCentered(Point position, std::string text, uint32_t size, DrawTextOptions options = {})
    {
        return 123;
    }

    int text(Point position, std::string text, uint32_t size, DrawTextOptions options = {})
    {
        return 123;
    }

    int textRight(Point position, std::string text, uint32_t size, DrawTextOptions options = {})
    {
        return 123;
    }

    void clear()
    {
    }

    void filledRect(Point position, Size size, DrawOptions options = {})
    {
    }

    void rect(Point position, Size size, DrawOptions options = {})
    {
    }

    void filledRect(Point position, Size size, uint8_t radius, DrawOptions options = {})
    {
        filledRect({ position.x + radius, position.y }, { size.w - 2 * radius, size.h }, options);
        filledRect({ position.x, position.y + radius }, { size.w, size.h - 2 * radius }, options);
        filledPie({ position.x + radius, position.y + radius }, radius, 180, 270, options);
        filledPie({ position.x + size.w - radius, position.y + radius }, radius, 270, 360, options);
        filledPie({ position.x + radius, position.y + size.h - radius }, radius, 90, 180, options);
        filledPie({ position.x + size.w - radius, position.y + size.h - radius }, radius, 0, 90, options);
    }

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

    void arc(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {})
    {
    }

    void filledPie(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {})
    {
    }

    void filledEllipse(Point position, int radiusX, int radiusY, DrawOptions options = {})
    {
    }

    void ellipse(Point position, int radiusX, int radiusY, DrawOptions options = {})
    {
    }

    void filledPolygon(std::vector<Point> points, DrawOptions options = {})
    {
    }

    void polygon(std::vector<Point> points, DrawOptions options = {})
    {
    }

    void aaline(Point start, Point end, DrawOptions options = {})
    {
    }

    void aalines(std::vector<Point> points, DrawOptions options = {})
    {
    }

    void line(Point start, Point end, DrawOptions options = {})
    {
    }

    void lines(std::vector<Point> points, DrawOptions options = {})
    {
    }

    void pixel(Point position, DrawOptions options = {})
    {
    }

    void* setTextureRenderer(Size size)
    {
        return NULL;
    }

    void setMainRenderer()
    {
    }

    void destroyTexture(void* texture)
    {
    }

    void applyTexture(void* texture, Rect dest)
    {
    }

    Color darken(Color color, float amount)
    {
        return color;
    }

    Color lighten(Color color, float amount)
    {
        return color;
    }

    Color alpha(Color color, float amount)
    {
        return color;
    }

    Color getColor(char* color)
    {
        return styles.colors.white;
    }

    // TODO use string...
    void setColor(char* name, char* color)
    {
    }
};

#endif