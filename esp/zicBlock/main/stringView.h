#pragma once

#include "audio.h"
#include "valueView.h"

#include "helpers/format.h"

#include <string>

class StringView : public ValueView {
public:
    StringView(DrawInterface& draw)
        : ValueView(draw)
    {
    }

    void render() override
    {
        draw.clear();

        renderBar(valuePos[0], audio.stringVolume);
        renderStringValue(valuePos[0], "Amount", std::to_string((int)(audio.stringVolume * 100)) + "%");

        renderCenteredBar(valuePos[1], audio.drumString.pitch / 36.0f);
        renderStringValue(valuePos[1], "Pitch", std::to_string(audio.drumString.pitch));

        renderBar(valuePos[2], audio.drumString.damping);
        renderStringValue(valuePos[2], "Damping", std::to_string((int)(audio.drumString.damping * 100)) + "%");

        float decayPct = (audio.drumString.decay - 0.80f) / 0.19f;
        renderBar(valuePos[3], decayPct);
        renderStringValue(valuePos[3], "Decay", std::to_string((int)(decayPct * 100)) + "%");

        renderBar(valuePos[4], audio.drumString.toneLevel);
        renderStringValue(valuePos[4], "Tone", std::to_string((int)(audio.drumString.toneLevel * 100)) + "%");

        renderBar(valuePos[5], audio.drumString.pluckNoise);
        renderStringValue(valuePos[5], "Noise", std::to_string((int)(audio.drumString.pluckNoise * 100)) + "%");

        renderBar(valuePos[6], audio.drumString.ringMod);
        renderStringValue(valuePos[6], "RingMod", std::to_string((int)(audio.drumString.ringMod * 100)) + "%");

        renderBar(valuePos[7], audio.drumString.lfoDepth);
        renderStringValue(valuePos[7], "LFO", std::to_string((int)(audio.drumString.lfoDepth * 100)) + "%");

        renderBar(valuePos[8], audio.drumString.reverb);
        renderStringValue(valuePos[8], "Reverb", std::to_string((int)(audio.drumString.reverb * 100)) + "%");
    }

    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        if (id == 1) {
            audio.stringVolume = CLAMP(audio.stringVolume + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 2) {
            audio.drumString.setPitch(audio.drumString.pitch + direction);
        } else if (id == 3) {
            audio.drumString.setDamping(audio.drumString.damping + direction * 0.01f);
        } else if (id == 4) {
            audio.drumString.setDecay(audio.drumString.decay + direction * 0.005f);
        } else if (id == 5) {
            audio.drumString.setToneLevel(audio.drumString.toneLevel + direction * 0.01f);
        } else if (id == 6) {
            audio.drumString.setPluckNoise(audio.drumString.pluckNoise + direction * 0.01f);
        } else if (id == 7) {
            audio.drumString.setRingMod(audio.drumString.ringMod + direction * 0.01f);
        } else if (id == 8) {
            audio.drumString.setLfoDepth(audio.drumString.lfoDepth + direction * 0.01f);
        } else if (id == 9) {
            audio.drumString.setReverb(audio.drumString.reverb + direction * 0.01f);
        }
        draw.renderNext();
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};