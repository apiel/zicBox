#pragma once

#include "audio.h"
#include "valueView.h"

#include "helpers/format.h"

#include <string>

class MetalicView : public ValueView {
public:
    MetalicView(DrawInterface& draw)
        : ValueView(draw)
    {
    }

    void render() override
    {
        draw.clear();

        renderTitle("METAL");

        renderBar(valuePos[0], audio.volume);
        renderStringValue(valuePos[0], "Volume", std::to_string((int)(audio.volume * 100)) + "%");

        renderBar(valuePos[1], audio.metalic.duration / 3000.0f);
        renderStringValue(valuePos[1], "Duration", std::to_string(audio.metalic.duration) + "ms");

        renderCenteredBar(valuePos[2], audio.metalic.pitch / 36.0f);
        renderStringValue(valuePos[2], "Pitch", std::to_string(audio.metalic.pitch));

        renderBar(valuePos[3], audio.metalic.fmAmplitude);
        renderStringValue(valuePos[3], "FM Amp", fToString(audio.metalic.fmAmplitude, 2));

        renderBar(valuePos[4], audio.metalic.fmFreq / 500.0f);
        renderStringValue(valuePos[4], "FM Freq", fToString(audio.metalic.fmFreq, 2));

        renderBar(valuePos[5], audio.metalic.envMod);
        renderStringValue(valuePos[5], "Env Mod", fToString(audio.metalic.envMod, 2));

        renderBar(valuePos[6], audio.metalic.resonator / 1.5f);
        renderStringValue(valuePos[6], "Resonate", fToString(audio.metalic.resonator, 2));

        renderBar(valuePos[7], audio.metalic.timbre);
        renderStringValue(valuePos[7], "Timbre", std::to_string((int)(audio.metalic.timbre * 100)) + "%");

        renderBar(valuePos[8], audio.metalic.reverb);
        renderStringValue(valuePos[8], "Reverb", std::to_string((int)(audio.metalic.reverb * 100)) + "%");
    }

    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        if (id == 1) {
            audio.volume = CLAMP(audio.volume + direction * 0.01f, 0.0f, 1.0f);
        } else if (id == 2) {
            audio.metalic.setDuration(audio.metalic.duration + direction * 10);
        } else if (id == 3) {
            audio.metalic.setPitch(audio.metalic.pitch + direction);
        } else if (id == 4) {
            audio.metalic.setFmAmplitude(audio.metalic.fmAmplitude + direction * 0.01f);
        } else if (id == 5) {
            audio.metalic.setFmFreq(audio.metalic.fmFreq + direction);
        } else if (id == 6) {
            audio.metalic.setEnvMod(audio.metalic.envMod + direction * 0.01f);
        } else if (id == 7) {
            audio.metalic.setResonator(audio.metalic.resonator + direction * 0.01f);
        } else if (id == 8) {
            audio.metalic.setTimbre(audio.metalic.timbre + direction * 0.01f);
        } else if (id == 9) {
            audio.metalic.setReverb(audio.metalic.reverb + direction * 0.01f);
        }
        draw.renderNext();
    }

    bool onKey(uint16_t id, int key, int8_t state) override
    {
        return false;
    }
};