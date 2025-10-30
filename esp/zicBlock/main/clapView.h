#pragma once

#include "audio.h"
#include "valueView.h"

#include "helpers/format.h"

#include <string>

class ClapView : public ValueView {
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

        renderBar(valuePos[1], audio.clap.envelopAmp.getMorph());
        renderStringValue(valuePos[1], "Amp", fToString(audio.clap.envelopAmp.getMorph() * 100, 2) + "%");

        renderBar(valuePos[2], audio.clap.burstSpacing);
        renderStringValue(valuePos[2], "Spacing", std::to_string((int)(audio.clap.burstSpacing * 100)) + "%");

        renderBar(valuePos[3], audio.clap.decay);
        renderStringValue(valuePos[3], "Decay", std::to_string((int)(audio.clap.decay * 100)) + "%");

        renderBar(valuePos[4], (audio.clap.burstCount - 1) / 9.0f);
        renderStringValue(valuePos[4], "Count", std::to_string(audio.clap.burstCount));

        renderBar(valuePos[5], audio.clap.noiseColor);
        renderStringValue(valuePos[5], "Color", std::to_string((int)(audio.clap.noiseColor * 100)) + "%");

        renderCenteredBar(valuePos[6], audio.clap.punch);
        renderStringValue(valuePos[6], "Punch", std::to_string((int)(fabs(audio.clap.punch) * 100)) + "%");

        renderBar(valuePos[7], audio.clap.filter);
        renderStringValue(valuePos[7], "Filter", std::to_string((int)(audio.clap.filter * 100)) + "%");

        renderBar(valuePos[8], audio.clap.resonance);
        renderStringValue(valuePos[8], "Reso.", std::to_string((int)(audio.clap.resonance * 100)) + "%");
    }

    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        if (id == 1) {
            audio.clapVolume = CLAMP(audio.clapVolume + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 2) {
            audio.clap.envelopAmp.morph(audio.clap.envelopAmp.getMorph() + direction * 0.01f);
        } else if (id == 3) {
            audio.clap.setBurstSpacing(audio.clap.burstSpacing + direction * 0.01f);
        } else if (id == 4) {
            audio.clap.setDecay(audio.clap.decay + direction * 0.01f);
        } else if (id == 5) {
            audio.clap.setBurstCount(audio.clap.burstCount + direction);
        } else if (id == 6) {
            audio.clap.setNoiseColor(audio.clap.noiseColor + direction * 0.01f);
        } else if (id == 7) {
            audio.clap.setPunch(audio.clap.punch + direction * 0.01f);
        } else if (id == 8) {
            audio.clap.setFilter(audio.clap.filter + direction * 0.01f);
        } else if (id == 9) {
            audio.clap.setResonance(audio.clap.resonance + direction * 0.01f);
        }
        draw.renderNext();
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};