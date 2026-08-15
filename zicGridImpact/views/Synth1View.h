#pragma once

#include "draw/draw.h"
#include "zicGridImpact/gridState.h"
#include "zicGridImpact/studio.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"

class Synth1View {
public:
    void updateEncoders() {
        gridState.setEncoderParam(0, studio.synth1.pitch, { 40, 200, 255, 255 });
        gridState.setEncoderParam(1, studio.synth1.waveform, { 100, 220, 255, 255 });
        gridState.setEncoderParam(2, studio.synth1.cutoff, { 255, 180, 40, 255 });
        gridState.setEncoderParam(3, studio.synth1.resonance, { 255, 220, 40, 255 });

        gridState.setEncoderParam(4, studio.synth1.release, { 120, 255, 120, 255 });
        gridState.setEncoderParam(5, studio.synth1.envAmt, { 255, 120, 200, 255 });
        gridState.setEncoderParam(6, studio.synth1.filterMorph, { 200, 120, 255, 255 });
        gridState.setEncoderParam(7, studio.synth1.delaySend, { 0, 180, 255, 255 });

        gridState.setEncoderParam(8, studio.synth1.modType, { 255, 100, 100, 255 });
        gridState.setEncoderParam(9, studio.synth1.modDepth, { 255, 160, 40, 255 });
        gridState.setEncoderParam(10, studio.synth1.modSpeed, { 60, 220, 100, 255 });
        gridState.setEncoderParam(11, studio.synth1.crushFm, { 255, 80, 180, 255 });
    }

    void handleEncoder(int idx, int delta) {
        float step = gridState.encoders[idx].step;
        float change = delta * step;
        switch (idx) {
            case 0: studio.synth1.pitch.value = std::clamp(studio.synth1.pitch.value + change, studio.synth1.pitch.min, studio.synth1.pitch.max); break;
            case 1: studio.synth1.waveform.value = std::clamp(studio.synth1.waveform.value + change, studio.synth1.waveform.min, studio.synth1.waveform.max); break;
            case 2: studio.synth1.cutoff.value = std::clamp(studio.synth1.cutoff.value + change, studio.synth1.cutoff.min, studio.synth1.cutoff.max); break;
            case 3: studio.synth1.resonance.value = std::clamp(studio.synth1.resonance.value + change, studio.synth1.resonance.min, studio.synth1.resonance.max); break;
            case 4: studio.synth1.release.value = std::clamp(studio.synth1.release.value + change, studio.synth1.release.min, studio.synth1.release.max); break;
            case 5: studio.synth1.envAmt.value = std::clamp(studio.synth1.envAmt.value + change, studio.synth1.envAmt.min, studio.synth1.envAmt.max); break;
            case 6: studio.synth1.filterMorph.value = std::clamp(studio.synth1.filterMorph.value + change, studio.synth1.filterMorph.min, studio.synth1.filterMorph.max); break;
            case 7: studio.synth1.delaySend.value = std::clamp(studio.synth1.delaySend.value + change, studio.synth1.delaySend.min, studio.synth1.delaySend.max); break;
            case 8: studio.synth1.modType.value = std::clamp(studio.synth1.modType.value + change, studio.synth1.modType.min, studio.synth1.modType.max); break;
            case 9: studio.synth1.modDepth.value = std::clamp(studio.synth1.modDepth.value + change, studio.synth1.modDepth.min, studio.synth1.modDepth.max); break;
            case 10: studio.synth1.modSpeed.value = std::clamp(studio.synth1.modSpeed.value + change, studio.synth1.modSpeed.min, studio.synth1.modSpeed.max); break;
            case 11: studio.synth1.crushFm.value = std::clamp(studio.synth1.crushFm.value + change, studio.synth1.crushFm.min, studio.synth1.crushFm.max); break;
        }
        updateEncoders();
    }

    void render(Draw& d, int x, int y, int w, int h) {
        d.filledRect({ x, y }, { w, h }, { .color = { 18, 22, 30, 255 } });
        d.rect({ x, y }, { w, h }, { .color = { 45, 55, 75, 255 } });

        d.text({ x + 10, y + 8 }, "DRIFT SYNTH 1 ENGINE", 12, { .color = { 40, 200, 255, 255 }, .font = &PoppinsLight_12 });

        int cx = x + 10;
        int cy = y + 36;
        int cw = w - 20;
        int ch = h - 46;

        d.filledRect({ cx, cy }, { cw, ch }, { .color = { 10, 14, 20, 255 } });
        d.rect({ cx, cy }, { cw, ch }, { .color = { 40, 50, 70, 255 } });

        // Filter Response curve rendering
        float cut = studio.synth1.cutoff.value;
        float res = studio.synth1.resonance.value;
        int midY = cy + ch / 2;
        int prevX = cx;
        int prevY = midY;

        for (int px = 0; px < cw; px += 2) {
            float f = (float)px / cw;
            float resp = 1.0f / std::sqrt(1.0f + std::pow(f / (cut + 0.01f), 4.0f));
            if (std::abs(f - cut) < 0.1f) resp += res * 0.8f;
            int currY = cy + ch - 8 - static_cast<int>(resp * (ch - 16));

            if (px > 0) {
                d.line({ prevX, prevY }, { cx + px, currY }, { .color = { 40, 200, 255, 255 } });
            }
            prevX = cx + px;
            prevY = currY;
        }
    }
};
