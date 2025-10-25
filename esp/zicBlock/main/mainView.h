#pragma once

#include "view.h"

class MainView : public View {
protected:
    int valueTop = 36;
    Size valueSize;
    Point valuePos[9];
    int valueCenter;

    int intValue = 80;
    float floatValue = 0.80f;

    void renderValue(Point pos)
    {
        int w = (valueSize.w - 2);
        int wVal = w * floatValue;
        draw.line({ pos.x + 1, pos.y + 0 }, { pos.x + 1 + w, pos.y + 0 }, true);
        draw.line({ pos.x + 1, pos.y + 1 }, { pos.x + 1 + wVal, pos.y + 1 }, true);
        draw.line({ pos.x + 1, pos.y + 2 }, { pos.x + 1 + wVal, pos.y + 2 }, true);

        draw.textCentered({ pos.x + valueCenter, pos.y + 4}, "Value", { .maxWidth = valueSize.w });
        draw.textCentered({ pos.x + valueCenter, pos.y + 14}, std::to_string(intValue) + "%", { .maxWidth = valueSize.w });
    }

public:
    MainView(DrawInterface& draw)
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

    void render() override
    {
        draw.clear();
        renderValue(valuePos[0]);
        renderValue(valuePos[1]);
        renderValue(valuePos[2]);
        renderValue(valuePos[3]);
        renderValue(valuePos[4]);
        renderValue(valuePos[5]);
        renderValue(valuePos[6]);
        renderValue(valuePos[7]);
        renderValue(valuePos[8]);
    }
    void onEncoder(int id, int8_t direction, uint32_t tick) override
    {
        intValue = CLAMP(intValue + direction, 0, 100);
        floatValue = intValue / 100.0f;
        draw.renderNext();
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};