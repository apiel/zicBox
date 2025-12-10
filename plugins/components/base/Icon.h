#pragma once

#include "../drawInterface.h"
#include "plugins/components/utils/color.h"
#include <cmath> // for std::round
#include <functional>
#include <string>
#include <vector>

/*
  Icon library — updated to correctly fit icons into arbitrary Size {w,h}
  Key points:
  - Each icon has an intrinsic design width/height (designWidth, designHeight).
  - We compute scale = min(boxW/designWidth, boxH/designHeight) so the whole
    icon fits inside the provided rectangle without overflowing.
  - We compute integer pixel width/height and a base/top offset to center the icon.
  - Rectangles / circles receive integer sizes >= 1 to avoid zero-size drawing.
*/

class Icon {
protected:
    DrawInterface& draw;

    struct Transform {
        int baseX; // integer pixel X where icon origin will be drawn
        int baseY; // integer pixel Y where icon origin will be drawn
        int pixelWidth; // integer pixel width of the scaled design
        int pixelHeight; // integer pixel height of the scaled design
        float scale; // floating point scale used (design -> pixels)
    };

    // Compute a transform that scales a (designWidth x designHeight) icon
    // to fit inside the provided boxSize (boxWidth x boxHeight) at position (boxX, boxY).
    Transform computeTransform(Point boxOrigin, Size boxSize, float designWidth, float designHeight)
    {
        // avoid division by zero
        if (designWidth <= 0.0f) designWidth = 1.0f;
        if (designHeight <= 0.0f) designHeight = 1.0f;

        float scaleX = static_cast<float>(boxSize.w) / designWidth;
        float scaleY = static_cast<float>(boxSize.h) / designHeight;

        // uniform scale to keep aspect ratio and prevent overflow
        float scale = std::min(scaleX, scaleY);

        // compute integer pixel dimensions (round to nearest)
        int pixelWidth = std::max(1, static_cast<int>(std::round(designWidth * scale)));
        int pixelHeight = std::max(1, static_cast<int>(std::round(designHeight * scale)));

        // center the icon inside the box
        int baseX = boxOrigin.x + (boxSize.w - pixelWidth) / 2;
        int baseY = boxOrigin.y + (boxSize.h - pixelHeight) / 2;

        return { baseX, baseY, pixelWidth, pixelHeight, scale };
    }

public:
    Icon(DrawInterface& drawInterface)
        : draw(drawInterface)
    {
    }

    // get returns a function that draws an icon into a rectangle (position + size)
    std::function<void(Point, Size, Color)> get(std::string name)
    {
        if (name.empty() || name[0] != '&') return nullptr;

        if (name == "&icon::backspace") {
            return [this](Point pos, Size s, Color c) { backspace(pos, s, c); };
        }
        if (name == "&icon::backspace::filled") {
            return [this](Point pos, Size s, Color c) { backspaceFilled(pos, s, c); };
        }
        if (name == "&icon::play") {
            return [this](Point pos, Size s, Color c) { play(pos, s, c); };
        }
        if (name == "&icon::play::filled") {
            return [this](Point pos, Size s, Color c) { play(pos, s, c, true); };
        }
        if (name == "&icon::stop") {
            return [this](Point pos, Size s, Color c) { stop(pos, s, c); };
        }
        if (name == "&icon::stop::filled") {
            return [this](Point pos, Size s, Color c) { stop(pos, s, c, true); };
        }
        if (name == "&icon::pause") {
            return [this](Point pos, Size s, Color c) { pause(pos, s, c); };
        }
        if (name == "&icon::pause::filled") {
            return [this](Point pos, Size s, Color c) { pause(pos, s, c, true); };
        }
        if (name == "&icon::arrowUp") {
            return [this](Point pos, Size s, Color c) { arrowUp(pos, s, c); };
        }
        if (name == "&icon::arrowUp::filled") {
            return [this](Point pos, Size s, Color c) { arrowUp(pos, s, c, true); };
        }
        if (name == "&icon::arrowDown") {
            return [this](Point pos, Size s, Color c) { arrowDown(pos, s, c); };
        }
        if (name == "&icon::arrowDown::filled") {
            return [this](Point pos, Size s, Color c) { arrowDown(pos, s, c, true); };
        }
        if (name == "&icon::arrowLeft") {
            return [this](Point pos, Size s, Color c) { arrowLeft(pos, s, c); };
        }
        if (name == "&icon::arrowLeft::filled") {
            return [this](Point pos, Size s, Color c) { arrowLeft(pos, s, c, true); };
        }
        if (name == "&icon::arrowRight") {
            return [this](Point pos, Size s, Color c) { arrowRight(pos, s, c); };
        }
        if (name == "&icon::arrowRight::filled") {
            return [this](Point pos, Size s, Color c) { arrowRight(pos, s, c, true); };
        }
        if (name == "&icon::musicNote") {
            return [this](Point pos, Size s, Color c) { musicNote(pos, s, c); };
        }
        if (name == "&icon::musicNote::pixelated") {
            return [this](Point pos, Size s, Color c) { musicNotePixelated(pos, s, c); };
        }
        if (name == "&icon::tape") {
            return [this](Point pos, Size s, Color c) { tape(pos, s, c); };
        }
        if (name == "&icon::toggle") {
            return [this](Point pos, Size s, Color c) { toggleRect(pos, s, c); };
        }
        if (name == "&icon::trash") {
            return [this](Point pos, Size s, Color c) { trash(pos, s, c); };
        }
        if (name == "&empty") {
            return [](Point, Size, Color) { };
        }

        return nullptr;
    }

