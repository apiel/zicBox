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

        renderCenteredBar(valuePos[0], audio.tone.filterCutoff);
        renderStringValue(valuePos[0], audio.tone.filterCutoff < 0 ? "HPF" : "LPF", std::to_string((int)(fabs(audio.tone.filterCutoff) * 100)) + "%");

        renderBar(valuePos[1], audio.tone.getResonance());
        renderStringValue(valuePos[1], "Reso.", std::to_string((int)(audio.tone.getResonance() * 100)) + "%");
    }

    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        if (id == 1) {
            audio.tone.setFilterCutoff(audio.tone.filterCutoff + direction * 0.01f);
        } else if (id == 2) {
            audio.tone.setResonance(audio.tone.getResonance() + direction * 0.01f);
        } 
        draw.renderNext();
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};