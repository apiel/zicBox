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
        draw.line({ pos.x + 1, pos.y + 0 }, { pos.x + 1 + w, pos.y + 0 }, true);
        draw.line({ pos.x + 1, pos.y + 1 }, { pos.x + 1 + wVal, pos.y + 1 }, true);
        draw.line({ pos.x + 1, pos.y + 2 }, { pos.x + 1 + wVal, pos.y + 2 }, true);
    }

    void renderCenteredBar(Point pos, float value)
    {
        int w = (valueSize.w - 2);
        int wVal = w * value * 0.5f;
        draw.line({ pos.x + 1, pos.y + 0 }, { pos.x + 1 + w, pos.y + 0 }, true);
        draw.line({ valueCenter, pos.y + 1 }, { valueCenter + wVal, pos.y + 1 }, true);
        draw.line({ valueCenter, pos.y + 2 }, { valueCenter + wVal, pos.y + 2 }, true);
    }

    void renderStringValue(Point pos, std::string name, std::string value)
    {
        draw.textCentered({ pos.x + valueCenter, pos.y + 4 }, name, { .maxWidth = valueSize.w });
        draw.textCentered({ pos.x + valueCenter, pos.y + 14 }, value, { .maxWidth = valueSize.w });
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