    bool render(std::string name, Point pos, int size, Color color)
    {
        return render(name, pos, { size, size }, color);
    }

    bool render(std::string name, Point pos, Size size, Color color)
    {
        auto drawFunc = get(name);
        if (!drawFunc) return false;
        drawFunc(pos, size, color);
        return true;
    }

    // --- ICON IMPLEMENTATIONS ---
    // Each icon uses a design coordinate system:
    // Most icons use designWidth=100, designHeight=100 (square).
    // Backspace uses designWidth=150, designHeight=100 (wider).

    void backspace(Point boxOrigin, Size boxSize, Color color)
    {
        // Intrinsic design: width = 150, height = 100 (same as earlier *1.5)
        Transform transform = computeTransform(boxOrigin, boxSize, 150.0f, 100.0f);

        int leftX = transform.baseX;
        int topY = transform.baseY;
        int pixelWidth = transform.pixelWidth; // corresponds to designWidth * scale
        int pixelHeight = transform.pixelHeight; // corresponds to designHeight * scale

        // Use integer geometry derived from the scaled pixelWidth/pixelHeight
        int p0x = leftX + static_cast<int>(std::round(pixelWidth * 0.25f));
        int p0y = topY;
        int p1x = leftX + pixelWidth;
        int p1y = topY;
        int p2x = leftX + pixelWidth;
        int p2y = topY + pixelHeight;
        int p3x = leftX + static_cast<int>(std::round(pixelWidth * 0.25f));
        int p3y = topY + pixelHeight;
        int p4x = leftX;
        int p4y = topY + static_cast<int>(std::round(pixelHeight * 0.5f));

        draw.lines({ { p0x, p0y },
                       { p1x, p1y },
                       { p2x, p2y },
                       { p3x, p3y },
                       { p4x, p4y },
                       { p0x, p0y } },
            { color });

        // X mark inside backspace
        draw.line(
            { static_cast<int>(std::round(leftX + pixelWidth * 0.4f)), static_cast<int>(std::round(topY + pixelHeight * 0.25f)) },
            { static_cast<int>(std::round(leftX + pixelWidth * 0.75f)), static_cast<int>(std::round(topY + pixelHeight * 0.75f)) },
            { color });

        draw.line(
            { static_cast<int>(std::round(leftX + pixelWidth * 0.75f)), static_cast<int>(std::round(topY + pixelHeight * 0.25f)) },
            { static_cast<int>(std::round(leftX + pixelWidth * 0.4f)), static_cast<int>(std::round(topY + pixelHeight * 0.75f)) },
            { color });
    }

    void backspaceFilled(Point boxOrigin, Size boxSize, Color color)
    {
        Transform transform = computeTransform(boxOrigin, boxSize, 150.0f, 100.0f);

        int leftX = transform.baseX;
        int topY = transform.baseY;
        int pixelWidth = transform.pixelWidth;
        int pixelHeight = transform.pixelHeight;

        draw.filledPolygon({ { leftX + static_cast<int>(std::round(pixelWidth * 0.25f)), topY },
                               { leftX + pixelWidth, topY },
                               { leftX + pixelWidth, topY + pixelHeight },
                               { leftX + static_cast<int>(std::round(pixelWidth * 0.25f)), topY + pixelHeight },
                               { leftX, topY + static_cast<int>(std::round(pixelHeight * 0.5f)) },
                               { leftX + static_cast<int>(std::round(pixelWidth * 0.25f)), topY } },
            { color });
    }

