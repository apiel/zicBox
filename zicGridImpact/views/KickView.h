#pragma once

#include "draw/draw.h"
#include "zicGridImpact/gridState.h"
#include "zicGridImpact/studio.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <vector>

class KickView {
private:
    static constexpr Color THEME_COLOR = { 0, 195, 255, 255 }; // Electric Kick Blue
    float animTime = 0.0f;
    float kickPulseLevel = 0.0f;

public:
    void updateEncoders() {
        gridState.setEncoderParam(0, studio.kick.baseFreq, THEME_COLOR);
        gridState.setEncoderParam(1, studio.kick.clickAmt, THEME_COLOR);
        gridState.setEncoderParam(2, studio.kick.duration, THEME_COLOR);
        gridState.setEncoderParam(3, studio.kick.vcoMorph, THEME_COLOR);

        gridState.setEncoderParam(4, studio.kick.fmDepth, THEME_COLOR);
        gridState.setEncoderParam(5, studio.kick.drive, THEME_COLOR);
        gridState.setEncoderParam(6, studio.kick.rumbleAmt, THEME_COLOR);
        gridState.setEncoderParam(7, studio.kick.rumbleGap, THEME_COLOR);

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
        animTime += 0.05f;

        int graphX = x;
        int graphY = y;
        int graphW = w;
        int graphH = h;

        // Solid graph box background + vibrant frame outline from zicPixelDrift
        d.filledRect({ graphX, graphY }, { graphW, graphH }, { .color = { 12, 14, 20, 255 } });
        d.rect({ graphX, graphY }, { graphW, graphH }, { .color = THEME_COLOR });

        d.text({ graphX + 12, graphY + 8 }, "DRIFT KICK SYNTHESIZER ENGINE", 12, { .color = THEME_COLOR, .font = &PoppinsLight_12 });

        int cx = graphX + graphW / 2;
        int cy = graphY + (graphH / 2) - 10;
        int halfW = 75;
        int halfH = 55;

        float morphVal = std::clamp(studio.kick.vcoMorph.value / 100.0f, 0.0f, 1.0f);
        float clickAmt = studio.kick.clickAmt.value;
        float durMs = studio.kick.duration.value;
        float freqHz = studio.kick.baseFreq.value;

        // Kick Trigger Pulse Decay & Expanding Shockwaves
        float decayRate = 12.0f / (std::clamp(durMs, 50.0f, 1500.0f) + 50.0f);
        kickPulseLevel = std::max(0.0f, kickPulseLevel - decayRate);

        if (kickPulseLevel > 0.01f) {
            for (int r = 0; r < 3; r++) {
                float pFactor = kickPulseLevel - (r * 0.22f);
                if (pFactor > 0.0f) {
                    int radius = (int)(50.0f + (1.0f - pFactor) * 60.0f + r * 10);
                    uint8_t alpha = (uint8_t)(pFactor * 130.0f);
                    d.circle({ cx, cy }, radius, { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, alpha } });
                }
            }
        }

        // VCO Morph Geometry
        Point pBL = { cx - halfW, cy + halfH };
        Point pBR = { cx + halfW, cy + halfH };
        Point pTL, pTR;

        if (morphVal <= 0.5f) {
            float t = morphVal / 0.5f;
            int topX = cx + (int)(t * halfW);
            pTL = { topX, cy - halfH };
            pTR = { topX, cy - halfH };
        } else {
            float t = (morphVal - 0.5f) / 0.5f;
            int tlX = (cx + halfW) - (int)(t * 2.0f * halfW);
            pTR = { cx + halfW, cy - halfH };
            pTL = { tlX, cy - halfH };
        }

        std::vector<Point> morphShape;
        if (std::abs(pTL.x - pTR.x) <= 1) {
            morphShape = { pBL, pTR, pBR };
        } else {
            morphShape = { pBL, pTL, pTR, pBR };
        }

        // Rumble Sub-Bass Echo Ghost
        float rAmt = std::clamp(studio.kick.rumbleAmt.value / 100.0f, 0.0f, 1.0f);
        float rGapMs = studio.kick.rumbleGap.value;

