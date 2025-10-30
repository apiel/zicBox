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
        renderBar(valuePos[0], audio.volume);
        renderStringValue(valuePos[0], "Volume", std::to_string((int)(audio.volume * 100)) + "%");

        renderBar(valuePos[1], audio.tone.duration / 3000.0f);
        renderStringValue(valuePos[1], "Duration", std::to_string(audio.tone.duration) + "ms");

        renderBar(valuePos[2], audio.tone.envelopAmp.getMorph());
        renderStringValue(valuePos[2], "Amp", fToString(audio.tone.envelopAmp.getMorph() * 100, 2) + "%");

        renderBar(valuePos[3], audio.tone.envelopFreq.getMorph());
        renderStringValue(valuePos[3], "Bend", fToString(audio.tone.envelopFreq.getMorph() * 100, 2) + "%");

        renderCenteredBar(valuePos[4], audio.tone.pitch / 36.0f);
        renderStringValue(valuePos[4], "Pitch", std::to_string(audio.tone.pitch));

        renderBar(valuePos[5], ((float)audio.tone.waveform.getType() * 100.0f + audio.tone.waveform.getMorph() * 100) / ((float)WavetableGenerator::Type::COUNT * 100.0f));
        renderStringValue(valuePos[5], audio.tone.waveform.getTypeName(), std::to_string((int)(audio.tone.waveform.getMorph() * 100)) + "%");

        renderCenteredBar(valuePos[6], audio.tone.filterCutoff);
        renderStringValue(valuePos[6], audio.tone.filterCutoff < 0 ? "HPF" : "LPF", std::to_string((int)(fabs(audio.tone.filterCutoff) * 100)) + "%");

        renderBar(valuePos[7], audio.tone.getResonance());
        renderStringValue(valuePos[7], "Reso.", std::to_string((int)(audio.tone.getResonance() * 100)) + "%");

        renderBar(valuePos[8], audio.tone.transient.getMorph());
        renderStringValue(valuePos[8], "Transient", fToString(audio.tone.transient.getMorph() * 100, 2) + "%");
    }

    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        if (id == 1) {
            audio.volume = CLAMP(audio.volume + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 2) {
            audio.tone.setDuration(audio.tone.duration + direction * 10);
        } else if (id == 3) {
            audio.tone.envelopAmp.morph(audio.tone.envelopAmp.getMorph() + direction * 0.01f);
        } else if (id == 4) {
            audio.tone.envelopFreq.setMorph(audio.tone.envelopFreq.getMorph() + direction * 0.0005f);
        } else if (id == 5) {
            audio.tone.setPitch(audio.tone.pitch + direction);
        } else if (id == 6) {
            float morph = audio.tone.waveform.getMorph();
            int type = (int)audio.tone.waveform.getType();
            morph = morph + direction * 0.01f;
            if (morph > 1.0f && type < (int)WavetableGenerator::Type::COUNT - 1) {
                morph = 0.0f;
                type++;
                audio.tone.waveform.setType((WavetableGenerator::Type)type);
            } else if (morph < 0.0f && type > 0) {
                morph = 1.0f;
                type--;
                audio.tone.waveform.setType((WavetableGenerator::Type)type);
            }
            audio.tone.waveform.setMorph(morph);
        } else if (id == 7) {
            audio.tone.setFilterCutoff(audio.tone.filterCutoff + direction * 0.01f);
        } else if (id == 8) {
            audio.tone.setResonance(audio.tone.getResonance() + direction * 0.01f);
        } else {
            audio.tone.transient.setMorph(audio.tone.transient.getMorph() + direction * 0.0005f);
        }
        draw.renderNext();
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};