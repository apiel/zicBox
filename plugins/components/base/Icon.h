/** Description:
This C++ header file defines a centralized library responsible for managing and generating user interface icons. It serves as a blueprint for all standardized graphical symbols used within the application's components.

The core of the system is the `Icon` class. It does not perform the actual drawing but relies entirely on an external "Draw Interface" to render shapes (like lines, circles, and polygons) onto the screen.

**How it works:**
The system stores specific geometric instructions for a wide range of symbols, including media controls (play, pause, stop), navigation arrows, backspace, trash, and toggle switches. To use an icon, you request it by a unique text name (e.g., `&icon::arrowUp`).

The system then provides a dedicated function ready to draw that specific image. This drawing function accepts parameters to control:

1.  **Position:** The location on the screen.
2.  **Size:** The overall dimensions of the icon.
3.  **Color:** The color to be used for rendering.
4.  **Alignment:** How the icon is positioned relative to the coordinates (left, center, or right justified).

Many icons also feature "filled" variants for solid shapes versus simple outlines. This setup ensures that every part of the user interface can consistently access and display high-quality, standardized graphical assets by name.

sha: 4af919b81c6b8104d3ddc218a04a68ce75ac4f33383f207186fc1524be41cb12 
*/
#ifndef _UI_DRAW_ICON_H_
#define _UI_DRAW_ICON_H_

#include <functional>
#include <string>