    void play(Point boxOrigin, Size boxSize, Color color, bool filled = false)
    {
        Transform transform = computeTransform(boxOrigin, boxSize, 100.0f, 100.0f);

        int leftX = transform.baseX;
        int topY = transform.baseY;
        int pixelSize = std::min(transform.pixelWidth, transform.pixelHeight);

        std::vector<Point> polygon = {
            { leftX, topY },
            { leftX + pixelSize, topY + static_cast<int>(std::round(pixelSize * 0.5f)) },
            { leftX, topY + pixelSize },
            { leftX, topY }
        };

        if (filled) draw.filledPolygon(polygon, { color });
        else draw.lines(polygon, { color });
    }

    void stop(Point boxOrigin, Size boxSize, Color color, bool filled = false)
    {
        Transform transform = computeTransform(boxOrigin, boxSize, 100.0f, 100.0f);

        int leftX = transform.baseX;
        int topY = transform.baseY;
        int pixelSize = std::min(transform.pixelWidth, transform.pixelHeight);

        // ensure width/height passed to rect are >= 1
        int rectWidth = std::max(1, pixelSize);
        int rectHeight = std::max(1, pixelSize);

        if (filled) draw.filledRect({ leftX, topY }, { rectWidth, rectHeight }, { color });
        else draw.rect({ leftX, topY }, { rectWidth, rectHeight }, { color });
    }

    void pause(Point boxOrigin, Size boxSize, Color color, bool filled = false)
    {
        Transform transform = computeTransform(boxOrigin, boxSize, 100.0f, 100.0f);

        int leftX = transform.baseX;
        int topY = transform.baseY;
        int pixelSize = std::min(transform.pixelWidth, transform.pixelHeight);

        int barWidth = std::max(1, static_cast<int>(std::round(pixelSize * 0.3f)));
        int barHeight = std::max(1, pixelSize);

        if (filled) {
            draw.filledRect({ leftX, topY }, { barWidth, barHeight }, { color });
            draw.filledRect({ leftX + static_cast<int>(std::round(pixelSize * 0.7f)), topY }, { barWidth, barHeight }, { color });
        } else {
            draw.rect({ leftX, topY }, { barWidth, barHeight }, { color });
            draw.rect({ leftX + static_cast<int>(std::round(pixelSize * 0.7f)), topY }, { barWidth, barHeight }, { color });
        }
    }

    void arrowUp(Point boxOrigin, Size boxSize, Color color, bool filled = false)
    {
        Transform transform = computeTransform(boxOrigin, boxSize, 100.0f, 100.0f);
        int leftX = transform.baseX;
        int topY = transform.baseY;
        int pixelSize = std::min(transform.pixelWidth, transform.pixelHeight);

        std::vector<Point> polygon = {
            { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY },
            { leftX + pixelSize, topY + static_cast<int>(std::round(pixelSize * 0.5f)) },
            { leftX + static_cast<int>(std::round(pixelSize * 0.75f)), topY + static_cast<int>(std::round(pixelSize * 0.5f)) },
            { leftX + static_cast<int>(std::round(pixelSize * 0.75f)), topY + pixelSize },
            { leftX + static_cast<int>(std::round(pixelSize * 0.25f)), topY + pixelSize },
            { leftX + static_cast<int>(std::round(pixelSize * 0.25f)), topY + static_cast<int>(std::round(pixelSize * 0.5f)) },
            { leftX, topY + static_cast<int>(std::round(pixelSize * 0.5f)) },
            { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY }
        };

        if (filled) draw.filledPolygon(polygon, { color });
        else draw.lines(polygon, { color });
    }

    void arrowDown(Point boxOrigin, Size boxSize, Color color, bool filled = false)
    {
        Transform transform = computeTransform(boxOrigin, boxSize, 100.0f, 100.0f);
        int leftX = transform.baseX;
        int topY = transform.baseY;
        int pixelSize = std::min(transform.pixelWidth, transform.pixelHeight);

        std::vector<Point> polygon = {
            { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY + pixelSize },
            { leftX + pixelSize, topY + static_cast<int>(std::round(pixelSize * 0.5f)) },
            { leftX + static_cast<int>(std::round(pixelSize * 0.75f)), topY + static_cast<int>(std::round(pixelSize * 0.5f)) },
            { leftX + static_cast<int>(std::round(pixelSize * 0.75f)), topY },
            { leftX + static_cast<int>(std::round(pixelSize * 0.25f)), topY },
            { leftX + static_cast<int>(std::round(pixelSize * 0.25f)), topY + static_cast<int>(std::round(pixelSize * 0.5f)) },
            { leftX, topY + static_cast<int>(std::round(pixelSize * 0.5f)) },
            { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY + pixelSize }
        };

        if (filled) draw.filledPolygon(polygon, { color });
        else draw.lines(polygon, { color });
    }

