#pragma once

#include "draw/draw.h"
#include "zicGridImpact/gridState.h"
#include "zicGridImpact/studio.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"

class MasterView {
public:
    void updateEncoders() {
        gridState.setEncoder(0, "KICK Lvl", studio.mixer.kickLevel, 0.0f, 1.0f, 0.01f, nullptr, { 255, 80, 80, 255 });
        gridState.setEncoder(1, "SYN1 Lvl", studio.mixer.synth1Level, 0.0f, 1.0f, 0.01f, nullptr, { 40, 200, 255, 255 });
        gridState.setEncoder(2, "SYN2 Lvl", studio.mixer.synth2Level, 0.0f, 1.0f, 0.01f, nullptr, { 200, 80, 255, 255 });
        gridState.setEncoder(3, "MASTER Vol", studio.mixer.volume, 0.0f, 1.0f, 0.02f, nullptr, { 255, 215, 0, 255 });

        gridState.setEncoder(4, "DELAY Time", studio.mixer.delayTimeMs, 10.0f, 1000.0f, 10.0f, nullptr, { 0, 180, 255, 255 }, "ms");
        gridState.setEncoder(5, "DELAY Fdbk", studio.mixer.delayFeedback, 0.0f, 0.95f, 0.01f, nullptr, { 0, 180, 255, 255 });
        gridState.setEncoder(6, "MAST Cutoff", studio.mixer.masterCutoff, 0.0f, 1.0f, 0.01f, nullptr, { 255, 160, 40, 255 });
        gridState.setEncoder(7, "MAST Reso", studio.mixer.masterResonance, 0.0f, 0.95f, 0.01f, nullptr, { 255, 160, 40, 255 });

        gridState.setEncoder(8, "SCAT Crunch", studio.scatter.params[4][0], 0.0f, 1.0f, 0.02f, nullptr, { 0, 220, 255, 255 });
        gridState.setEncoder(9, "SCAT Drive", studio.scatter.params[5][0], 0.0f, 1.0f, 0.02f, nullptr, { 100, 120, 255, 255 });
        gridState.setEncoder(10, "SCAT Dist", studio.scatter.params[2][0], 0.0f, 1.0f, 0.02f, nullptr, { 255, 80, 180, 255 });
        gridState.setEncoder(11, "SCAT Acid", studio.scatter.params[6][0], 0.0f, 1.0f, 0.02f, nullptr, { 60, 220, 100, 255 });
    }

    void handleEncoder(int idx, int delta) {
        float step = gridState.encoders[idx].step;
        float change = delta * step;
        switch (idx) {
            case 0: studio.mixer.kickLevel = std::clamp(studio.mixer.kickLevel + change, 0.0f, 1.0f); break;
            case 1: studio.mixer.synth1Level = std::clamp(studio.mixer.synth1Level + change, 0.0f, 1.0f); break;
            case 2: studio.mixer.synth2Level = std::clamp(studio.mixer.synth2Level + change, 0.0f, 1.0f); break;
            case 3: studio.mixer.volume = std::clamp(studio.mixer.volume + change, 0.0f, 1.0f); break;
            case 4: studio.mixer.delayTimeMs = std::clamp(studio.mixer.delayTimeMs + change, 10.0f, 1000.0f); break;
            case 5: studio.mixer.delayFeedback = std::clamp(studio.mixer.delayFeedback + change, 0.0f, 0.95f); break;
            case 6: studio.mixer.masterCutoff = std::clamp(studio.mixer.masterCutoff + change, 0.0f, 1.0f); break;
            case 7: studio.mixer.masterResonance = std::clamp(studio.mixer.masterResonance + change, 0.0f, 0.95f); break;
            case 8: studio.scatter.params[4][0] = std::clamp(studio.scatter.params[4][0] + change, 0.0f, 1.0f); break;
            case 9: studio.scatter.params[5][0] = std::clamp(studio.scatter.params[5][0] + change, 0.0f, 1.0f); break;
            case 10: studio.scatter.params[2][0] = std::clamp(studio.scatter.params[2][0] + change, 0.0f, 1.0f); break;
            case 11: studio.scatter.params[6][0] = std::clamp(studio.scatter.params[6][0] + change, 0.0f, 1.0f); break;
        }
        updateEncoders();
    }

    void render(Draw& d, int x, int y, int w, int h) {
        d.filledRect({ x, y }, { w, h }, { .color = { 18, 22, 30, 255 } });
        d.rect({ x, y }, { w, h }, { .color = { 45, 55, 75, 255 } });

        d.text({ x + 10, y + 8 }, "4-CHANNEL LIVE MIXER & MASTER FX", 12, { .color = { 255, 215, 0, 255 }, .font = &PoppinsLight_12 });

        int vuAreaY = y + 32;
        int vuAreaH = h - 44;
        int vuWidth = 32;
        int gap = 24;
        int startX = x + (w - (4 * vuWidth + 3 * gap)) / 2;

        const char* channelNames[4] = { "KICK", "SYN 1", "SYN 2", "MASTER" };
        float peaks[4] = {
            studio.mixer.peakKick.load(),
            studio.mixer.peakSynth1.load(),
            studio.mixer.peakSynth2.load(),
            studio.mixer.peakMaster.load()
        };
        Color barColors[4] = {
            { 255, 80, 80, 255 },
            { 40, 200, 255, 255 },
            { 200, 80, 255, 255 },
            { 255, 215, 0, 255 }
        };

        for (int ch = 0; ch < 4; ++ch) {
            int cx = startX + ch * (vuWidth + gap);
            d.textCentered({ cx + vuWidth / 2, vuAreaY }, channelNames[ch], 8, { .color = { 180, 190, 210, 255 }, .font = &PoppinsLight_8 });

            int meterY = vuAreaY + 16;
            int meterH = vuAreaH - 32;
            d.filledRect({ cx, meterY }, { vuWidth, meterH }, { .color = { 28, 34, 45, 255 } });
            d.rect({ cx, meterY }, { vuWidth, meterH }, { .color = { 60, 75, 100, 255 } });

            float p = std::clamp(peaks[ch], 0.0f, 1.0f);
            int fillH = static_cast<int>(p * (meterH - 4));
            if (fillH > 0) {
                d.filledRect({ cx + 2, meterY + meterH - 2 - fillH }, { vuWidth - 4, fillH }, { .color = barColors[ch] });
            }
        }
    }
};
