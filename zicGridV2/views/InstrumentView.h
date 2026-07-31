#pragma once

#include "draw/draw.h"
#include "zicGridV2/ViewManager.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/uiComponents.h"

class InstrumentView : public View {
public:
    InstrumentView() : View("INSTRUMENT & SYNTH") {}

    void onActivate() override
    {
        updatePadLeds();
        updateEncoderLabels();
    }

    void updatePadLeds() override
    {
        for (int r = 0; r < PAD_ROWS; ++r) {
            for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
                int trkIdx = c;
                auto& pad = gridState.pads[c][r];
                auto& track = studio.tracks[trkIdx];

                pad.active = (studio.selTrack == trkIdx);
                pad.color = pad.active ? track->themeColor : Color{ (uint8_t)(track->themeColor.r / 3), (uint8_t)(track->themeColor.g / 3), (uint8_t)(track->themeColor.b / 3), 255 };

                if (r == 0) pad.label = "T" + std::to_string(trkIdx + 1);
                else if (r == 1) pad.label = engineRegistry[track->currentEngineIdx].name;
                else if (r == 2) pad.label = "TRIG";
                else if (r == 3) pad.label = track->isMuted ? "MUTED" : "ON";
            }
        }
    }

    void updateEncoderLabels() override
    {
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];

        if (t->engine) {
            size_t paramCount = t->engine->getParamCount();
            auto* params = t->engine->getParams();
            for (int i = 0; i < TOTAL_ENCODERS; ++i) {
                if ((size_t)i < paramCount && params) {
                    auto& p = params[i];
                    gridState.encoders[i].label = p.label ? p.label : "Param";
                    gridState.encoders[i].value = p.value;
                    gridState.encoders[i].minVal = p.min;
                    gridState.encoders[i].maxVal = p.max;
                    gridState.encoders[i].step = p.step;
                    gridState.encoders[i].displayVal = std::to_string(p.value).substr(0, 4);
                    gridState.encoders[i].color = t->themeColor;
                } else {
                    gridState.encoders[i] = { "---", 0.0f, 0.0f, 1.0f, 0.1f, "N/A", { 60, 70, 90, 255 } };
                }
            }
        }
    }

    void render(Draw& d, int x, int y, int w, int h) override
    {
        std::string titleStr = "VIEW: SYNTH & SAMPLER - " + std::string(engineRegistry[studio.tracks[studio.selTrack]->currentEngineIdx].name);
        d.text({ x + 6, y + 4 }, titleStr, 9, { .color = studio.tracks[studio.selTrack]->themeColor });
    }

    void handleDynamicPadPress(int col, int row, bool pressed) override
    {
        if (!pressed) return;

        int trk = col; // 0..7
        if (trk >= 0 && trk < MAX_TRACKS) {
            studio.selTrack = trk;
            gridState.utility.activeTrack = trk;
            auto& t = studio.tracks[trk];

            if (row == 1) { // Cycle engine
                int nextEngine = (t->currentEngineIdx + 1) % ENGINE_REGISTRY_COUNT;
                std::lock_guard<std::mutex> lock(studio.audioMutex);
                t->setEngine(nextEngine);
            } else if (row == 2) { // Trigger note
                std::lock_guard<std::mutex> lock(studio.audioMutex);
                t->engine->noteOn(60, 0.9f);
                driftVisualizer.triggerKickPulse();
            } else if (row == 3) { // Mute toggle
                t->isMuted = !t->isMuted;
            }
        }

        updatePadLeds();
        updateEncoderLabels();
    }

    void handleEncoder(int encoderId, int delta) override
    {
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];

        if (t->engine) {
            int paramIdx = encoderId - 1; // 0..11
            if (paramIdx >= 0 && (size_t)paramIdx < t->engine->getParamCount()) {
                auto& p = t->engine->getParams()[paramIdx];
                p.set(p.value + delta * p.step);
            }
        }

        updateEncoderLabels();
    }
};
