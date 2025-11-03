#pragma once

#include "audio.h"
#include "valueView.h"

#include "helpers/format.h"

#include <string>

class SnareHatView : public ValueView {
public:
    SnareHatView(DrawInterface& draw)
        : ValueView(draw)
    {
    }

    void render() override
    {
        draw.clear();

        renderBar(valuePos[0], audio.volume);
        renderStringValue(valuePos[0], "Volume", std::to_string((int)(audio.volume * 100)) + "%");

        renderBar(valuePos[1], audio.snareHat.decay);
        renderStringValue(valuePos[1], "Decay", std::to_string((int)(audio.snareHat.decay * 100)) + "%");

        renderBar(valuePos[2], audio.snareHat.type);
        renderStringValue(valuePos[2], "Type", std::to_string((int)(audio.snareHat.type * 100)) + "%");

        renderBar(valuePos[3], (audio.snareHat.toneFreq - 40.0f) / 4000.0f);
        renderStringValue(valuePos[3], "Freq", std::to_string((int)(audio.snareHat.toneFreq)));

        renderBar(valuePos[4], audio.snareHat.mix);
        renderStringValue(valuePos[4], "Noise", std::to_string((int)(audio.snareHat.mix * 100)) + "%");

        renderBar(valuePos[5], audio.snareHat.toneTimbre);
        renderStringValue(valuePos[5], "Timbre", std::to_string((int)(audio.snareHat.toneTimbre * 100)) + "%");

        renderBar(valuePos[6], audio.snareHat.toneFM * 10); // Because value is 0.0 bis 0.1
        renderStringValue(valuePos[6], "Fm", fToString(audio.snareHat.toneFM * 1000, 1) + "%");

        renderBar(valuePos[7], audio.snareHat.filter);
        renderStringValue(valuePos[7], "Filter", std::to_string((int)(audio.snareHat.filter * 100)) + "%");

        renderBar(valuePos[8], audio.snareHat.resonance);
        renderStringValue(valuePos[8], "Reso.", std::to_string((int)(audio.snareHat.resonance * 100)) + "%");
    }

    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        if (id == 1) {
            audio.volume = CLAMP(audio.volume + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 2) {
            audio.snareHat.setDecay(audio.snareHat.decay + direction * 0.01f);
        } else if (id == 3) {
            audio.snareHat.setType(audio.snareHat.type + direction * 0.01f);
        } else if (id == 4) {
            audio.snareHat.setToneFreq(audio.snareHat.toneFreq + direction * (audio.snareHat.toneFreq > 400.0f ? 10.0f : 1.0f ));
        } else if (id == 5) {
            audio.snareHat.setMix(audio.snareHat.mix + direction * 0.01f);
        } else if (id == 6) {
            audio.snareHat.setToneTimbre(audio.snareHat.toneTimbre + direction * 0.01f);
        } else if (id == 7) {
            audio.snareHat.setToneFM(audio.snareHat.toneFM + direction * 0.0001f);
        } else if (id == 8) {
            audio.snareHat.setFilter(audio.snareHat.filter + direction * 0.01f);
        } else if (id == 9) {
            audio.snareHat.setResonance(audio.snareHat.resonance + direction * 0.01f);
        }
        draw.renderNext();
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};