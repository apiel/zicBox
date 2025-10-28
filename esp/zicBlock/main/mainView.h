#pragma once

#include "audio.h"
#include "view.h"

#include "helpers/format.h"

#include <string>

class MainView : public View {
protected:
    Audio& audio = Audio::get();

    int valueCenter;

    int intValue = 70;
    float floatValue = 0.70f;

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
        renderBar(valuePos[0], audio.duration / 3000.0f);
        renderStringValue(valuePos[0], "Duration", std::to_string(audio.duration) + "ms");

        renderBar(valuePos[1], audio.envelopAmp.getMorph());
        renderStringValue(valuePos[1], "Amp", fToString(audio.envelopAmp.getMorph() * 100, 2) + "%");

        renderBar(valuePos[2], audio.envelopFreq.getMorph());
        renderStringValue(valuePos[2], "Freq", fToString(audio.envelopFreq.getMorph() * 100, 2) + "%");

        renderCenteredBar(valuePos[3], audio.pitch / 36.0f);
        renderStringValue(valuePos[3], "Pitch", std::to_string(audio.pitch));

        renderBar(valuePos[4], ((float)audio.waveform.getType() * 100.0f + audio.waveform.getMorph() * 100) / ((float)WavetableGenerator::Type::COUNT * 100.0f));
        renderStringValue(valuePos[4], audio.waveform.getTypeName(), std::to_string((int)(audio.waveform.getMorph() * 100)) + "%");

        renderBar(valuePos[5], audio.resonator / 1.5f);
        renderStringValue(valuePos[5], "Resonate", fToString(audio.resonator, 2));

        renderBar(valuePos[6], audio.timbre);
        renderStringValue(valuePos[6], "Timbre", std::to_string((int)(audio.timbre * 100)) + "%");

        renderBar(valuePos[7], intValue / 100.0f);
        renderStringValue(valuePos[7], "Val", std::to_string(intValue) + "%");

        renderBar(valuePos[8], intValue / 100.0f);
        renderStringValue(valuePos[8], "Val", std::to_string(intValue) + "%");
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
        } else if (id == 5) {
            float morph = audio.waveform.getMorph();
            int type = (int)audio.waveform.getType();
            morph = morph + direction * 0.01f;
            if (morph > 1.0f && type < (int)WavetableGenerator::Type::COUNT - 1) {
                morph = 0.0f;
                type++;
                audio.waveform.setType((WavetableGenerator::Type)type);
            } else if (morph < 0.0f && type > 0) {
                morph = 1.0f;
                type--;
                audio.waveform.setType((WavetableGenerator::Type)type);
            }
            audio.waveform.setMorph(morph);
        } else if (id == 6) {
            audio.resonator = CLAMP(audio.resonator + direction * 0.01f, 0.0f, 1.5f);
        } else if (id == 7) {
            audio.timbre = CLAMP(audio.timbre + direction * 0.01f, 0.0f, 1.0f);
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