#pragma once

#include "audio.h"
#include "valueView.h"

#include "helpers/format.h"

#include <string>

class ClapView : public ValueView {
protected:
    int intValue = 70;
    float floatValue = 0.70f;

public:
    ClapView(DrawInterface& draw)
        : ValueView(draw)
    {
    }

    void render() override
    {
        draw.clear();

        renderBar(valuePos[0], audio.clapVolume);
        renderStringValue(valuePos[0], "Amount", std::to_string((int)(audio.clapVolume * 100)) + "%");

        renderBar(valuePos[1], audio.clapEnvelopAmp.getMorph());
        renderStringValue(valuePos[1], "Amp", fToString(audio.clapEnvelopAmp.getMorph() * 100, 2) + "%");

        renderBar(valuePos[2], audio.burstSpacing);
        renderStringValue(valuePos[2], "Spacing", std::to_string((int)(audio.burstSpacing * 100)) + "%");

        renderBar(valuePos[3], audio.clapDecay);
        renderStringValue(valuePos[3], "Decay", std::to_string((int)(audio.clapDecay * 100)) + "%");

        renderBar(valuePos[4], (audio.burstCount - 1) / 9.0f);
        renderStringValue(valuePos[4], "Count", std::to_string(audio.burstCount));

        renderBar(valuePos[5], audio.clapNoiseColor);
        renderStringValue(valuePos[5], "Color", std::to_string((int)(audio.clapNoiseColor * 100)) + "%");

        renderCenteredBar(valuePos[6], audio.clapPunch);
        renderStringValue(valuePos[6], "Punch", std::to_string((int)(abs(audio.clapPunch) * 100)) + "%");

    }

    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        if (id == 1) {
            audio.clapVolume = CLAMP(audio.clapVolume + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 2) {
            audio.clapEnvelopAmp.morph(audio.clapEnvelopAmp.getMorph() + direction * 0.01f);
        } else if (id == 3) {
            audio.burstSpacing = CLAMP(audio.burstSpacing + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 4) {
            audio.clapDecay = CLAMP(audio.clapDecay + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 5) {
            audio.burstCount = CLAMP(audio.burstCount + direction, 1, 10);
        } else if (id == 6) {
            audio.clapNoiseColor = CLAMP(audio.clapNoiseColor + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 7) {
            audio.clapPunch = CLAMP(audio.clapPunch + direction * 0.01f, -1.0f, 1.0f);
        }
        draw.renderNext();
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};