        if (rAmt > 0.01f) {
            int rOffsetX = (int)((rGapMs / 400.0f) * 60.0f);
            int rOffsetY = (int)((rGapMs / 400.0f) * 12.0f);
            uint8_t rAlpha = (uint8_t)(std::max(0.2f, rAmt) * 140.0f);

            std::vector<Point> rumbleGhostShape;
            for (const auto& pt : morphShape) {
                rumbleGhostShape.push_back({ pt.x + rOffsetX, pt.y + rOffsetY });
            }

            d.filledPolygon(rumbleGhostShape, { .color = { 0, 180, 255, (uint8_t)(rAlpha * 0.35f) } });
            d.lines(rumbleGhostShape, { .color = { 0, 220, 255, rAlpha }, .thickness = 1 });
            d.line(rumbleGhostShape.back(), rumbleGhostShape.front(), { .color = { 0, 220, 255, rAlpha }, .thickness = 1 });
        }

        // FM Modulator Orbiting Shell
        float fmVal = std::clamp(studio.kick.fmDepth.value / 100.0f, 0.0f, 1.0f);
        if (fmVal > 0.01f) {
            float rotAngle = animTime * (1.0f + fmVal * 8.0f);
            int numShellPts = 5;
            std::vector<Point> modShell;
            for (int i = 0; i < numShellPts; i++) {
                float a = rotAngle + i * (6.28318f / numShellPts);
                float radiusW = (halfW + 16.0f) + std::sin(a * 3.0f + animTime * 4.0f) * (fmVal * 20.0f);
                float radiusH = (halfH + 16.0f) + std::cos(a * 2.0f + animTime * 3.0f) * (fmVal * 16.0f);
                int mx = cx + (int)(std::cos(a) * radiusW);
                int my = cy + (int)(std::sin(a) * radiusH);
                modShell.push_back({ mx, my });
            }
            uint8_t shellAlpha = (uint8_t)(80 + fmVal * 165.0f);
            d.lines(modShell, { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, shellAlpha }, .thickness = 1 });
            d.line(modShell.back(), modShell.front(), { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, shellAlpha }, .thickness = 1 });
        }

        // Drive Overdrive Saturation Stroke & Fill
        float drv = std::clamp(studio.kick.drive.value / 100.0f, 0.0f, 1.0f);
        Color shapeStroke = THEME_COLOR;
        if (drv > 0.01f) {
            shapeStroke = Color {
                (uint8_t)(THEME_COLOR.r * (1.0f - drv) + 255 * drv),
                (uint8_t)(THEME_COLOR.g * (1.0f - drv) + 120 * drv),
                (uint8_t)(THEME_COLOR.b * (1.0f - drv) + 40 * drv),
                255
            };
        }
        int strokeThickness = (drv > 0.35f) ? 2 : 1;
        uint8_t fillAlpha = (uint8_t)(60 + drv * 60.0f);

        d.filledPolygon(morphShape, { .color = { shapeStroke.r, shapeStroke.g, shapeStroke.b, fillAlpha } });
        d.lines(morphShape, { .color = shapeStroke, .thickness = strokeThickness });
        d.line(pBR, pBL, { .color = shapeStroke, .thickness = strokeThickness });

        // Click Noise Dot Swarm
        int dotCount = (int)(clickAmt * 0.85f);
        for (int i = 0; i < dotCount; i++) {
            float angle = i * 0.488f + animTime * (0.6f + (i % 4) * 0.3f);
            float dist = 24.0f + std::fmod((float)(i * 9 + animTime * 20.0f), 60.0f);
            int dotX = cx + (int)(std::cos(angle) * dist);
            int dotY = cy + (int)(std::sin(angle) * dist);
            dotX = std::clamp(dotX, graphX + 8, graphX + graphW - 8);
            dotY = std::clamp(dotY, graphY + 16, graphY + graphH - 24);
            uint8_t dotAlpha = (uint8_t)(110 + (i * 13 + (int)(animTime * 100)) % 145);
            d.pixel({ dotX, dotY }, Color { 255, 245, 170, dotAlpha });
        }

        // Frequency Sine Ribbon & Overlay
        int freqY = graphY + graphH - 18;
        std::vector<Point> freqWave;
        int innerW = graphW - 24;
        for (int gx = 0; gx < innerW; gx += 2) {
            float t = (float)gx / (float)innerW;
            float wave = std::sin(t * (freqHz * 0.22f) + animTime * (freqHz * 0.07f)) * (5.5f + (freqHz * 0.025f));
            freqWave.push_back({ graphX + 12 + gx, freqY + (int)wave });
        }
        d.lines(freqWave, { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, 255 } });

        std::stringstream ssF;
        ssF << "FREQ: " << std::fixed << std::setprecision(1) << freqHz << " Hz";
        d.text({ graphX + 12, graphY + graphH - 20 }, ssF.str(), 8, { .color = Color { 230, 240, 255, 255 }, .font = &PoppinsLight_8 });
    }
};