#include "../drawInterface.h"
#include "plugins/components/utils/color.h"

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

        /*md - `&icon::backspace::filled` */
        if (name == "&icon::backspace::filled") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                backspaceFilled(position, size, color, align);
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

        /*md - `&icon::arrowLeft` */
        if (name == "&icon::arrowLeft") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                arrowLeft(position, size, color, align);
            };
        }

        /*md - `&icon::arrowLeft::filled` */
        if (name == "&icon::arrowLeft::filled") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                arrowLeft(position, size, color, align, true);
            };
        }

        /*md - `&icon::arrowRight` */
        if (name == "&icon::arrowRight") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                arrowRight(position, size, color, align);
            };
        }

        /*md - `&icon::arrowRight::filled` */
        if (name == "&icon::arrowRight::filled") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                arrowRight(position, size, color, align, true);
            };
        }

        /*md - `&icon::musicNote` */
        if (name == "&icon::musicNote") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                musicNote(position, size, color, align);
            };
        }

        /*md - `&icon::musicNote::pixelated` */
        if (name == "&icon::musicNote::pixelated") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                musicNotePixelated(position, size, color, align);
            };
        }

        /*md - `&icon::tape` */
        if (name == "&icon::tape") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                tape(position, size, color, align);
            };
        }

        /*md - `&icon::toggle` */
        if (name == "&icon::toggle") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                toggleRect(position, size, color, align);
            };
        }

        /*md - `&icon::trash` */
        if (name == "&icon::trash") {
            return [&](Point position, uint8_t size, Color color, Align align) {
                trash(position, size, color, align);
            };
        }

        /*md - `&empty` */
        if (name == "&empty") {
            return [&](Point position, uint8_t size, Color color, Align align) { };
        }

        // printf("Unknown icon: %s\n", name.c_str());

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
            { color });

        draw.line(
            { (int)(x + w * 0.4), (int)(position.y + size * 0.25) },
            { (int)(x + w * 0.75), (int)(position.y + size * 0.75) }, { color });

        draw.line(
            { (int)(x + w * 0.75), (int)(position.y + size * 0.25) },
            { (int)(x + w * 0.4), (int)(position.y + size * 0.75) }, { color });
    }

    void backspaceFilled(Point position, uint8_t size, Color color, Align align = LEFT)
    {
        int w = size * 1.5;
        int x = getX(position, size, align, w);

        draw.filledPolygon({ { (int)(x + w * 0.25), position.y },
                               { x + w, position.y },
                               { x + w, position.y + size },
                               { (int)(x + w * 0.25), position.y + size },
                               { x, (int)(position.y + size * 0.5) },
                               { (int)(x + w * 0.25), position.y } },
            { color });
    }

    void play(Point position, uint8_t size, Color color, Align align = LEFT, bool filled = false)
    {
        int x = getX(position, size, align, size);
        std::vector<Point> points = {
            { x, position.y },
            { x + size, (int)(position.y + size * 0.5) },
            { x, position.y + size },
            { x, position.y },
        };
        if (filled) {
            draw.filledPolygon(points, { color });
        } else {
            draw.lines(points, { color });
        }
    }

    void stop(Point position, uint8_t size, Color color, Align align = LEFT, bool filled = false)
    {
        int x = getX(position, size, align, size);
        if (filled) {
            draw.filledRect({ x, position.y }, { size, size }, { color });
        } else {
            draw.rect({ x, position.y }, { size, size }, { color });
        }
    }

    void pause(Point position, uint8_t size, Color color, Align align = LEFT, bool filled = false)
    {
        int x = getX(position, size, align, size);
        if (filled) {
            draw.filledRect({ x, position.y }, { (int)(size * 0.3), size }, { color });
            draw.filledRect({ x + (int)(size * 0.7), position.y }, { (int)(size * 0.3), size }, { color });
        } else {
            draw.rect({ x, position.y }, { (int)(size * 0.3), size }, { color });
            draw.rect({ x + (int)(size * 0.7), position.y }, { (int)(size * 0.3), size }, { color });
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
            { x, (int)(position.y + size * 0.5) },
            { (int)(x + size * 0.5), position.y },
        };
        if (filled) {
            draw.filledPolygon(points, { color });
        } else {
            draw.lines(points, { color });
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
            { x, (int)(position.y + size * 0.5) },
            { (int)(x + size * 0.5), position.y + size },
        };
        if (filled) {
            draw.filledPolygon(points, { color });
        } else {
            draw.lines(points, { color });
        }
    }

    void arrowLeft(Point position, uint8_t size, Color color, Align align = LEFT, bool filled = false)
    {
        int x = getX(position, size, align, size);
        std::vector<Point> points = {
            { (int)(x + size * 0.5), (int)(position.y + size * 0.25) },
            { (int)(x + size * 0.5), position.y },
            { x, (int)(position.y + size * 0.5) },
            { (int)(x + size * 0.5), position.y + size },
            { (int)(x + size * 0.5), (int)(position.y + size * 0.75) },
            { x + size, (int)(position.y + size * 0.75) },
            { x + size, (int)(position.y + size * 0.25) },
            { (int)(x + size * 0.5), (int)(position.y + size * 0.25) },
        };
        if (filled) {
            draw.filledPolygon(points, { color });
        } else {
            draw.lines(points, { color });
        }
    }

    void arrowRight(Point position, uint8_t size, Color color, Align align = LEFT, bool filled = false)
    {
        int x = getX(position, size, align, size);
        std::vector<Point> points = {
            { (int)(x + size * 0.5), (int)(position.y + size * 0.25) },
            { (int)(x + size * 0.5), position.y + size },
            { x + size, (int)(position.y + size * 0.5) },
            { (int)(x + size * 0.5), position.y },
            { (int)(x + size * 0.5), (int)(position.y + size * 0.75) },
            { x, (int)(position.y + size * 0.75) },
            { x, (int)(position.y + size * 0.25) },
            { (int)(x + size * 0.5), (int)(position.y + size * 0.25) },
        };
        if (filled) {
            draw.filledPolygon(points, { color });
        } else {
            draw.lines(points, { color });
        }
    }

    void musicNote(Point position, uint8_t size, Color color, Align align = LEFT)
    {
        int x = getX(position, size, align, size);
        std::vector<Point> points = {
            { (int)(x + size * 0.75), (int)(position.y + size * 0.25) },
            { (int)(x + size * 0.5), (int)(position.y) },
            { (int)(x + size * 0.5), (int)(position.y + size * 0.75) },
        };
        draw.lines(points, { color });
        draw.filledCircle({ (int)(x + size * 0.4), (int)(position.y + size * 0.75) }, size * 0.25, { color });
    }

    void musicNotePixelated(Point position, uint8_t size, Color color, Align align = LEFT)
    {
        int x = getX(position, size, align, size);
        std::vector<Point> points = {
            { (int)(x + size * 0.75), (int)(position.y + size * 0.25) },
            { (int)(x + size * 0.5), (int)(position.y) },
            { (int)(x + size * 0.5), (int)(position.y + size * 0.75) },
        };
        draw.lines(points, { color });
        // use rect instead of filledCircle because filledCircle is pixelated
        draw.filledRect({ (int)(x + size * 0.25), (int)(position.y + size * 0.75) }, { (int)(size * 0.25), (int)(size * 0.25) }, { color });
    }

    void tape(Point position, uint8_t size, Color color, Align align = LEFT)
    {
        int x = getX(position, size, align, size);
        // draw.line({ x, position.y + 1 }, { x + size, position.y + 1 }, { color });
        draw.filledCircle({ x, (int)(position.y + size * 0.6) }, size * 0.4, { color });
        draw.filledCircle({ x + size, (int)(position.y + size * 0.6) }, size * 0.4, { color });
        draw.line({ x, (int)(position.y + size * 0.8) }, { x + size, (int)(position.y + size * 0.8) }, { color });
    }

    void toggleRect(Point position, uint8_t size, Color color, Align align = LEFT)
    {
        int x = getX(position, size, align, size);
        draw.rect({ (int)(x - size * 0.5), position.y }, { size * 2, size }, { color });
        // draw.filledCircle({ (int)(x + size * 0.1), (int)(position.y + size * 0.5) }, size * 0.25, { color });
        draw.filledRect({ (int)(x - size * 0.5) + 2, position.y + 2 }, { size - 4, size - 4 }, { color });
    }

    void trash(Point position, uint8_t size, Color color, Align align = LEFT)
    {
        int x = getX(position, size, align, size);
        // draw.rect({ (int)(x - size * 0.5), position.y + 1 }, { size, size - 1}, { color });
        // draw.line({ (int)(x - size * 0.75), position.y + 1 }, { (int)(x + size * 0.75), position.y + 1 }, { color });
        // draw.line({ (int)(x - size * 0.2), position.y }, { (int)(x + size * 0.2), position.y }, { color });

        draw.rect({ x, position.y + 1 }, { size, size - 1 }, { color });
        int edge = (int)(size * 0.3);
        draw.line({ x - edge, position.y + 1 }, { x + size + edge, position.y + 1 }, { color });
        draw.line({ (int)(x + size * 0.3), position.y }, { (int)(x + size * 0.7), position.y }, { color });
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
