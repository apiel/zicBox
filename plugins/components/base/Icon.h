#ifndef _UI_DRAW_ICON_H_
#define _UI_DRAW_ICON_H_

#include "../drawInterface.h"

class Icon {
protected:
    DrawInterface& draw;

public:
    Icon(DrawInterface& draw)
        : draw(draw)
    {
    }

    void backspace(Point position, uint8_t size, Color color)
    {
        int w = size * 1.5;
        draw.lines({ { (int)(position.x + w * 0.25), position.y },
                       { position.x + w, position.y },
                       { position.x + w, position.y + size },
                       { (int)(position.x + w * 0.25), position.y + size },
                       { position.x, (int)(position.y + size * 0.5) },
                       { (int)(position.x + w * 0.25), position.y } },
            color);

        draw.line(
            { (int)(position.x + w * 0.4), (int)(position.y + size * 0.25) },
            { (int)(position.x + w * 0.75), (int)(position.y + size * 0.75) }, color);

        draw.line(
            { (int)(position.x + w * 0.75), (int)(position.y + size * 0.25) },
            { (int)(position.x + w * 0.4), (int)(position.y + size * 0.75) }, color);
    }
};

#endif
