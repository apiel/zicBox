#pragma once

#include "audio.h"
#include "valueView.h"

#include "helpers/format.h"

#include <string>

class FxView : public ValueView {
protected:
    int intValue = 70;
    float floatValue = 0.70f;

public:
    FxView(DrawInterface& draw)
        : ValueView(draw)
    {
    }

    void render() override
    {
        draw.clear();

        renderBar(valuePos[0], audio.cutoff);
        renderStringValue(valuePos[0], "Filter", std::to_string((int)(audio.cutoff * 100)) + "%");

        renderBar(valuePos[1], audio.getResonance());
        renderStringValue(valuePos[1], "Reso.", std::to_string((int)(audio.getResonance() * 100)) + "%");

        // renderBar(valuePos[2], audio.burstSpacing);
        // renderStringValue(valuePos[2], "Spacing", std::to_string((int)(audio.burstSpacing * 100)) + "%");

        // renderBar(valuePos[3], audio.clapDecay);
        // renderStringValue(valuePos[3], "Decay", std::to_string((int)(audio.clapDecay * 100)) + "%");

        // renderBar(valuePos[4], (audio.burstCount - 1) / 9.0f);
        // renderStringValue(valuePos[4], "Count", std::to_string(audio.burstCount));

        // renderBar(valuePos[5], audio.clapNoiseColor);
        // renderStringValue(valuePos[5], "Color", std::to_string((int)(audio.clapNoiseColor * 100)) + "%");

        // renderCenteredBar(valuePos[6], audio.clapPunch);
        // renderStringValue(valuePos[6], "Punch", std::to_string((int)(abs(audio.clapPunch) * 100)) + "%");

        // renderBar(valuePos[7], audio.clapFilter);
        // renderStringValue(valuePos[7], "Filter", std::to_string((int)(audio.clapFilter * 100)) + "%");

        // renderBar(valuePos[8], audio.clapResonance);
        // renderStringValue(valuePos[8], "Reso.", std::to_string((int)(audio.clapResonance * 100)) + "%");
    }

    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        if (id == 1) {
            audio.cutoff = CLAMP(audio.cutoff + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 2) {
            audio.setResonance(CLAMP(audio.getResonance() + direction * 0.01f, 0.0f, 1.0f));
        } 
        draw.renderNext();
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};