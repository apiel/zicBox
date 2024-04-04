#ifndef _UI_DRAW_ICON_H_
#define _UI_DRAW_ICON_H_

#include <functional>
#include <string>

#include "../drawInterface.h"

/*md
## Icons

List of icon used in plugin component:
*/
class Icon {
protected:
    DrawInterface& draw;

public:
    enum Align {
        LEFT,
        CENTER,
        RIGHT
    };

    Icon(DrawInterface& draw)
        : draw(draw)
    {
    }

    std::function<void(Point, uint8_t, Color, Align)> get(std::string name)
    {
        // if first char is different than & then it's not an icon
        if (name[0] != '&') {
            return nullptr;
        }

        /*md - `&icon::backspace` */
        if (name == "&icon::backspace") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                backspace(position, size, color, align);
            };
        }

        /*md - `&icon::play` */
        if (name == "&icon::play") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                play(position, size, color, align);
            };
        }

        /*md - `&icon::play::filled` */
        if (name == "&icon::play::filled") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                play(position, size, color, align, true);
            };
        }

        /*md - `&icon::stop` */
        if (name == "&icon::stop") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                stop(position, size, color, align);
            };
        }

        /*md - `&icon::stop::filled` */
        if (name == "&icon::stop::filled") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                stop(position, size, color, align, true);
            };
        }

        /*md - `&icon::pause` */
        if (name == "&icon::pause") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                pause(position, size, color, align);
            };
        }

        /*md - `&icon::pause::filled` */
        if (name == "&icon::pause::filled") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                pause(position, size, color, align, true);
            };
        }

        /*md - `&icon::arrowUp` */
        if (name == "&icon::arrowUp") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                arrowUp(position, size, color, align);
            };
        }

        /*md - `&icon::arrowUp::filled` */
        if (name == "&icon::arrowUp::filled") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                arrowUp(position, size, color, align, true);
            };
        }

        /*md - `&icon::arrowDown` */
        if (name == "&icon::arrowDown") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                arrowDown(position, size, color, align);
            };
        }

        /*md - `&icon::arrowDown::filled` */
        if (name == "&icon::arrowDown::filled") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                arrowDown(position, size, color, align, true);
            };
        }

        return nullptr;
    }

    std::function<void()> get(std::string name, Point position, uint8_t size, Color color, Align align = LEFT)
    {
        std::function<void(Point, uint8_t, Color, Align)> func = get(name);
        if (func) {
            return [func, position, size, color, align]() {
                func(position, size, color, align);
            };
        }
        return nullptr;
    }

    bool render(std::string name, Point position, uint8_t size, Color color, Align align = LEFT)
    {
        std::function<void(Point, uint8_t, Color, Align)> func = get(name);
        if (func) {
            func(position, size, color, align);
            return true;
        }
        return false;
    }

    void backspace(Point position, uint8_t size, Color color, Align align = LEFT)
    {
        int w = size * 1.5;
        int x = getX(position, size, align, w);

        draw.lines({ { (int)(x + w * 0.25), position.y },
                       { x + w, position.y },
                       { x + w, position.y + size },
                       { (int)(x + w * 0.25), position.y + size },
                       { x, (int)(position.y + size * 0.5) },
                       { (int)(x + w * 0.25), position.y } },
            color);

        draw.line(
            { (int)(x + w * 0.4), (int)(position.y + size * 0.25) },
            { (int)(x + w * 0.75), (int)(position.y + size * 0.75) }, color);

        draw.line(
            { (int)(x + w * 0.75), (int)(position.y + size * 0.25) },
            { (int)(x + w * 0.4), (int)(position.y + size * 0.75) }, color);
    }

    void play(Point position, uint8_t size, Color color, Align align = LEFT, bool filled = false)
    {
        int x = getX(position, size, align, size);
        std::vector<Point> points = {
            { x, position.y },
            { x + size, (int)(position.y + size * 0.5) },
            { x, position.y + size }
        };
        if (filled) {
            draw.filledPolygon(points, color);
        } else {
            draw.polygon(points, color);
        }
    }

    void stop(Point position, uint8_t size, Color color, Align align = LEFT, bool filled = false)
    {
        int x = getX(position, size, align, size);
        if (filled) {
            draw.filledRect({ x, position.y }, { size, size }, color);
        } else {
            draw.rect({ x, position.y }, { size, size }, color);
        }
    }

    void pause(Point position, uint8_t size, Color color, Align align = LEFT, bool filled = false)
    {
        int x = getX(position, size, align, size);
        if (filled) {
            draw.filledRect({ x, position.y }, { (int)(size * 0.3), size }, color);
            draw.filledRect({ x + (int)(size * 0.7), position.y }, { (int)(size * 0.3), size }, color);
        } else {
            draw.rect({ x, position.y }, { (int)(size * 0.3), size }, color);
            draw.rect({ x + (int)(size * 0.7), position.y }, { (int)(size * 0.3), size }, color);
        }
    }

    void arrowUp(Point position, uint8_t size, Color color, Align align = LEFT, bool filled = false)
    {
        int x = getX(position, size, align, size);
        std::vector<Point> points = {
            { (int)(x + size * 0.5), position.y },
            { x + size, (int)(position.y + size * 0.5) },
            { (int)(x + size * 0.75), (int)(position.y + size * 0.5) },
            { (int)(x + size * 0.75), position.y + size },
            { (int)(x + size * 0.25), position.y + size },
            { (int)(x + size * 0.25), (int)(position.y + size * 0.5) },
            { x, (int)(position.y + size * 0.5) }
        };
        if (filled) {
            draw.filledPolygon(points, color);
        } else {
            draw.polygon(points, color);
        }
    }

    void arrowDown(Point position, uint8_t size, Color color, Align align = LEFT, bool filled = false)
    {
        int x = getX(position, size, align, size);
        std::vector<Point> points = {
            { (int)(x + size * 0.5), position.y + size },
            { x + size, (int)(position.y + size * 0.5) },
            { (int)(x + size * 0.75), (int)(position.y + size * 0.5) },
            { (int)(x + size * 0.75), position.y },
            { (int)(x + size * 0.25), position.y },
            { (int)(x + size * 0.25), (int)(position.y + size * 0.5) },
            { x, (int)(position.y + size * 0.5) }
        };
        if (filled) {
            draw.filledPolygon(points, color);
        } else {
            draw.polygon(points, color);
        }
    }

protected:
    int getX(Point position, uint8_t size, Align align, int width)
    {
        if (align == CENTER) {
            return position.x - width * 0.5f;
        } else if (align == RIGHT) {
            return position.x - width;
        }
        return position.x;
    }
};

#endif
