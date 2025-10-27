#pragma once

#include "audio.h"
#include "view.h"

#include "helpers/format.h"

class MainView : public View {
protected:
    Audio& audio = Audio::get();

    int valueCenter;

    int intValue = 70;
    float floatValue = 0.70f;

    void renderValue(Point pos, const std::string& name, std::function<std::string()> getValue, std::function<float()> getFloatValue)
    {
        int w = (valueSize.w - 2);
        int wVal = w * getFloatValue();
        draw.line({ pos.x + 1, pos.y + 0 }, { pos.x + 1 + w, pos.y + 0 }, true);
        draw.line({ pos.x + 1, pos.y + 1 }, { pos.x + 1 + wVal, pos.y + 1 }, true);
        draw.line({ pos.x + 1, pos.y + 2 }, { pos.x + 1 + wVal, pos.y + 2 }, true);

        renderValue(pos, name, getValue);
    }

    void renderCenteredValue(Point pos, const std::string& name, std::function<std::string()> getValue, std::function<float()> getFloatValue)
    {
        int w = (valueSize.w - 2);
        int wVal = w * getFloatValue() * 0.5f;
        draw.line({ pos.x + 1, pos.y + 0 }, { pos.x + 1 + w, pos.y + 0 }, true);
        draw.line({ valueCenter, pos.y + 1 }, { valueCenter + wVal, pos.y + 1 }, true);
        draw.line({ valueCenter, pos.y + 2 }, { valueCenter + wVal, pos.y + 2 }, true);

        renderValue(pos, name, getValue);
    }

    void renderValue(Point pos, const std::string& name, std::function<std::string()> getValue)
    {
        draw.textCentered({ pos.x + valueCenter, pos.y + 4 }, name, { .maxWidth = valueSize.w });
        draw.textCentered({ pos.x + valueCenter, pos.y + 14 }, getValue(), { .maxWidth = valueSize.w });
    }

public:
    int valueTop = 36;
    Size valueSize;
    Point valuePos[9];

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

    // std::to_string(intValue) + "%"
    void render() override
    {
        draw.clear();
        renderValue(valuePos[0], "Duration", [&]() { return std::to_string(audio.duration) + "ms"; }, [&]() { return audio.duration / 3000.0f; });
        renderValue(valuePos[1], "Amp", [&]() { return std::to_string((int)(audio.envelopAmp.getMorph() * 100)) + "%"; }, [&]() { return audio.envelopAmp.getMorph(); });
        renderValue(valuePos[2], "Freq", [&]() { return fToString(audio.envelopFreq.getMorph() * 100, 2) + "%"; }, [&]() { return audio.envelopFreq.getMorph(); });
        renderCenteredValue(valuePos[3], "Pitch", [&]() { return std::to_string(audio.pitch); }, [&]() { return audio.pitch / 36.0f; });
        renderValue(valuePos[4], "Wave", [&]() { return std::to_string(intValue) + "%"; }, [&]() { return intValue / 100.0f; });
        renderValue(valuePos[5], "Val", [&]() { return std::to_string(intValue) + "%"; }, [&]() { return intValue / 100.0f; });
        renderValue(valuePos[6], "Val", [&]() { return std::to_string(intValue) + "%"; }, [&]() { return intValue / 100.0f; });
        renderValue(valuePos[7], "Val", [&]() { return std::to_string(intValue) + "%"; }, [&]() { return intValue / 100.0f; });
        renderValue(valuePos[8], "Val", [&]() { return std::to_string(intValue) + "%"; }, [&]() { return intValue / 100.0f; });
    }

    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        if (id == 1) {
            audio.duration = CLAMP(audio.duration + direction * 10, 0, 3000);
        } else if (id == 2) {
            audio.envelopAmp.morph(audio.envelopAmp.getMorph() + direction * 0.01f);
        } else if (id == 3) {
            audio.envelopFreq.setMorph(audio.envelopFreq.getMorph() + direction * 0.0005f);
        } else if (id == 4) {
            audio.pitch = CLAMP(audio.pitch + direction, -36, 36);
        } else {
            intValue = CLAMP(intValue + direction, 0, 100);
            floatValue = intValue / 100.0f;
        }
        draw.renderNext();
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};