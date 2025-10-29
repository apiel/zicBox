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

        renderCenteredBar(valuePos[1], audio.stringPitch / 36.0f);
        renderStringValue(valuePos[1], "Pitch", std::to_string(audio.stringPitch));

        renderBar(valuePos[2], audio.damping);
        renderStringValue(valuePos[2], "Damping", std::to_string((int)(audio.damping * 100)) + "%");

        float decayPct = (audio.stringDecay - 0.80f) / 0.19f;
        renderBar(valuePos[3], decayPct);
        renderStringValue(valuePos[3], "Decay", std::to_string((int)(decayPct * 100)) + "%");

        renderBar(valuePos[4], audio.stringToneLevel);
        renderStringValue(valuePos[4], "Tone", std::to_string((int)(audio.stringToneLevel * 100)) + "%");

        renderBar(valuePos[5], audio.stringPluckNoise);
        renderStringValue(valuePos[5], "Noise", std::to_string((int)(audio.stringPluckNoise * 100)) + "%");

        renderBar(valuePos[6], audio.stringRingMod);
        renderStringValue(valuePos[6], "RingMod", std::to_string((int)(audio.stringRingMod * 100)) + "%");

        // renderBar(valuePos[7], audio.stringTremolo);
        // renderStringValue(valuePos[7], "Tremlo", std::to_string((int)(audio.stringTremolo * 100)) + "%");

        // renderBar(valuePos[8], audio.stringDecimator);
        // renderStringValue(valuePos[8], "Decim.", std::to_string((int)(audio.stringDecimator * 100)) + "%");

        renderBar(valuePos[7], audio.stringLfoDepth);
        renderStringValue(valuePos[7], "LFO", std::to_string((int)(audio.stringLfoDepth * 100)) + "%");

        renderBar(valuePos[8], audio.stringReverb);
        renderStringValue(valuePos[8], "Reverb", std::to_string((int)(audio.stringReverb * 100)) + "%");
    }

    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        if (id == 1) {
            audio.stringVolume = CLAMP(audio.stringVolume + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 2) {
            audio.stringPitch = CLAMP(audio.stringPitch + direction, -36, 36);
        } else if (id == 3) {
            audio.damping = CLAMP(audio.damping + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 4) {
            audio.stringDecay = CLAMP(audio.stringDecay + direction * 0.005f, 0.8f, 0.99f);
        } else if (id == 5) {
            audio.stringToneLevel = CLAMP(audio.stringToneLevel + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 6) {
            audio.stringPluckNoise = CLAMP(audio.stringPluckNoise + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 7) {
            audio.stringRingMod = CLAMP(audio.stringRingMod + direction * 0.01f, 0.0f, 1.0f);
        // } else if (id == 8) {
        //     audio.stringTremolo = CLAMP(audio.stringTremolo + direction * 0.01f, 0.0f, 1.0f);
        // } else if (id == 9) {
        //     audio.stringDecimator = CLAMP(audio.stringDecimator + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 8) {
            audio.stringLfoDepth = CLAMP(audio.stringLfoDepth + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 9) {
            audio.stringReverb = CLAMP(audio.stringReverb + direction * 0.01f, 0.0f, 1.0f);
        }
        draw.renderNext();
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};