    void arrowLeft(Point boxOrigin, Size boxSize, Color color, bool filled = false)
    {
        Transform transform = computeTransform(boxOrigin, boxSize, 100.0f, 100.0f);
        int leftX = transform.baseX;
        int topY = transform.baseY;
        int pixelSize = std::min(transform.pixelWidth, transform.pixelHeight);

        std::vector<Point> polygon = {
            { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY + static_cast<int>(std::round(pixelSize * 0.25f)) },
            { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY },
            { leftX, topY + static_cast<int>(std::round(pixelSize * 0.5f)) },
            { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY + pixelSize },
            { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY + static_cast<int>(std::round(pixelSize * 0.75f)) },
            { leftX + pixelSize, topY + static_cast<int>(std::round(pixelSize * 0.75f)) },
            { leftX + pixelSize, topY + static_cast<int>(std::round(pixelSize * 0.25f)) },
            { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY + static_cast<int>(std::round(pixelSize * 0.25f)) }
        };

        if (filled) draw.filledPolygon(polygon, { color });
        else draw.lines(polygon, { color });
    }

    void arrowRight(Point boxOrigin, Size boxSize, Color color, bool filled = false)
    {
        Transform transform = computeTransform(boxOrigin, boxSize, 100.0f, 100.0f);
        int leftX = transform.baseX;
        int topY = transform.baseY;
        int pixelSize = std::min(transform.pixelWidth, transform.pixelHeight);

        std::vector<Point> polygon = {
            // Start at left-upper
            { leftX, topY + static_cast<int>(std::round(pixelSize * 0.25f)) },
            { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY + static_cast<int>(std::round(pixelSize * 0.25f)) },
            { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY },
            { leftX + pixelSize, topY + static_cast<int>(std::round(pixelSize * 0.5f)) },
            { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY + pixelSize },
            { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY + static_cast<int>(std::round(pixelSize * 0.75f)) },
            { leftX, topY + static_cast<int>(std::round(pixelSize * 0.75f)) },
            // Closing point: repeat first (as you originally did)
            { leftX, topY + static_cast<int>(std::round(pixelSize * 0.25f)) }
        };

        if (filled) draw.filledPolygon(polygon, { color });
        else draw.lines(polygon, { color });
    }

    void musicNote(Point boxOrigin, Size boxSize, Color color)
    {
        Transform transform = computeTransform(boxOrigin, boxSize, 100.0f, 100.0f);
        int leftX = transform.baseX;
        int topY = transform.baseY;
        int pixelSize = std::min(transform.pixelWidth, transform.pixelHeight);

        draw.lines({ { leftX + static_cast<int>(std::round(pixelSize * 0.75f)), topY + static_cast<int>(std::round(pixelSize * 0.25f)) },
                       { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY },
                       { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY + static_cast<int>(std::round(pixelSize * 0.75f)) } },
            { color });

        int circleCenterX = leftX + static_cast<int>(std::round(pixelSize * 0.4f));
        int circleCenterY = topY + static_cast<int>(std::round(pixelSize * 0.75f));
        int circleRadius = std::max(1, static_cast<int>(std::round(pixelSize * 0.25f)));

        draw.filledCircle({ circleCenterX, circleCenterY }, circleRadius, { color });
    }

    void musicNotePixelated(Point boxOrigin, Size boxSize, Color color)
    {
        Transform transform = computeTransform(boxOrigin, boxSize, 100.0f, 100.0f);
        int leftX = transform.baseX;
        int topY = transform.baseY;
        int pixelSize = std::min(transform.pixelWidth, transform.pixelHeight);

        draw.lines({ { leftX + static_cast<int>(std::round(pixelSize * 0.75f)), topY + static_cast<int>(std::round(pixelSize * 0.25f)) },
                       { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY },
                       { leftX + static_cast<int>(std::round(pixelSize * 0.5f)), topY + static_cast<int>(std::round(pixelSize * 0.75f)) } },
            { color });

        int rectLeft = leftX + static_cast<int>(std::round(pixelSize * 0.25f));
        int rectTop = topY + static_cast<int>(std::round(pixelSize * 0.75f));
        int rectW = std::max(1, static_cast<int>(std::round(pixelSize * 0.25f)));
        int rectH = std::max(1, static_cast<int>(std::round(pixelSize * 0.25f)));

        draw.filledRect({ rectLeft, rectTop }, { rectW, rectH }, { color });
    }

