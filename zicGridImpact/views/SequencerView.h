#pragma once

#include "draw/draw.h"
#include "zicGridImpact/gridState.h"
#include "zicGridImpact/studio.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

class SequencerView {
private:
    static constexpr Color THEME_COLOR = { 60, 220, 100, 255 }; // Bright Sequencer Green

public:
    void updateEncoders() {
        gridState.setEncoder(0, "BPM", studio.seq.bpm, 40.0f, 260.0f, 1.0f, nullptr, THEME_COLOR, "bpm");
        gridState.setEncoder(1, "GEN Kick", studio.seq.genKick, 0.0f, 1.0f, 0.05f, nullptr, THEME_COLOR);

        int t1 = static_cast<int>(std::round(studio.seq.synth1TrigMode));
        const char* str1 = (t1 >= 0 && t1 < (int)studio.seq.trigDisplayStrings.size()) ? studio.seq.trigDisplayStrings[t1].c_str() : "follow";
        gridState.setEncoder(2, "SYN1 Trig", studio.seq.synth1TrigMode, 0.0f, 7.0f, 1.0f, str1, THEME_COLOR);

        int t2 = static_cast<int>(std::round(studio.seq.synth2TrigMode));
        const char* str2 = (t2 >= 0 && t2 < (int)studio.seq.trigDisplayStrings.size()) ? studio.seq.trigDisplayStrings[t2].c_str() : "4";
        gridState.setEncoder(3, "SYN2 Trig", studio.seq.synth2TrigMode, 0.0f, 7.0f, 1.0f, str2, THEME_COLOR);

        int tc = static_cast<int>(std::round(studio.seq.chaosTrigMode));
        const char* strC = (tc >= 0 && tc < (int)studio.seq.trigDisplayStrings.size()) ? studio.seq.trigDisplayStrings[tc].c_str() : "2";
        gridState.setEncoder(4, "CHS Trig", studio.seq.chaosTrigMode, 0.0f, 7.0f, 1.0f, strC, THEME_COLOR);

        gridState.setEncoder(5, "STEPS", (float)studio.seq.totalSteps, 16.0f, 64.0f, 16.0f, nullptr, THEME_COLOR);
        gridState.setEncoder(6, "RPT Rate", (float)studio.seq.kickRepeatRate, 1.0f, 16.0f, 1.0f, nullptr, THEME_COLOR);
        gridState.setEncoder(7, "RND Bar", (float)studio.seq.kickRandomBarSteps, 1.0f, 16.0f, 1.0f, nullptr, THEME_COLOR);

        for (int i = 8; i < 12; ++i) {
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
            case 4: studio.seq.chaosTrigMode = std::clamp(studio.seq.chaosTrigMode + change, 0.0f, 7.0f); break;
            case 5:
                studio.seq.totalSteps = std::clamp((int)(studio.seq.totalSteps + change), 16, 64);
                studio.seq.updateKickEuclidean();
                break;
            case 6: studio.seq.kickRepeatRate = std::clamp((int)(studio.seq.kickRepeatRate + change), 1, 16); break;
            case 7: studio.seq.kickRandomBarSteps = std::clamp((int)(studio.seq.kickRandomBarSteps + change), 1, 16); break;
        }
        updateEncoders();
    }

    void render(Draw& d, int x, int y, int w, int h) {
        int graphX = x;
        int graphY = y;
        int graphW = w;
        int graphH = h;

        d.filledRect({ graphX, graphY }, { graphW, graphH }, { .color = { 12, 14, 20, 255 } });
        d.rect({ graphX, graphY }, { graphW, graphH }, { .color = THEME_COLOR });

        d.text({ graphX + 12, graphY + 8 }, "PATTERNS & TEKNO STEP SEQUENCER", 12, { .color = THEME_COLOR, .font = &PoppinsLight_12 });

        int gridX = graphX + 8;
        int gridY = graphY + 32;
        int stepStride = (graphW - 80) / 64;
        int cellW = std::max(2, stepStride - 1);
        int rowH = 26;

        const char* trackNames[4] = { "KICK", "SYN1", "SYN2", "CHS" };
        Color trackColors[4] = { { 0, 195, 255, 255 }, { 0, 240, 190, 255 }, { 215, 125, 255, 255 }, { 255, 45, 85, 255 } };

        // Draw Bar Headers
        for (int b = 0; b < 4; b++) {
            int bx = gridX + 50 + b * 16 * stepStride;
            std::string barLabel = "Bar " + std::to_string(b + 1);
            d.text({ bx, gridY }, barLabel, 8, { .color = THEME_COLOR, .font = &PoppinsLight_8 });
        }

        int tracksStartY = gridY + 16;

        for (int r = 0; r < 4; r++) {
            int ry = tracksStartY + r * (rowH + 8);

            d.filledRect({ gridX + 46, ry - 2 }, { 64 * stepStride + 4, rowH + 4 }, { .color = { 18, 22, 32, 255 } });
            d.text({ gridX, ry + 6 }, trackNames[r], 8, { .color = trackColors[r], .font = &PoppinsLight_8 });

            for (int s = 0; s < studio.seq.totalSteps; s++) {
                int sx = gridX + 50 + s * stepStride;
                bool isHit = false;
                if (r == 0) isHit = studio.seq.kickPattern[s];
                else if (r == 1) isHit = studio.seq.shouldTrigSynth((int)std::round(studio.seq.synth1TrigMode), s, studio.seq.kickPattern[s]);
                else if (r == 2) isHit = studio.seq.shouldTrigSynth((int)std::round(studio.seq.synth2TrigMode), s, studio.seq.kickPattern[s]);
                else if (r == 3) isHit = studio.seq.shouldTrigSynth((int)std::round(studio.seq.chaosTrigMode), s, studio.seq.kickPattern[s]);

                Color cellBg;
                if (s == studio.seq.currentStep) {
                    cellBg = { 255, 255, 255, 255 }; // Playhead highlight
                } else if (isHit) {
                    cellBg = trackColors[r];
                } else if (s % 4 == 0) {
                    cellBg = { 90, 105, 130, 255 };
                } else {
                    cellBg = { 45, 52, 68, 255 };
                }

                d.filledRect({ sx, ry }, { cellW, rowH }, { .color = cellBg });
            }
        }

        std::string statusStr = "Status: " + std::string(studio.seq.isPlaying ? "PLAYING" : "PAUSED") +
                                " | BPM: " + std::to_string((int)studio.seq.bpm) +
                                " | Step: " + std::to_string(studio.seq.currentStep + 1) + "/" + std::to_string(studio.seq.totalSteps);
        d.text({ graphX + 12, graphY + graphH - 18 }, statusStr, 8, { .color = Color { 200, 215, 235, 255 }, .font = &PoppinsLight_8 });
    }
};
