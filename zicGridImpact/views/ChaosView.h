#pragma once

#include "draw/draw.h"
#include "zicGridImpact/gridState.h"
#include "zicGridImpact/studio.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"
#include <algorithm>
#include <cmath>
#include <vector>

class ChaosView {
private:
    static constexpr Color THEME_COLOR = { 255, 45, 85, 255 }; // Electric Crimson Chaos
    float animTime = 0.0f;
    float chaosPulseLevel = 0.0f;

    // Lorenz Phase Portrait Trajectory Points
    float px = 0.1f, py = 0.0f, pz = 0.1f;

    float lerp(float a, float b, float t) { return a + t * (b - a); }

public:
    void updateEncoders() {
        gridState.setEncoderParam(0, studio.chaos.pitch, THEME_COLOR);
        gridState.setEncoderParam(1, studio.chaos.vcoMorph, THEME_COLOR);
        gridState.setEncoderParam(2, studio.chaos.drive, THEME_COLOR);
        gridState.setEncoderParam(3, studio.chaos.release, THEME_COLOR);

        gridState.setEncoderParam(4, studio.chaos.crush, THEME_COLOR);
        gridState.setEncoderParam(5, studio.chaos.fmDepth, THEME_COLOR);
        gridState.setEncoderParam(6, studio.chaos.ringMod, THEME_COLOR);
        gridState.setEncoderParam(7, studio.chaos.color, THEME_COLOR);

        gridState.setEncoderParam(8, studio.chaos.pitchGlitch, THEME_COLOR);
        gridState.setEncoderParam(9, studio.chaos.lfoSpeed, THEME_COLOR);
        gridState.setEncoderParam(10, studio.chaos.lfoDepth, THEME_COLOR);
        gridState.setEncoderParam(11, studio.chaos.delaySend, THEME_COLOR);
    }

    void handleEncoder(int idx, int delta) {
        float step = gridState.encoders[idx].step;
        float change = delta * step;
        switch (idx) {
            case 0: studio.chaos.pitch.value = std::clamp(studio.chaos.pitch.value + change, studio.chaos.pitch.min, studio.chaos.pitch.max); break;
            case 1: studio.chaos.vcoMorph.value = std::clamp(studio.chaos.vcoMorph.value + change, studio.chaos.vcoMorph.min, studio.chaos.vcoMorph.max); break;
            case 2: studio.chaos.drive.value = std::clamp(studio.chaos.drive.value + change, studio.chaos.drive.min, studio.chaos.drive.max); break;
            case 3: studio.chaos.release.value = std::clamp(studio.chaos.release.value + change, studio.chaos.release.min, studio.chaos.release.max); break;
            case 4: studio.chaos.crush.value = std::clamp(studio.chaos.crush.value + change, studio.chaos.crush.min, studio.chaos.crush.max); break;
            case 5: studio.chaos.fmDepth.value = std::clamp(studio.chaos.fmDepth.value + change, studio.chaos.fmDepth.min, studio.chaos.fmDepth.max); break;
            case 6: studio.chaos.ringMod.value = std::clamp(studio.chaos.ringMod.value + change, studio.chaos.ringMod.min, studio.chaos.ringMod.max); break;
            case 7: studio.chaos.color.value = std::clamp(studio.chaos.color.value + change, studio.chaos.color.min, studio.chaos.color.max); break;
            case 8: studio.chaos.pitchGlitch.value = std::clamp(studio.chaos.pitchGlitch.value + change, studio.chaos.pitchGlitch.min, studio.chaos.pitchGlitch.max); break;
            case 9: studio.chaos.lfoSpeed.value = std::clamp(studio.chaos.lfoSpeed.value + change, studio.chaos.lfoSpeed.min, studio.chaos.lfoSpeed.max); break;
            case 10: studio.chaos.lfoDepth.value = std::clamp(studio.chaos.lfoDepth.value + change, studio.chaos.lfoDepth.min, studio.chaos.lfoDepth.max); break;
            case 11: studio.chaos.delaySend.value = std::clamp(studio.chaos.delaySend.value + change, studio.chaos.delaySend.min, studio.chaos.delaySend.max); break;
        }
        updateEncoders();
    }

    void render(Draw& d, int x, int y, int w, int h) {
        animTime += 0.05f;

        int graphX = x;
        int graphY = y;
        int graphW = w;
        int graphH = h;

        d.filledRect({ graphX, graphY }, { graphW, graphH }, { .color = { 20, 10, 14, 255 } });
        d.rect({ graphX, graphY }, { graphW, graphH }, { .color = THEME_COLOR });

        d.text({ graphX + 12, graphY + 8 }, "IMPACT CHAOS CIRCUIT-BENT NOISE ENGINE", 12, { .color = THEME_COLOR, .font = &PoppinsLight_12 });

        int cx = graphX + graphW / 2;
        int cy = graphY + (graphH / 2) - 5;

        float drvVal = std::clamp(studio.chaos.drive.value * 0.01f, 0.0f, 1.0f);
        float relMs = studio.chaos.release.value;

        // Trigger Pulse Decay & Radiant Crimson Shockwave
        float decayRate = 12.0f / (std::clamp(relMs, 20.0f, 3000.0f) + 40.0f);
        gridState.chaosPulseLevel = std::max(0.0f, gridState.chaosPulseLevel - decayRate);

        if (gridState.chaosPulseLevel > 0.01f) {
            for (int r = 0; r < 4; r++) {
                float pFactor = gridState.chaosPulseLevel - (r * 0.18f);
                if (pFactor > 0.0f) {
                    int radius = (int)(25.0f + (1.0f - pFactor) * 80.0f + r * 15);
                    uint8_t alpha = (uint8_t)(pFactor * 160.0f);
                    d.circle({ cx, cy }, radius, { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, alpha } });
                }
            }
        }

        // Render Real-time Lorenz Attractor Phase Portrait Graph
        std::vector<Point> trajPoints;
        float curX = px, curY = py, curZ = pz;
        float dt = 0.014f;

        for (int i = 0; i < 90; ++i) {
            float dx = 10.0f * (curY - curX);
            float dy = curX * (28.0f - curZ) - curY;
            float dz = curX * curY - (8.0f / 3.0f) * curZ;

            curX += dx * dt;
            curY += dy * dt;
            curZ += dz * dt;

            int ptX = cx + (int)(curX * 3.5f);
            int ptY = cy + (int)((curZ - 25.0f) * 2.2f);
            trajPoints.push_back({ ptX, ptY });
        }
        px = curX; py = curY; pz = curZ;

        d.lines(trajPoints, { .color = THEME_COLOR, .thickness = 2 });

        // Overlay Drive & Bitcrush Geometric Rings
        int crushR = (int)(20.0f + studio.chaos.crush.value * 0.6f);
        d.circle({ cx, cy }, crushR, { .color = Color { 255, 200, 50, (uint8_t)(100 + drvVal * 155) } });

        std::string statusStr = "Morph: " + std::to_string((int)studio.chaos.vcoMorph.value) + "%" +
                                " | Drive: " + std::to_string((int)studio.chaos.drive.value) + "%" +
                                " | Crush: " + std::to_string((int)studio.chaos.crush.value) + "%" +
                                " | FM: " + std::to_string((int)studio.chaos.fmDepth.value) + "%";
        d.text({ graphX + 12, graphY + graphH - 18 }, statusStr, 8, { .color = Color { 240, 200, 210, 255 }, .font = &PoppinsLight_8 });
    }
};
