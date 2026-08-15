#pragma once

#include "draw/draw.h"
#include "zicGridImpact/gridState.h"
#include "zicGridImpact/studio.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"

class SequencerView {
public:
    void updateEncoders() {
        gridState.setEncoder(0, "BPM", studio.seq.bpm, 40.0f, 260.0f, 1.0f, nullptr, { 60, 220, 100, 255 }, "bpm");
        gridState.setEncoder(1, "GEN Kick", studio.seq.genKick, 0.0f, 1.0f, 0.05f, nullptr, { 255, 160, 40, 255 });

        int t1 = static_cast<int>(std::round(studio.seq.synth1TrigMode));
        const char* str1 = (t1 >= 0 && t1 < (int)studio.seq.trigDisplayStrings.size()) ? studio.seq.trigDisplayStrings[t1].c_str() : "follow";
        gridState.setEncoder(2, "SYN1 Trig", studio.seq.synth1TrigMode, 0.0f, 7.0f, 1.0f, str1, { 40, 200, 255, 255 });

        int t2 = static_cast<int>(std::round(studio.seq.synth2TrigMode));
        const char* str2 = (t2 >= 0 && t2 < (int)studio.seq.trigDisplayStrings.size()) ? studio.seq.trigDisplayStrings[t2].c_str() : "4";
        gridState.setEncoder(3, "SYN2 Trig", studio.seq.synth2TrigMode, 0.0f, 7.0f, 1.0f, str2, { 200, 80, 255, 255 });

        gridState.setEncoder(4, "STEPS", (float)studio.seq.totalSteps, 16.0f, 64.0f, 16.0f, nullptr, { 220, 220, 220, 255 });
        gridState.setEncoder(5, "RPT Rate", (float)studio.seq.kickRepeatRate, 1.0f, 16.0f, 1.0f, nullptr, { 255, 215, 0, 255 });

        for (int i = 6; i < 12; ++i) {
            gridState.setEncoder(i, "", 0.0f, 0.0f, 1.0f, 1.0f, nullptr, { 0, 0, 0, 0 });
        }
    }

    void handleEncoder(int idx, int delta) {
        float step = gridState.encoders[idx].step;
        float change = delta * step;
        switch (idx) {
            case 0: studio.seq.setBpm(studio.seq.bpm + change); break;
            case 1:
                studio.seq.genKick = std::clamp(studio.seq.genKick + change, 0.0f, 1.0f);
                studio.seq.updateKickEuclidean();
                break;
            case 2: studio.seq.synth1TrigMode = std::clamp(studio.seq.synth1TrigMode + change, 0.0f, 7.0f); break;
            case 3: studio.seq.synth2TrigMode = std::clamp(studio.seq.synth2TrigMode + change, 0.0f, 7.0f); break;
            case 4:
                studio.seq.totalSteps = std::clamp((int)(studio.seq.totalSteps + change), 16, 64);
                studio.seq.updateKickEuclidean();
                break;
            case 5: studio.seq.kickRepeatRate = std::clamp((int)(studio.seq.kickRepeatRate + change), 1, 16); break;
        }
        updateEncoders();
    }

    void render(Draw& d, int x, int y, int w, int h) {
        d.filledRect({ x, y }, { w, h }, { .color = { 18, 22, 30, 255 } });
        d.rect({ x, y }, { w, h }, { .color = { 45, 55, 75, 255 } });

        d.text({ x + 10, y + 8 }, "PATTERNS & TEKNO STEP SEQUENCER", 12, { .color = { 60, 220, 100, 255 }, .font = &PoppinsLight_12 });

        // Draw 16 step blocks visualizer
        int seqY = y + 36;
        int boxW = (w - 20 - 15 * 2) / 16;
        int boxH = 40;

        for (int i = 0; i < 16; ++i) {
            int bx = x + 10 + i * (boxW + 2);
            bool isActive = studio.seq.kickPattern[i];
            bool isCurrent = (studio.seq.currentStep % 16 == i);

            Color boxCol = isActive ? Color { 255, 160, 40, 255 } : Color { 30, 38, 50, 255 };
            if (isCurrent) boxCol = Color { 255, 255, 255, 255 };

            d.filledRect({ bx, seqY }, { boxW, boxH }, { .color = boxCol });
            d.rect({ bx, seqY }, { boxW, boxH }, { .color = { 80, 95, 120, 255 } });

            d.textCentered({ bx + boxW / 2, seqY + boxH / 2 - 4 }, std::to_string(i + 1), 8,
                { .color = (isCurrent || isActive) ? Color { 0, 0, 0, 255 } : Color { 160, 170, 190, 255 }, .font = &PoppinsLight_8 });
        }

        // Status text
        std::string statusStr = "Status: " + std::string(studio.seq.isPlaying ? "PLAYING" : "PAUSED") +
                                " | BPM: " + std::to_string((int)studio.seq.bpm) +
                                " | Step: " + std::to_string(studio.seq.currentStep + 1) + "/" + std::to_string(studio.seq.totalSteps);
        d.text({ x + 10, y + h - 20 }, statusStr, 8, { .color = { 180, 200, 220, 255 }, .font = &PoppinsLight_8 });
    }
};
