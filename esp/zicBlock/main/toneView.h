#pragma once

#include "audio.h"
#include "valueView.h"

#include "helpers/format.h"

#include <string>

class ToneView : public ValueView {
public:
    ToneView(DrawInterface& draw)
        : ValueView(draw)
    {
    }

    void render() override
    {
        draw.clear();
        renderBar(valuePos[0], audio.toneVolume);
        renderStringValue(valuePos[0], "Amount", std::to_string((int)(audio.toneVolume * 100)) + "%");

        renderBar(valuePos[1], audio.duration / 3000.0f);
        renderStringValue(valuePos[1], "Duration", std::to_string(audio.duration) + "ms");

        renderBar(valuePos[2], audio.envelopAmp.getMorph());
        renderStringValue(valuePos[2], "Amp", fToString(audio.envelopAmp.getMorph() * 100, 2) + "%");

        renderBar(valuePos[3], audio.envelopFreq.getMorph());
        renderStringValue(valuePos[3], "Bend", fToString(audio.envelopFreq.getMorph() * 100, 2) + "%");

        renderCenteredBar(valuePos[4], audio.pitch / 36.0f);
        renderStringValue(valuePos[4], "Pitch", std::to_string(audio.pitch));

        renderBar(valuePos[5], ((float)audio.waveform.getType() * 100.0f + audio.waveform.getMorph() * 100) / ((float)WavetableGenerator::Type::COUNT * 100.0f));
        renderStringValue(valuePos[5], audio.waveform.getTypeName(), std::to_string((int)(audio.waveform.getMorph() * 100)) + "%");

        renderBar(valuePos[6], audio.resonator / 1.5f);
        renderStringValue(valuePos[6], "Resonate", fToString(audio.resonator, 2));

        renderBar(valuePos[7], audio.timbre);
        renderStringValue(valuePos[7], "Timbre", std::to_string((int)(audio.timbre * 100)) + "%");

        renderBar(valuePos[8], audio.transient.getMorph());
        renderStringValue(valuePos[8], "Transient", fToString(audio.transient.getMorph() * 100, 2) + "%");
    }

    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        if (id == 1) {
            audio.toneVolume = CLAMP(audio.toneVolume + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 2) {
            audio.duration = CLAMP(audio.duration + direction * 10, 0, 3000);
        } else if (id == 3) {
            audio.envelopAmp.morph(audio.envelopAmp.getMorph() + direction * 0.01f);
        } else if (id == 4) {
            audio.envelopFreq.setMorph(audio.envelopFreq.getMorph() + direction * 0.0005f);
        } else if (id == 5) {
            audio.pitch = CLAMP(audio.pitch + direction, -36, 36);
        } else if (id == 6) {
            float morph = audio.waveform.getMorph();
            int type = (int)audio.waveform.getType();
            morph = morph + direction * 0.01f;
            if (morph > 1.0f && type < (int)WavetableGenerator::Type::COUNT - 1) {
                morph = 0.0f;
                type++;
                audio.waveform.setType((WavetableGenerator::Type)type);
            } else if (morph < 0.0f && type > 0) {
                morph = 1.0f;
                type--;
                audio.waveform.setType((WavetableGenerator::Type)type);
            }
            audio.waveform.setMorph(morph);
        } else if (id == 7) {
            audio.resonator = CLAMP(audio.resonator + direction * 0.01f, 0.0f, 1.5f);
        } else if (id == 8) {
            audio.timbre = CLAMP(audio.timbre + direction * 0.01f, 0.0f, 1.0f);
        } else {
            audio.transient.setMorph(audio.transient.getMorph() + direction * 0.0005f);
        }
        draw.renderNext();
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};