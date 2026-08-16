#pragma once

#include "draw/draw.h"
#include "zicGridImpact/gridState.h"
#include "zicGridImpact/studio.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"
#include <algorithm>
#include <cmath>
#include <vector>

class Synth2View {
private:
    static constexpr Color THEME_COLOR = { 215, 125, 255, 255 }; // Electric Synth 2 Purple
    float animTime = 0.0f;
    float synth2PulseLevel = 0.0f;

public:
    void updateEncoders() {
        gridState.setEncoderParam(0, studio.synth2.wtSelect, THEME_COLOR);
        gridState.setEncoderParam(1, studio.synth2.wavetable, THEME_COLOR);
        gridState.setEncoderParam(2, studio.synth2.cutoff, THEME_COLOR);
        gridState.setEncoderParam(3, studio.synth2.resonance, THEME_COLOR);

        gridState.setEncoderParam(4, studio.synth2.pitch, THEME_COLOR);

        int cIdx = static_cast<int>(std::round(studio.synth2.chord.value));
        const char* cStr = "Unison";
        if (cIdx == 1) cStr = "Fifth";
        else if (cIdx == 2) cStr = "Octave";
        else if (cIdx == 3) cStr = "Maj 7th";
        else if (cIdx == 4) cStr = "Min 7th";
        else if (cIdx == 5) cStr = "Sus 4";
        gridState.setEncoder(5, "Chord", studio.synth2.chord.value, 0.0f, 5.0f, 1.0f, cStr, THEME_COLOR);

        gridState.setEncoderParam(6, studio.synth2.attack, THEME_COLOR);
        gridState.setEncoderParam(7, studio.synth2.release, THEME_COLOR);

        gridState.setEncoderParam(8, studio.synth2.modType, THEME_COLOR);
        gridState.setEncoderParam(9, studio.synth2.modDepth, THEME_COLOR);
        gridState.setEncoderParam(10, studio.synth2.modSpeed, THEME_COLOR);
        gridState.setEncoderParam(11, studio.synth2.delaySend, THEME_COLOR);
    }

    void handleEncoder(int idx, int delta) {
        float step = gridState.encoders[idx].step;
        float change = delta * step;
        switch (idx) {
            case 0: studio.synth2.wtSelect.set(studio.synth2.wtSelect.value + change); break;
            case 1: studio.synth2.wavetable.set(studio.synth2.wavetable.value + change); break;
            case 2: studio.synth2.cutoff.set(studio.synth2.cutoff.value + change); break;
            case 3: studio.synth2.resonance.set(studio.synth2.resonance.value + change); break;
            case 4: studio.synth2.pitch.set(studio.synth2.pitch.value + change); break;
            case 5: studio.synth2.chord.set(studio.synth2.chord.value + change); break;
            case 6: studio.synth2.attack.set(studio.synth2.attack.value + change); break;
            case 7: studio.synth2.release.set(studio.synth2.release.value + change); break;
            case 8: studio.synth2.modType.set(studio.synth2.modType.value + change); break;
            case 9: studio.synth2.modDepth.set(studio.synth2.modDepth.value + change); break;
            case 10: studio.synth2.modSpeed.set(studio.synth2.modSpeed.value + change); break;
            case 11: studio.synth2.delaySend.set(studio.synth2.delaySend.value + change); break;
        }
        updateEncoders();
    }

