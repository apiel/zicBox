#pragma once

#include "audio.h"
#include "valueView.h"

#include "helpers/format.h"

#include <string>

class FxView : public ValueView {
public:
    FxView(DrawInterface& draw)
        : ValueView(draw)
    {
    }

    void render() override
    {
        draw.clear();

        renderCenteredBar(valuePos[0], audio.toneEngine.filterCutoff);
        renderStringValue(valuePos[0], audio.toneEngine.filterCutoff < 0 ? "HPF" : "LPF", std::to_string((int)(fabs(audio.toneEngine.filterCutoff) * 100)) + "%");

        renderBar(valuePos[1], audio.toneEngine.getResonance());
        renderStringValue(valuePos[1], "Reso.", std::to_string((int)(audio.toneEngine.getResonance() * 100)) + "%");
    }

    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        if (id == 1) {
            audio.toneEngine.filterCutoff = CLAMP(audio.toneEngine.filterCutoff + direction * 0.01f, -1.0f, 1.0f);
        } else if (id == 2) {
            audio.toneEngine.setResonance(CLAMP(audio.toneEngine.getResonance() + direction * 0.01f, 0.0f, 1.0f));
        } 
        draw.renderNext();
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};