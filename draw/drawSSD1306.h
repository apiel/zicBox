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

    void filledRect(Point position, Size size, Color color)
    {
    }

    void rect(Point position, Size size, Color color)
    {
    }

    void filledRect(Point position, Size size, Color color, uint8_t radius)
    {
        filledRect({ position.x + radius, position.y }, { size.w - 2 * radius, size.h }, color);
        filledRect({ position.x, position.y + radius }, { size.w, size.h - 2 * radius }, color);
        filledPie({ position.x + radius, position.y + radius }, radius, 180, 270, color);
        filledPie({ position.x + size.w - radius, position.y + radius }, radius, 270, 360, color);
        filledPie({ position.x + radius, position.y + size.h - radius }, radius, 90, 180, color);
        filledPie({ position.x + size.w - radius, position.y + size.h - radius }, radius, 0, 90, color);
    }

    void rect(Point position, Size size, Color color, uint8_t radius)
    {
        arc({ position.x + radius, position.y + radius }, radius, 180, 270, color);
        line({ position.x, position.y + radius }, { position.x, position.y + size.h - radius }, color);
        arc({ position.x + size.w - radius, position.y + radius }, radius, 270, 360, color);
        line({ position.x + radius, position.y }, { position.x + size.w - radius, position.y }, color);
        arc({ position.x + radius, position.y + size.h - radius }, radius, 90, 180, color);
        line({ position.x + radius, position.y + size.h }, { position.x + size.w - radius, position.y + size.h }, color);
        arc({ position.x + size.w - radius, position.y + size.h - radius }, radius, 0, 90, color);
        line({ position.x + size.w, position.y + size.h - radius }, { position.x + size.w, position.y + radius }, color);
    }

    void arc(Point position, int radius, int startAngle, int endAngle, Color color)
    {
    }

    void filledPie(Point position, int radius, int startAngle, int endAngle, Color color)
    {
    }

    void filledEllipse(Point position, int radiusX, int radiusY, Color color)
    {
    }

    void ellipse(Point position, int radiusX, int radiusY, Color color)
    {
    }

    void filledPolygon(std::vector<Point> points, Color color)
    {
    }

    void polygon(std::vector<Point> points, Color color)
    {
    }

    void aaline(Point start, Point end, Color color)
    {
    }

    void aalines(std::vector<Point> points, Color color)
    {
    }

    void line(Point start, Point end, Color color)
    {
    }

    void lines(std::vector<Point> points, Color color)
    {
    }

    void pixel(Point position, Color color)
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