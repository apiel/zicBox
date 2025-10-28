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

        renderBar(valuePos[1], audio.burstSpacing);
        renderStringValue(valuePos[1], "Spacing", std::to_string((int)(audio.burstSpacing * 100)) + "%");

        renderBar(valuePos[2], audio.clapDecay);
        renderStringValue(valuePos[2], "Decay", std::to_string((int)(audio.clapDecay * 100)) + "%");

        renderBar(valuePos[3], (audio.burstCount - 1) / 9.0f);
        renderStringValue(valuePos[3], "Count", std::to_string(audio.burstCount));

        renderBar(valuePos[4], audio.clapNoiseColor);
        renderStringValue(valuePos[4], "Color", std::to_string((int)(audio.clapNoiseColor * 100)) + "%");

        renderCenteredBar(valuePos[5], audio.clapPunch);
        renderStringValue(valuePos[5], "Punch", std::to_string((int)(abs(audio.clapPunch) * 100)) + "%");

        // renderBar(valuePos[6], audio.resonator / 1.5f);
        // renderStringValue(valuePos[6], "Resonate", fToString(audio.resonator, 2));

        // renderBar(valuePos[7], audio.timbre);
        // renderStringValue(valuePos[7], "Timbre", std::to_string((int)(audio.timbre * 100)) + "%");

        // renderBar(valuePos[8], intValue / 100.0f);
        // renderStringValue(valuePos[8], "Value", std::to_string(intValue) + "%");
    }

    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        if (id == 1) {
            audio.clapVolume = CLAMP(audio.clapVolume + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 2) {
            audio.burstSpacing = CLAMP(audio.burstSpacing + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 3) {
            audio.clapDecay = CLAMP(audio.clapDecay + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 4) {
            audio.burstCount = CLAMP(audio.burstCount + direction, 1, 10);
        } else if (id == 5) {
            audio.clapNoiseColor = CLAMP(audio.clapNoiseColor + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 6) {
            audio.clapPunch = CLAMP(audio.clapPunch + direction * 0.01f, -1.0f, 1.0f);
        } else if (id == 7) {
            // audio.resonator = CLAMP(audio.resonator + direction * 0.01f, 0.0f, 1.5f);
        } else if (id == 8) {
            // audio.timbre = CLAMP(audio.timbre + direction * 0.01f, 0.0f, 1.0f);
        }
        draw.renderNext();
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};