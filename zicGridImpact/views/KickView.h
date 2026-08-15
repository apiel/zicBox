#pragma once

#include "draw/draw.h"
#include "zicGridImpact/gridState.h"
#include "zicGridImpact/studio.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"

class KickView {
public:
    void updateEncoders() {
        gridState.setEncoderParam(0, studio.kick.baseFreq, { 255, 100, 100, 255 });
        gridState.setEncoderParam(1, studio.kick.clickAmt, { 255, 180, 80, 255 });
        gridState.setEncoderParam(2, studio.kick.duration, { 255, 220, 100, 255 });
        gridState.setEncoderParam(3, studio.kick.vcoMorph, { 100, 220, 255, 255 });

        gridState.setEncoderParam(4, studio.kick.fmDepth, { 200, 120, 255, 255 });
        gridState.setEncoderParam(5, studio.kick.drive, { 255, 80, 180, 255 });
        gridState.setEncoderParam(6, studio.kick.rumbleAmt, { 80, 200, 255, 255 });
        gridState.setEncoderParam(7, studio.kick.rumbleGap, { 120, 255, 120, 255 });

        for (int i = 8; i < 12; ++i) {
            gridState.setEncoder(i, "", 0.0f, 0.0f, 1.0f, 1.0f, nullptr, { 0, 0, 0, 0 });
        }
    }

    void handleEncoder(int idx, int delta) {
        float step = gridState.encoders[idx].step;
        float change = delta * step;
        switch (idx) {
            case 0: studio.kick.baseFreq.value = std::clamp(studio.kick.baseFreq.value + change, studio.kick.baseFreq.min, studio.kick.baseFreq.max); break;
            case 1: studio.kick.clickAmt.value = std::clamp(studio.kick.clickAmt.value + change, studio.kick.clickAmt.min, studio.kick.clickAmt.max); break;
            case 2: studio.kick.duration.value = std::clamp(studio.kick.duration.value + change, studio.kick.duration.min, studio.kick.duration.max); break;
            case 3: studio.kick.vcoMorph.value = std::clamp(studio.kick.vcoMorph.value + change, studio.kick.vcoMorph.min, studio.kick.vcoMorph.max); break;
            case 4: studio.kick.fmDepth.value = std::clamp(studio.kick.fmDepth.value + change, studio.kick.fmDepth.min, studio.kick.fmDepth.max); break;
            case 5: studio.kick.drive.value = std::clamp(studio.kick.drive.value + change, studio.kick.drive.min, studio.kick.drive.max); break;
            case 6: studio.kick.rumbleAmt.value = std::clamp(studio.kick.rumbleAmt.value + change, studio.kick.rumbleAmt.min, studio.kick.rumbleAmt.max); break;
            case 7: studio.kick.rumbleGap.value = std::clamp(studio.kick.rumbleGap.value + change, studio.kick.rumbleGap.min, studio.kick.rumbleGap.max); break;
        }
        updateEncoders();
    }

    void render(Draw& d, int x, int y, int w, int h) {
        d.filledRect({ x, y }, { w, h }, { .color = { 18, 22, 30, 255 } });
        d.rect({ x, y }, { w, h }, { .color = { 45, 55, 75, 255 } });

        d.text({ x + 10, y + 8 }, "DRIFT KICK SYNTHESIZER ENGINE", 12, { .color = { 255, 100, 100, 255 }, .font = &PoppinsLight_12 });

        // Waveform graphic simulation
        int waveY = y + 36;
        int waveH = h - 46;
        int cx = x + 10;
        int cw = w - 20;

        d.filledRect({ cx, waveY }, { cw, waveH }, { .color = { 10, 14, 20, 255 } });
        d.rect({ cx, waveY }, { cw, waveH }, { .color = { 40, 50, 70, 255 } });

        // Draw kick envelope curve
        float freq = studio.kick.baseFreq.value;
        float dur = studio.kick.duration.value;
        float fm = studio.kick.fmDepth.value;

        int midY = waveY + waveH / 2;
        int prevX = cx;
        int prevY = midY;

        for (int px = 0; px < cw; px += 2) {
            float t = (float)px / cw;
            float env = std::exp(-t * (4.0f / (dur * 0.01f + 0.1f)));
            float osc = std::sin(t * (freq * 0.5f + fm * env * 2.0f));
            int currY = midY + static_cast<int>(osc * env * (waveH / 2.2f));

            if (px > 0) {
                d.line({ prevX, prevY }, { cx + px, currY }, { .color = { 255, 120, 80, 255 } });
            }
            prevX = cx + px;
            prevY = currY;
        }
    }
};
