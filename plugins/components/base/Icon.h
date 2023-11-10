#ifndef _UI_DRAW_ICON_H_
#define _UI_DRAW_ICON_H_

#include <string>

#include "../drawInterface.h"

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

    bool render(std::string name, Point position, uint8_t size, Color color, Align align = LEFT)
    {
        if (name == "&icon::backspace") {
            backspace(position, size, color, align);
            return true;
        }
        return false;
    }

    void backspace(Point position, uint8_t size, Color color, Align align = LEFT)
    {
        int w = size * 1.5;
        int x = position.x;
        if (align == CENTER) {
            x = position.x - w / 2;
        } else if (align == RIGHT) {
            x = position.x - w;
        }

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
};

#endif