    void render(Draw& d, int x, int y, int w, int h) {
        animTime += 0.05f;

        int graphX = x;
        int graphY = y;
        int graphW = w;
        int graphH = h;

        d.filledRect({ graphX, graphY }, { graphW, graphH }, { .color = { 12, 14, 20, 255 } });
        d.rect({ graphX, graphY }, { graphW, graphH }, { .color = THEME_COLOR });

        d.text({ graphX + 12, graphY + 8 }, "DRIFT SYNTH 2 WAVETABLE ENGINE", 12, { .color = THEME_COLOR, .font = &PoppinsLight_12 });

        int innerW = graphW - 24;

        float pitchMidi = studio.synth2.pitch.value;
        float cutVal = studio.synth2.cutoff.value;
        float modD = studio.synth2.modDepth.value * 0.01f;
        float modS = studio.synth2.modSpeed.value * 0.01f;

        int activeFrameIdx = std::clamp((int)std::round(studio.synth2.wavetable.value - 1.0f), 0, 63);

        std::vector<int> sliceFrames = { 0, 8, 16, 24, 32, 40, 48, 56, 63 };
        if (std::find(sliceFrames.begin(), sliceFrames.end(), activeFrameIdx) == sliceFrames.end()) {
            sliceFrames.push_back(activeFrameIdx);
            std::sort(sliceFrames.begin(), sliceFrames.end());
        }

        int numSlices = (int)sliceFrames.size();
        int baseSliceW = innerW - 54;
        int originX = graphX + 12;
        int originY = graphY + graphH - 45;

        std::vector<std::vector<Point>> allSlicePoints(numSlices);

        for (int i = 0; i < numSlices; i++) {
            int frameIdx = sliceFrames[i];
            float z = (float)frameIdx / 63.0f;

            int sliceOffsetX = (int)((1.0f - z) * 54.0f);
            int sliceOffsetY = (int)(-(1.0f - z) * 36.0f);
            int sliceW = (int)(baseSliceW * (0.72f + z * 0.28f));
            int sliceH = (int)(22.0f * (0.55f + z * 0.45f));

            int sx0 = originX + sliceOffsetX;
            int sy0 = originY + sliceOffsetY;

            int ptsCount = 36;
            float driftPhase = animTime * 0.5f;
            for (int p = 0; p <= ptsCount; p++) {
                float t = (float)p / (float)ptsCount;
                float tDrift = std::fmod(t + driftPhase, 1.0f);

                float rawWave = studio.synth2.wt.getSampleAt(frameIdx, tDrift);
                float freqNorm = t;
                float filterDamp = 1.0f / (1.0f + std::pow(freqNorm / std::max(0.04f, cutVal), 3.0f));

                float waveH = rawWave * filterDamp * sliceH;
                if (frameIdx == activeFrameIdx && std::abs(modD) > 0.05f) {
                    waveH += std::sin(t * 16.0f + animTime * 8.0f) * (modD * 4.5f) * (modS * 2.0f);
                }

                int px = sx0 + (int)(t * sliceW);
                int py = sy0 - (int)waveH;
                allSlicePoints[i].push_back({ px, py });
            }
        }

        // Connecting Lattice Wireframe Lines across keyframes
        for (int i = 0; i < numSlices - 1; i++) {
            float z = (float)sliceFrames[i] / 63.0f;
            uint8_t meshAlpha = (uint8_t)(70 + z * 125.0f);
            Color meshCol = Color { 160, 90, 225, meshAlpha };

            size_t step = 4;
            for (size_t p = 0; p < allSlicePoints[i].size(); p += step) {
                d.line(allSlicePoints[i][p], allSlicePoints[i + 1][p], { .color = meshCol });
            }
        }

        // 3D Slice Curves (Back-to-Front)
        for (int i = 0; i < numSlices; i++) {
            int frameIdx = sliceFrames[i];
            float z = (float)frameIdx / 63.0f;
            const auto& slicePts = allSlicePoints[i];

            if (frameIdx == activeFrameIdx) {
                int sliceW = (int)(baseSliceW * (0.72f + z * 0.28f));
                int sliceOffsetX = (int)((1.0f - z) * 54.0f);
                int sliceOffsetY = (int)(-(1.0f - z) * 36.0f);
                int sx0 = originX + sliceOffsetX;
                int sy0 = originY + sliceOffsetY;

                std::vector<Point> fillPoly = slicePts;
                fillPoly.push_back({ sx0 + sliceW, sy0 });
                fillPoly.push_back({ sx0, sy0 });
                d.filledPolygon(fillPoly, { .color = { 220, 110, 255, 75 } });
                d.lines(slicePts, { .color = { 255, 195, 255, 255 }, .thickness = 1 });
            } else {
                uint8_t alpha = (uint8_t)(90 + z * 115);
                Color depthCol = (frameIdx < activeFrameIdx) ? Color { 150, 80, 210, alpha } : Color { 205, 120, 255, alpha };
                d.lines(slicePts, { .color = depthCol, .thickness = 1 });
            }
        }

        // Pitch Ribbon at Base
        float pitchHz = 440.0f * std::pow(2.0f, (pitchMidi - 69.0f) / 12.0f);
        int freqY = graphY + graphH - 14;
        std::vector<Point> pitchWave;
        float cycScale = (pitchHz / 110.0f) * 0.15f;
        for (int gx = 0; gx < innerW; gx += 2) {
            float t = (float)gx / (float)innerW;
            float wave = std::sin(t * (cycScale * 25.0f) + animTime * 4.0f) * 3.5f;
            pitchWave.push_back({ graphX + 12 + gx, freqY + (int)wave });
        }
        d.lines(pitchWave, { .color = THEME_COLOR });
    }
};
