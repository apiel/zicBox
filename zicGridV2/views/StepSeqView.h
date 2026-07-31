#pragma once

#include "draw/draw.h"
#include "zicGridV2/ViewManager.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/uiComponents.h"

class StepSeqView : public View {
private:
    int stepPage = 0;

public:
    StepSeqView() : View("STEP SEQUENCER") {}

    void onActivate() override
    {
        updatePadLeds();
        updateEncoderLabels();
    }

    void updatePadLeds() override
    {
        int trkIdx = studio.selTrack;
        if (trkIdx < 0 || trkIdx >= MAX_TRACKS) return;

        auto& track = studio.tracks[trkIdx];
        uint32_t currentStep = studio.currentStep % SEQ_STEPS;

        for (int r = 0; r < PAD_ROWS; ++r) {
            for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
                int stepIdx = (stepPage * 32) + (r * DYNAMIC_PAD_COLS + c);
                auto& pad = gridState.pads[c][r];

                if (stepIdx < (int)track->sequence.size()) {
                    bool isActive = track->sequence[stepIdx].active;
                    pad.active = isActive;
                    pad.label = std::to_string(stepIdx + 1);

                    if (stepIdx == (int)currentStep) {
                        pad.color = { 255, 255, 255, 255 };
                    } else if (isActive) {
                        pad.color = track->themeColor;
                    } else {
                        pad.color = { 35, 45, 60, 255 };
                    }
                }
            }
        }
    }

    void updateEncoderLabels() override
    {
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];

        gridState.encoders[0] = { "BPM", studio.bpm, 20.0f, 300.0f, 1.0f, std::to_string((int)studio.bpm), { 255, 180, 0, 255 } };
        gridState.encoders[1] = { "Track", (float)(trk + 1), 1.0f, 8.0f, 1.0f, "T" + std::to_string(trk + 1), t->themeColor };
        gridState.encoders[2] = { "Page", (float)(stepPage + 1), 1.0f, 2.0f, 1.0f, "P" + std::to_string(stepPage + 1), { 180, 220, 255, 255 } };
        gridState.encoders[3] = { "Vol", t->volume, 0.0f, 1.0f, 0.05f, std::to_string((int)(t->volume * 100)) + "%", { 100, 255, 150, 255 } };

        int selStep = (studio.selStep >= 0) ? studio.selStep : 0;
        auto& step = t->sequence[selStep];

        gridState.encoders[4] = { "Step", (float)(selStep + 1), 1.0f, 64.0f, 1.0f, "S" + std::to_string(selStep + 1), { 255, 200, 100, 255 } };
        gridState.encoders[5] = { "Note", (float)step.note, 12.0f, 108.0f, 1.0f, std::to_string(step.note), { 200, 150, 255, 255 } };
        gridState.encoders[6] = { "Velo", step.velocity, 0.0f, 1.0f, 0.05f, std::to_string((int)(step.velocity * 100)), { 100, 200, 255, 255 } };
        gridState.encoders[7] = { "Prob", step.condition, 0.0f, 1.0f, 0.1f, std::to_string((int)(step.condition * 100)) + "%", { 255, 120, 180, 255 } };

        gridState.encoders[8] = { "Engine", (float)t->currentEngineIdx, 0.0f, (float)(ENGINE_REGISTRY_COUNT - 1), 1.0f, engineRegistry[t->currentEngineIdx].name, { 100, 220, 255, 255 } };
        gridState.encoders[9] = { "Gen", 0.0f, 0.0f, 1.0f, 1.0f, "Kick", { 255, 140, 60, 255 } };
        gridState.encoders[10] = { "Length", step.len, 0.25f, 16.0f, 0.25f, std::to_string(step.len).substr(0, 4), { 220, 220, 100, 255 } };
        gridState.encoders[11] = { "Master", studio.masterFx.volume, 0.0f, 1.0f, 0.05f, std::to_string((int)(studio.masterFx.volume * 100)) + "%", { 255, 80, 80, 255 } };
    }

    void render(Draw& d, int x, int y, int w, int h) override
    {
        std::string titleStr = "VIEW: STEP SEQUENCER - TRACK " + std::to_string(studio.selTrack + 1);
        d.text({ x + 6, y + 4 }, titleStr, 9, { .color = studio.tracks[studio.selTrack]->themeColor });
    }

    void handleDynamicPadPress(int col, int row, bool pressed) override
    {
        if (!pressed) return;

        int stepIdx = (stepPage * 32) + (row * DYNAMIC_PAD_COLS + col);
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];

        if (stepIdx < (int)t->sequence.size()) {
            t->sequence[stepIdx].active = !t->sequence[stepIdx].active;
            studio.selStep = stepIdx;

            if (t->sequence[stepIdx].active) {
                std::lock_guard<std::mutex> lock(studio.audioMutex);
                t->engine->noteOn(t->sequence[stepIdx].note, t->sequence[stepIdx].velocity);
                driftVisualizer.triggerKickPulse();
            }
        }
        updatePadLeds();
        updateEncoderLabels();
    }

    void handleEncoder(int encoderId, int delta) override
    {
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];
        int selStep = (studio.selStep >= 0) ? studio.selStep : 0;
        auto& step = t->sequence[selStep];

        switch (encoderId) {
        case 1:
            studio.updateBpm(studio.bpm + delta);
            break;
        case 2:
            studio.selTrack = std::clamp(studio.selTrack + delta, 0, MAX_TRACKS - 1);
            gridState.utility.activeTrack = studio.selTrack;
            break;
        case 3:
            stepPage = std::clamp(stepPage + delta, 0, 1);
            break;
        case 4:
            t->volume = std::clamp(t->volume + delta * 0.05f, 0.0f, 1.0f);
            break;
        case 5:
            studio.selStep = std::clamp(selStep + delta, 0, SEQ_STEPS - 1);
            break;
        case 6:
            step.note = std::clamp(step.note + delta, 12, 108);
            break;
        case 7:
            step.velocity = std::clamp(step.velocity + delta * 0.05f, 0.0f, 1.0f);
            break;
        case 8:
            step.condition = std::clamp(step.condition + delta * 0.05f, 0.0f, 1.0f);
            break;
        case 9:
        {
            int nextEngine = std::clamp((int)t->currentEngineIdx + delta, 0, ENGINE_REGISTRY_COUNT - 1);
            if (nextEngine != t->currentEngineIdx) {
                std::lock_guard<std::mutex> lock(studio.audioMutex);
                t->setEngine(nextEngine);
            }
            break;
        }
        case 10:
            if (delta != 0 && t->generate) {
                t->generate(t->sequence, 0.5f, 0.5f, 0.5f);
            }
            break;
        case 11:
            step.len = std::clamp(step.len + delta * 0.25f, 0.25f, 16.0f);
            break;
        case 12:
            studio.masterFx.volume = std::clamp(studio.masterFx.volume + delta * 0.05f, 0.0f, 1.0f);
            break;
        }

        updatePadLeds();
    }
};
