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

        renderCenteredBar(valuePos[0], audio.filterCutoff);
        renderStringValue(valuePos[0], audio.filterCutoff < 0 ? "HPF" : "LPF", std::to_string((int)(fabs(audio.filterCutoff) * 100)) + "%");

        renderBar(valuePos[1], audio.getResonance());
        renderStringValue(valuePos[1], "Reso.", std::to_string((int)(audio.getResonance() * 100)) + "%");
    }

    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        if (id == 1) {
            audio.filterCutoff = CLAMP(audio.filterCutoff + direction * 0.01f, -1.0f, 1.0f);
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