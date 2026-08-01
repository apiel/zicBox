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

    std::pair<int, int> getViewPageInfo() const override
    {
        return { stepPage + 1, 2 };
    }

    void changePage(int delta) override
    {
        stepPage = (stepPage + delta + 2) % 2;
    }

    void onActivate() override
    {
        updatePadLeds();
        updateEncoderLabels();
    }

    void onDeactivate() override
    {
        // Restore standard utility row 3 pad labels when leaving StepSeqView
        gridState.pads[8][3].label = "&icon::arrowLeft::filled";
        gridState.pads[8][3].color = { 255, 160, 40, 255 };
        gridState.pads[9][3].label = "&icon::arrowRight::filled";
        gridState.pads[9][3].color = { 255, 160, 40, 255 };
        gridState.pads[10][3].label = "Oct-";
        gridState.pads[10][3].color = { 100, 120, 255, 255 };
        gridState.pads[11][3].label = "Oct+";
        gridState.pads[11][3].color = { 100, 120, 255, 255 };
    }

    void updatePadLeds() override
    {
        // Utility Row 3 pads on StepSeqView: Z, X, C are empty; V is Gen.
        gridState.pads[8][3].label = "";
        gridState.pads[9][3].label = "";
        gridState.pads[10][3].label = "";
        gridState.pads[11][3].label = "Gen.";
        gridState.pads[11][3].color = { 255, 160, 40, 255 };

        int trkIdx = studio.selTrack;
        if (trkIdx < 0 || trkIdx >= MAX_TRACKS) return;

        auto& track = studio.tracks[trkIdx];
        uint32_t currentStep = studio.currentStep % SEQ_STEPS;
        int selStep = (studio.selStep >= 0) ? studio.selStep : 0;

        for (int r = 0; r < PAD_ROWS; ++r) {
            for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
                int stepIdx = (stepPage * 32) + (r * DYNAMIC_PAD_COLS + c);
                auto& pad = gridState.pads[c][r];

                if (stepIdx < (int)track->sequence.size()) {
                    bool isActive = track->sequence[stepIdx].active;
                    bool isSelected = (stepIdx == selStep);
                    pad.active = isActive;
                    pad.pressed = isSelected;
                    pad.label = std::to_string(stepIdx + 1);

                    if (studio.isPlaying && stepIdx == (int)currentStep) {
                        pad.color = { 255, 255, 255, 255 };
                    } else if (isSelected) {
                        if (isActive) {
                            pad.color = { 255, 230, 100, 255 }; // Bright gold for active selected step
                        } else {
                            pad.color = { 100, 140, 180, 255 }; // Soft blue-grey for inactive selected step
                        }
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
        Color c = t->themeColor;
        Color genBg = { 35, 45, 65, 255 }; // Distinct background color for generator parameters

        // Encoders 0..3: Sequencer Timing & Global Track Params
        gridState.setEncoder(0, "BPM", studio.bpm, 20.0f, 300.0f, 1.0f, std::to_string((int)studio.bpm).c_str(), c);
        gridState.setEncoderBg(0, { 0, 0, 0, 0 });

        gridState.setEncoder(1, "Track", (float)(trk + 1), 1.0f, 8.0f, 1.0f, ("T" + std::to_string(trk + 1)).c_str(), c);
        gridState.setEncoderBg(1, { 0, 0, 0, 0 });

        int selStep = (studio.selStep >= 0) ? studio.selStep : 0;
        auto& step = t->sequence[selStep];

        gridState.setEncoder(2, "Length", step.len, 0.25f, 16.0f, 0.25f, nullptr, c);
        gridState.setEncoderBg(2, { 0, 0, 0, 0 });

        gridState.setEncoder(3, nullptr, 0.0f, 0.0f, 0.0f);
        gridState.setEncoderBg(3, { 0, 0, 0, 0 });

        // Encoders 4..7: Step Params
        gridState.setEncoder(4, "Step", (float)(selStep + 1), 1.0f, 64.0f, 1.0f, ("S" + std::to_string(selStep + 1)).c_str(), c);
        gridState.setEncoderBg(4, { 0, 0, 0, 0 });

        gridState.setEncoder(5, "Note", (float)step.note, 12.0f, 108.0f, 1.0f, std::to_string(step.note).c_str(), c);
        gridState.setEncoderBg(5, { 0, 0, 0, 0 });

        gridState.setEncoder(6, "Velo", step.velocity * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, c, "%");
        gridState.setEncoderBg(6, { 0, 0, 0, 0 });

        gridState.setEncoder(7, "Prob", step.condition * 100.0f, 0.0f, 100.0f, 10.0f, nullptr, c, "%");
        gridState.setEncoderBg(7, { 0, 0, 0, 0 });

        // Encoders 8..11: Generator Engine & Generator Params (with distinct background color)
        const char* genEngineName = t->genEngine == 0 ? "Kick" : t->genEngine == 1 ? "Bass" : "Drum";
        gridState.setEncoder(8, "Gen", (float)t->genEngine, 0.0f, 2.0f, 1.0f, genEngineName, c);
        gridState.setEncoderBg(8, genBg);

        if (t->genEngine == 0) { // Kick
            gridState.setEncoder(9, "Tribe Vel", t->genParams[0] * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, c, "%");
            gridState.setEncoderBg(9, genBg);

            gridState.setEncoder(10, "Ghost Dens", t->genParams[1] * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, c, "%");
            gridState.setEncoderBg(10, genBg);

            gridState.setEncoder(11, "End Rumble", t->genParams[2] * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, c, "%");
            gridState.setEncoderBg(11, genBg);
        } else if (t->genEngine == 1) { // Bass
            const char* scaleName = t->genParams[0] < 0.25f ? "Min Pent" : t->genParams[0] < 0.5f ? "Nat Minor" : t->genParams[0] < 0.75f ? "Dorian" : "Acid Chrom";
            gridState.setEncoder(9, "Scale", t->genParams[0] * 100.0f, 0.0f, 100.0f, 5.0f, scaleName, c);
            gridState.setEncoderBg(9, genBg);

            gridState.setEncoder(10, "Density", t->genParams[1] * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, c, "%");
            gridState.setEncoderBg(10, genBg);

            gridState.setEncoder(11, "Slide/Oct", t->genParams[2] * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, c, "%");
            gridState.setEncoderBg(11, genBg);
        } else { // Drum
            const char* styleName = t->genParams[0] < 0.2f ? "Snare" : t->genParams[0] < 0.4f ? "Hat" : t->genParams[0] < 0.6f ? "Clap" : t->genParams[0] < 0.8f ? "Perc" : "Mixed";
            gridState.setEncoder(9, "Style", t->genParams[0] * 100.0f, 0.0f, 100.0f, 5.0f, styleName, c);
            gridState.setEncoderBg(9, genBg);

            gridState.setEncoder(10, "Ghost/Flam", t->genParams[1] * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, c, "%");
            gridState.setEncoderBg(10, genBg);

            const char* intName = t->genParams[2] < 0.3f ? "Sparse" : t->genParams[2] < 0.7f ? "Medium" : "Dense";
            gridState.setEncoder(11, "Interval", t->genParams[2] * 100.0f, 0.0f, 100.0f, 5.0f, intName, c);
            gridState.setEncoderBg(11, genBg);
        }
    }

    void render(Draw& d, int x, int y, int w, int h) override
    {
        std::string titleStr = "VIEW: STEP SEQUENCER - TRACK " + std::to_string(studio.selTrack + 1);
        d.text({ x + 4, y + 2 }, titleStr, 8, { .color = studio.tracks[studio.selTrack]->themeColor, .font = &PoppinsLight_8 });

        // Draw Page Indicator Dots on the right side of the View Title Badge
        auto [pageIdx, totalPages] = getViewPageInfo();
        if (totalPages > 1) {
            int dotW = 5;
            int dotH = 3;
            int gap = 2;
            int totalDotsW = totalPages * dotW + (totalPages - 1) * gap;
            int dotsX = x + w - 6 - totalDotsW;
            int dotsY = y + (h - dotH) / 2;
            for (int p = 0; p < totalPages; p++) {
                Color dotCol = (p + 1 == pageIdx) ? studio.tracks[studio.selTrack]->themeColor : Color { 60, 72, 95, 255 };
                d.filledRect({ dotsX + p * (dotW + gap), dotsY }, { dotW, dotH }, { .color = dotCol });
            }
        }
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
            step.len = std::clamp(step.len + delta * 0.25f, 0.25f, 16.0f);
            break;
        case 4:
            // Empty param slot
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
            t->genEngine = std::clamp(t->genEngine + delta, 0, 2);
            break;
        case 10:
            t->genParams[0] = std::clamp(t->genParams[0] + delta * 0.05f, 0.0f, 1.0f);
            break;
        case 11:
            t->genParams[1] = std::clamp(t->genParams[1] + delta * 0.05f, 0.0f, 1.0f);
            break;
        case 12:
            t->genParams[2] = std::clamp(t->genParams[2] + delta * 0.05f, 0.0f, 1.0f);
            break;
        }

        updatePadLeds();
        updateEncoderLabels();
    }
};
