#pragma once

#include "draw/draw.h"
#include "zicGridImpact/gridState.h"
#include "zicGridImpact/studio.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"

class Synth2View {
public:
    void updateEncoders() {
        gridState.setEncoderParam(0, studio.synth2.wtSelect, { 200, 80, 255, 255 });
        gridState.setEncoderParam(1, studio.synth2.wavetable, { 255, 120, 200, 255 });
        gridState.setEncoderParam(2, studio.synth2.cutoff, { 255, 180, 40, 255 });
        gridState.setEncoderParam(3, studio.synth2.resonance, { 255, 220, 40, 255 });

        gridState.setEncoderParam(4, studio.synth2.pitch, { 40, 200, 255, 255 });

        int cIdx = static_cast<int>(std::round(studio.synth2.chord.value));
        const char* cStr = "Unison";
        if (cIdx == 1) cStr = "Fifth";
        else if (cIdx == 2) cStr = "Octave";
        else if (cIdx == 3) cStr = "Maj 7th";
        else if (cIdx == 4) cStr = "Min 7th";
        else if (cIdx == 5) cStr = "Sus 4";
        gridState.setEncoder(5, "Chord", studio.synth2.chord.value, 0.0f, 5.0f, 1.0f, cStr, { 255, 215, 0, 255 });

        gridState.setEncoderParam(6, studio.synth2.attack, { 100, 220, 100, 255 });
        gridState.setEncoderParam(7, studio.synth2.release, { 120, 255, 120, 255 });

        gridState.setEncoderParam(8, studio.synth2.modType, { 255, 100, 100, 255 });
        gridState.setEncoderParam(9, studio.synth2.modDepth, { 255, 160, 40, 255 });
        gridState.setEncoderParam(10, studio.synth2.modSpeed, { 60, 220, 100, 255 });
        gridState.setEncoderParam(11, studio.synth2.delaySend, { 0, 180, 255, 255 });
    }

    void handleEncoder(int idx, int delta) {
        float step = gridState.encoders[idx].step;
        float change = delta * step;
        switch (idx) {
            case 0: studio.synth2.wtSelect.value = std::clamp(studio.synth2.wtSelect.value + change, studio.synth2.wtSelect.min, studio.synth2.wtSelect.max); break;
            case 1: studio.synth2.wavetable.value = std::clamp(studio.synth2.wavetable.value + change, studio.synth2.wavetable.min, studio.synth2.wavetable.max); break;
            case 2: studio.synth2.cutoff.value = std::clamp(studio.synth2.cutoff.value + change, studio.synth2.cutoff.min, studio.synth2.cutoff.max); break;
            case 3: studio.synth2.resonance.value = std::clamp(studio.synth2.resonance.value + change, studio.synth2.resonance.min, studio.synth2.resonance.max); break;
            case 4: studio.synth2.pitch.value = std::clamp(studio.synth2.pitch.value + change, studio.synth2.pitch.min, studio.synth2.pitch.max); break;
            case 5: studio.synth2.chord.value = std::clamp(studio.synth2.chord.value + change, 0.0f, 5.0f); break;
            case 6: studio.synth2.attack.value = std::clamp(studio.synth2.attack.value + change, studio.synth2.attack.min, studio.synth2.attack.max); break;
            case 7: studio.synth2.release.value = std::clamp(studio.synth2.release.value + change, studio.synth2.release.min, studio.synth2.release.max); break;
            case 8: studio.synth2.modType.value = std::clamp(studio.synth2.modType.value + change, studio.synth2.modType.min, studio.synth2.modType.max); break;
            case 9: studio.synth2.modDepth.value = std::clamp(studio.synth2.modDepth.value + change, studio.synth2.modDepth.min, studio.synth2.modDepth.max); break;
            case 10: studio.synth2.modSpeed.value = std::clamp(studio.synth2.modSpeed.value + change, studio.synth2.modSpeed.min, studio.synth2.modSpeed.max); break;
            case 11: studio.synth2.delaySend.value = std::clamp(studio.synth2.delaySend.value + change, studio.synth2.delaySend.min, studio.synth2.delaySend.max); break;
        }
        updateEncoders();
    }

    void render(Draw& d, int x, int y, int w, int h) {
        d.filledRect({ x, y }, { w, h }, { .color = { 18, 22, 30, 255 } });
        d.rect({ x, y }, { w, h }, { .color = { 45, 55, 75, 255 } });

        d.text({ x + 10, y + 8 }, "DRIFT SYNTH 2 WAVETABLE ENGINE", 12, { .color = { 200, 80, 255, 255 }, .font = &PoppinsLight_12 });

        int cx = x + 10;
        int cy = y + 36;
        int cw = w - 20;
        int ch = h - 46;

        d.filledRect({ cx, cy }, { cw, ch }, { .color = { 10, 14, 20, 255 } });
        d.rect({ cx, cy }, { cw, ch }, { .color = { 40, 50, 70, 255 } });

        // Wavetable graphic visualization
        float pos = studio.synth2.wavetable.value;
        int midY = cy + ch / 2;
        int prevX = cx;
        int prevY = midY;

        for (int px = 0; px < cw; px += 2) {
            float phase = (float)px / cw * 6.28318f * 3.0f;
            float morph = std::sin(phase) * (1.0f - pos) + (std::sin(phase) > 0 ? 1.0f : -1.0f) * pos;
            int currY = midY + static_cast<int>(morph * (ch / 2.5f));

            if (px > 0) {
                d.line({ prevX, prevY }, { cx + px, currY }, { .color = { 200, 80, 255, 255 } });
            }
            prevX = cx + px;
            prevY = currY;
        }
    }
};
