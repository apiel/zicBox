#pragma once

#include "audio.h"
#include "view.h"

#include "helpers/format.h"

#include <string>

class ValueView : public View {
protected:
    Audio& audio = Audio::get();

    int valueCenter;

    void renderBar(Point pos, float value)
    {
        int w = (valueSize.w - 2);
        int wVal = w * value;
        draw.line({ pos.x + 1, pos.y + 0 }, { pos.x + 1 + w, pos.y + 0 });
        draw.line({ pos.x + 1, pos.y + 1 }, { pos.x + 1 + wVal, pos.y + 1 });
        draw.line({ pos.x + 1, pos.y + 2 }, { pos.x + 1 + wVal, pos.y + 2 });
    }

    void renderCenteredBar(Point pos, float value)
    {
        int w = (valueSize.w - 2);
        int wVal = w * value * 0.5f;
        draw.line({ pos.x + 1, pos.y + 0 }, { pos.x + 1 + w, pos.y + 0 });
        draw.line({ pos.x + valueCenter, pos.y + 1 }, { pos.x + valueCenter + wVal, pos.y + 1 });
        draw.line({ pos.x + valueCenter, pos.y + 2 }, { pos.x + valueCenter + wVal, pos.y + 2 });
    }

    void renderStringValue(Point pos, std::string name, std::string value)
    {
        renderTextCentered(pos, name, 4);
        renderTextCentered(pos, value, 14);
    }

    void renderTextCentered(Point pos, std::string value, uint8_t marginTop = 0)
    {
        draw.textCentered({ pos.x + valueCenter, pos.y + marginTop }, value, { .maxWidth = valueSize.w });
    }

    void renderTitle(std::string title)
    {
        draw.text({ 4, 8 }, title, { .scale = 2 });
    }

public:
    int valueTop = 36;
    Size valueSize;
    Point valuePos[9];

    ValueView(DrawInterface& draw)
        : View(draw)
    {
        const Size& size = draw.getSize();
        valueSize = { size.w / 3, (size.h - valueTop) / 3 };
        valueCenter = valueSize.w / 2;
        valuePos[0] = { 0, valueTop };
        valuePos[1] = { valueSize.w, valueTop };
        valuePos[2] = { valueSize.w * 2, valueTop };
        valuePos[3] = { 0, valueSize.h + valueTop };
        valuePos[4] = { valueSize.w, valueSize.h + valueTop };
        valuePos[5] = { valueSize.w * 2, valueSize.h + valueTop };
        valuePos[6] = { 0, valueSize.h * 2 + valueTop };
        valuePos[7] = { valueSize.w, valueSize.h * 2 + valueTop };
        valuePos[8] = { valueSize.w * 2, valueSize.h * 2 + valueTop };
    }
};