    void tape(Point boxOrigin, Size boxSize, Color color)
    {
        Transform transform = computeTransform(boxOrigin, boxSize, 100.0f, 100.0f);
        int leftX = transform.baseX;
        int topY = transform.baseY;
        int pixelSize = std::min(transform.pixelWidth, transform.pixelHeight);

        // Inset circles horizontally to avoid overflow
        int circleRadius = std::max(1, static_cast<int>(std::round(pixelSize * 0.20f)));

        int leftCircleX = leftX + circleRadius;
        int rightCircleX = leftX + pixelSize - circleRadius;

        // Vertical position lowered slightly so bottom stays inside
        int leftCircleY = topY + static_cast<int>(std::round(pixelSize * 0.60f));
        int rightCircleY = leftCircleY;

        draw.filledCircle({ leftCircleX, leftCircleY }, circleRadius, { color });
        draw.filledCircle({ rightCircleX, rightCircleY }, circleRadius, { color });

        // Horizontal bar slightly above circles
        int barY = topY + static_cast<int>(std::round(pixelSize * 0.75f));

        draw.line({ leftCircleX, barY }, { rightCircleX, barY }, { color });
    }

    void toggleRect(Point boxOrigin, Size boxSize, Color color)
    {
        // Treat toggle as 2:1 width:height design
        const float designWidth = 200.0f;
        const float designHeight = 100.0f;

        Transform transform = computeTransform(boxOrigin, boxSize, designWidth, designHeight);

        int baseX = transform.baseX;
        int baseY = transform.baseY;
        int scaledWidth = transform.pixelWidth; // corresponds to designWidth * scale
        int scaledHeight = transform.pixelHeight; // corresponds to designHeight * scale

        // --- Outer rectangle (the track) ---
        // Keep the outer rect exactly the scaled design size (fits inside box).
        int outerLeft = baseX;
        int outerTop = baseY;
        int outerWidth = std::max(1, scaledWidth);
        int outerHeight = std::max(1, scaledHeight);

        draw.rect({ outerLeft, outerTop }, { outerWidth, outerHeight }, { color });

        // --- Inner knob (the slider) ---
        // Provide a small padding inside the outer rect so knob doesn't touch edges.
        // Padding chosen as 6% of outerHeight (rounded) but at least 1 px.
        int knobPadding = std::max(1, static_cast<int>(std::round(outerHeight * 0.06f)));

        // Knob size: slightly smaller than the outer height (so it's visually inset)
        int knobWidth = std::max(1, outerHeight - knobPadding * 2);
        int knobHeight = std::max(1, outerHeight - knobPadding * 2);

        // Place knob on the left by default (you can place it on the right for "on" state):
        int knobLeft = outerLeft + knobPadding;
        int knobTop = outerTop + knobPadding;

        // If you later want an "on/off" knob position, compute:
        // int knobLeftOn  = outerLeft + outerWidth - knobPadding - knobWidth;
        // knobLeft = (isOn ? knobLeftOn : knobLeft);

        draw.filledRect({ knobLeft, knobTop }, { knobWidth, knobHeight }, { color });
    }

    void trash(Point boxOrigin, Size boxSize, Color color)
    {
        Transform transform = computeTransform(boxOrigin, boxSize, 100.0f, 100.0f);

        int leftX = transform.baseX;
        int topY = transform.baseY;
        int pixelSize = std::min(transform.pixelWidth, transform.pixelHeight);

        int margin = 0.2 * pixelSize;
        int rectLeft = leftX + margin;
        int rectTop = topY + 1;
        int rectWidth = std::max(1, pixelSize - margin * 2);
        int rectHeight = std::max(1, pixelSize - 1);

        draw.rect({ rectLeft, rectTop }, { rectWidth, rectHeight }, { color });

        draw.line({ leftX, rectTop }, { leftX + pixelSize, rectTop }, { color });

        draw.line({ leftX + static_cast<int>(pixelSize * 0.3f), topY },
            { leftX + static_cast<int>(pixelSize * 0.7f), topY },
            { color });
    }
};
