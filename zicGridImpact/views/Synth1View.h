#pragma once

#include "draw/draw.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"
#include "audio/engines/DriftSynth1.h"
#include "zicGridImpact/studio.h"
#include "zicGridImpact/gridState.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <iomanip>
#include <sstream>

class Synth1View {
private:
    static constexpr Color THEME_COLOR = { 0, 240, 190, 255 }; // Neon Synth 1 Cyan/Teal
    float animTime = 0.0f;

public:
    void updateEncoders() {
        gridState.setEncoderParam(0, studio.synth1.pitch, THEME_COLOR);
        gridState.setEncoderParam(1, studio.synth1.waveform, THEME_COLOR);
        gridState.setEncoderParam(2, studio.synth1.cutoff, THEME_COLOR);
        gridState.setEncoderParam(3, studio.synth1.resonance, THEME_COLOR);
        gridState.setEncoderParam(4, studio.synth1.release, THEME_COLOR);
        gridState.setEncoderParam(5, studio.synth1.envAmt, THEME_COLOR);
        gridState.setEncoderParam(6, studio.synth1.filterMorph, THEME_COLOR);
        gridState.setEncoderParam(7, studio.synth1.delaySend, THEME_COLOR);

        static const char* modTypeStrings[] = {
            "ENV Cutoff", "ENV Pitch", "ENV Wave", "ENV Crsh/FM",
            "LFO Tri Cut", "LFO Tri Pit", "LFO Tri Wave", "LFO Tri Lvl", "LFO Tri CFM",
            "LFO Saw Cut", "LFO Saw Pit", "LFO Saw Wave", "LFO Saw CFM",
            "LFO S&H Cut", "LFO S&H Pit", "LFO S&H CFM"
        };
        int modIdx = std::clamp((int)std::round(studio.synth1.modType.value), 0, 15);
        gridState.setEncoder(8, "Mod Type", studio.synth1.modType.value, 0.0f, 15.0f, 1.0f, modTypeStrings[modIdx], THEME_COLOR);
        gridState.setEncoderParam(9, studio.synth1.modDepth, THEME_COLOR);
        gridState.setEncoderParam(10, studio.synth1.modSpeed, THEME_COLOR);
        gridState.setEncoderParam(11, studio.synth1.crushFm, THEME_COLOR);
    }

    void handleEncoder(int idx, int delta) {
        float step = gridState.encoders[idx].step;
        float change = delta * step;
        switch (idx) {
            case 0: studio.synth1.pitch.set(studio.synth1.pitch.value + change); break;
            case 1: studio.synth1.waveform.set(studio.synth1.waveform.value + change); break;
            case 2: studio.synth1.cutoff.set(studio.synth1.cutoff.value + change); break;
            case 3: studio.synth1.resonance.set(studio.synth1.resonance.value + change); break;
            case 4: studio.synth1.release.set(studio.synth1.release.value + change); break;
            case 5: studio.synth1.envAmt.set(studio.synth1.envAmt.value + change); break;
            case 6: studio.synth1.filterMorph.set(studio.synth1.filterMorph.value + change); break;
            case 7: studio.synth1.delaySend.set(studio.synth1.delaySend.value + change); break;
            case 8: studio.synth1.modType.set(studio.synth1.modType.value + change); break;
            case 9: studio.synth1.modDepth.set(studio.synth1.modDepth.value + change); break;
            case 10: studio.synth1.modSpeed.set(studio.synth1.modSpeed.value + change); break;
            case 11: studio.synth1.crushFm.set(studio.synth1.crushFm.value + change); break;
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

        d.text({ graphX + 12, graphY + 8 }, "DRIFT SYNTH 1 ENGINE", 12, { .color = THEME_COLOR, .font = &PoppinsLight_12 });

        int cx = graphX + graphW / 2;
        int cy = graphY + (graphH / 2) - 8;
        int halfW = (graphW * 28) / 300;
        int halfH = (graphH * 22) / 100;
        if (halfW < 28) halfW = 28;
        if (halfH < 22) halfH = 22;

        float pitchMidi = studio.synth1.pitch.value; // 24 .. 72
        float wf = studio.synth1.waveform.value;     // 0.0 .. 1.0
        float cutVal = studio.synth1.cutoff.value;   // 0.02 .. 0.98
        float resVal = studio.synth1.resonance.value; // 0.0 .. 0.95

        float modD = studio.synth1.modDepth.value * 0.01f; // -1.0 .. +1.0
        float modS = studio.synth1.modSpeed.value * 0.01f; // 0.0 .. 1.0
        float lfoHz = 0.05f + (modS * modS * 39.95f);
        float lfoPhase = std::fmod(animTime * lfoHz * 0.5f, 1.0f);

        int routeIdx = std::clamp((int)std::round(studio.synth1.modType.value), 0, DriftSynth1::TOTAL_MOD_TYPES - 1);
        auto currentRoute = DriftSynth1::modMatrix[routeIdx];

        float lfoVal = 0.0f;
        switch (currentRoute.source) {
        case DriftSynth1::SRC_ENV:
            lfoVal = gridState.synth1PulseLevel;
            break;
        case DriftSynth1::SRC_LFO_TRI:
            lfoVal = (lfoPhase < 0.5f) ? (4.0f * lfoPhase - 1.0f) : (3.0f - 4.0f * lfoPhase);
            break;
        case DriftSynth1::SRC_LFO_SAW:
            lfoVal = 2.0f * lfoPhase - 1.0f;
            break;
        case DriftSynth1::SRC_LFO_SH: {
            float stepIdx = std::floor(lfoPhase * 10.0f);
            lfoVal = std::sin(stepIdx * 17.13f + 1.5f);
            break;
        }
        }

        float modAmount = lfoVal * modD;

        if (currentRoute.dest == DriftSynth1::DST_MORPH) {
            wf = std::clamp(wf + modAmount * 0.4f, 0.0f, 1.0f);
        }

        int innerW = graphW - 12;
        int cutX = graphX + 6 + (int)(std::clamp(cutVal, 0.02f, 0.98f) * innerW);

        // 2. Central Waveform Core
        Point pBL = { cx - halfW, cy + halfH };
        Point pBR = { cx + halfW, cy + halfH };
        Point pTL, pTR;

        float shapeMorph = std::min(wf, 0.666f) / 0.666f;

        if (shapeMorph <= 0.5f) {
            float t = shapeMorph / 0.5f;
            int topX = cx + (int)(t * halfW);
            pTL = { topX, cy - halfH };
            pTR = { topX, cy - halfH };
        } else {
            float t = (shapeMorph - 0.5f) / 0.5f;
            int tlX = (cx + halfW) - (int)(t * 2.0f * halfW);
            pTR = { cx + halfW, cy - halfH };
            pTL = { tlX, cy - halfH };
        }

        std::vector<Point> baseShape;
        if (std::abs(pTL.x - pTR.x) <= 1) {
            baseShape = { pBL, pTR, pBR };
        } else {
            baseShape = { pBL, pTL, pTR, pBR };
        }

        float noiseFactor = (wf > 0.666f) ? std::clamp((wf - 0.666f) / 0.334f, 0.0f, 1.0f) : 0.0f;

        float baseJitterX = 1.6f + noiseFactor * 5.4f;
        float baseJitterY = 1.4f + noiseFactor * 4.6f;

        std::vector<Point> morphedShape;
        for (size_t i = 0; i < baseShape.size(); ++i) {
            float noiseSeed = animTime * 15.0f + i * 2.3f;
            int jitterX = (int)(std::sin(noiseSeed * 3.7f) * baseJitterX);
            int jitterY = (int)(std::cos(noiseSeed * 4.1f) * baseJitterY);
            morphedShape.push_back({ baseShape[i].x + jitterX, baseShape[i].y + jitterY });
        }

        // Synth 1 Trigger Pulse Decay & Expanding Waveform-Shaped Shockwave Echoes
        float sDecayRate = 12.0f / (std::clamp(studio.synth1.release.value, 10.0f, 2000.0f) + 40.0f);
        gridState.synth1PulseLevel = std::max(0.0f, gridState.synth1PulseLevel - sDecayRate);

        if (gridState.synth1PulseLevel > 0.01f) {
            for (int r = 0; r < 3; r++) {
                float pFactor = gridState.synth1PulseLevel - (r * 0.22f);
                if (pFactor > 0.0f) {
                    float scale = 1.05f + (1.0f - pFactor) * 0.65f + r * 0.18f;
                    uint8_t pulseAlpha = (uint8_t)(pFactor * 160.0f);

                    std::vector<Point> pulseShape;
                    for (const auto& pt : morphedShape) {
                        int px = cx + (int)((pt.x - cx) * scale);
                        int py = cy + (int)((pt.y - cy) * scale);
                        pulseShape.push_back({ px, py });
                    }

                    d.lines(pulseShape, { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, pulseAlpha }, .thickness = 1 });
                    d.line(pulseShape.back(), pulseShape.front(), { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, pulseAlpha }, .thickness = 1 });
                }
            }
        }

        // Spatial Delay Echo Ghosts
        float dlyAmt = std::clamp(studio.synth1.delaySend.value * 0.01f, 0.0f, 1.0f);
        if (dlyAmt > 0.01f) {
            int ghostCount = (dlyAmt > 0.6f) ? 3 : ((dlyAmt > 0.3f) ? 2 : 1);
            for (int g = ghostCount; g >= 1; g--) {
                float gOffset = g * 14.0f * (0.5f + dlyAmt * 0.7f);
                float gScale = 1.0f - g * 0.12f;
                uint8_t gAlpha = (uint8_t)(dlyAmt * (110.0f / g) * (1.0f - noiseFactor * 0.6f));

                if (gAlpha > 5) {
                    std::vector<Point> ghostShape;
                    for (const auto& pt : morphedShape) {
                        int gx = cx + (int)(gOffset) + (int)((pt.x - cx) * gScale);
                        int gy = cy + (int)(g * 3.0f) + (int)((pt.y - cy) * gScale);
                        ghostShape.push_back({ gx, gy });
                    }

                    d.filledPolygon(ghostShape, { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, (uint8_t)(gAlpha * 0.25f) } });
                    d.lines(ghostShape, { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, gAlpha }, .thickness = 1 });
                    d.line(ghostShape.back(), ghostShape.front(), { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, gAlpha }, .thickness = 1 });
                }
            }
        }

        // Opacity & Level Modulation (DST_LEVEL)
        float levelMod = (currentRoute.dest == DriftSynth1::DST_LEVEL) ? std::clamp(1.0f + modAmount * 0.5f, 0.1f, 1.8f) : 1.0f;
        uint8_t lineAlpha = (uint8_t)(std::clamp(255.0f * (1.0f - noiseFactor * 0.85f) * levelMod, 10.0f, 255.0f));
        uint8_t fillAlpha = (uint8_t)(std::clamp(60.0f * (1.0f - noiseFactor) * levelMod, 5.0f, 180.0f));

        if (lineAlpha > 15) {
            d.filledPolygon(morphedShape, { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, fillAlpha } });
            d.lines(morphedShape, { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, lineAlpha }, .thickness = 1 });
            d.line(morphedShape.back(), morphedShape.front(), { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, lineAlpha }, .thickness = 1 });
        }

        // Dynamic Noise Particle Swarm
        if (noiseFactor > 0.01f) {
            int particleCount = (int)(noiseFactor * 90.0f);
            for (int p = 0; p < particleCount; p++) {
                float pAngle = p * 0.418f + animTime * (1.2f + (p % 5) * 0.4f);
                float pDist = std::fmod((float)(p * 7 + animTime * 35.0f), 32.0f);
                int px = cx + (int)(std::cos(pAngle) * pDist);
                int py = cy + (int)(std::sin(pAngle) * (pDist * 0.7f));

                px = std::clamp(px, graphX + 6, graphX + graphW - 6);
                py = std::clamp(py, graphY + 12, graphY + graphH - 14);

                uint8_t pAlpha = (uint8_t)((100 + (p * 17 + (int)(animTime * 120)) % 155) * noiseFactor);
                Color pCol = (p % 3 == 0) ? Color { 255, 255, 255, pAlpha } : Color { 0, 255, 210, pAlpha };
                d.pixel({ px, py }, pCol);
                if (p % 4 == 0) {
                    d.pixel({ px + 1, py }, Color { pCol.r, pCol.g, pCol.b, (uint8_t)(pAlpha * 0.5f) });
                }
            }
        }

        // Top-Left Rotating LFO Shape & Dotted Target Pointer
        if (std::abs(studio.synth1.modDepth.value) > 1.0f) {
            int lfoCx = graphX + 16;
            int lfoCy = graphY + 16;

            float lfoRadius = 4.5f + std::abs(modD) * 4.5f;
            float spinHz = 0.15f + modS * 1.8f;
            float rotAngle = animTime * spinHz * 6.28318f;

            Color grayCol = { 190, 210, 235, 255 };
            Color dimGrayCol = { 135, 150, 175, 255 };

            std::vector<Point> iconPts;
            if (currentRoute.source == DriftSynth1::SRC_LFO_TRI) {
                for (int i = 0; i < 3; i++) {
                    float a = rotAngle + i * (6.28318f / 3.0f) - 1.5708f;
                    iconPts.push_back({ lfoCx + (int)(std::cos(a) * lfoRadius), lfoCy + (int)(std::sin(a) * lfoRadius) });
                }
                d.lines(iconPts, { .color = grayCol, .thickness = 1 });
                d.line(iconPts.back(), iconPts.front(), { .color = grayCol, .thickness = 1 });
            } else if (currentRoute.source == DriftSynth1::SRC_LFO_SAW) {
                float a0 = rotAngle;
                float a1 = rotAngle + 2.1f;
                float a2 = rotAngle + 4.2f;
                iconPts = {
                    { lfoCx + (int)(std::cos(a0) * lfoRadius * 1.1f), lfoCy + (int)(std::sin(a0) * lfoRadius * 1.1f) },
                    { lfoCx + (int)(std::cos(a1) * lfoRadius * 0.7f), lfoCy + (int)(std::sin(a1) * lfoRadius * 0.7f) },
                    { lfoCx + (int)(std::cos(a2) * lfoRadius * 0.9f), lfoCy + (int)(std::sin(a2) * lfoRadius * 0.9f) }
                };
                d.lines(iconPts, { .color = grayCol, .thickness = 1 });
                d.line(iconPts.back(), iconPts.front(), { .color = grayCol, .thickness = 1 });
            } else if (currentRoute.source == DriftSynth1::SRC_LFO_SH) {
                for (int i = 0; i < 4; i++) {
                    float a = rotAngle + i * (6.28318f / 4.0f);
                    iconPts.push_back({ lfoCx + (int)(std::cos(a) * lfoRadius), lfoCy + (int)(std::sin(a) * lfoRadius) });
                }
                d.lines(iconPts, { .color = grayCol, .thickness = 1 });
                d.line(iconPts.back(), iconPts.front(), { .color = grayCol, .thickness = 1 });
            } else {
                d.circle({ lfoCx, lfoCy }, (int)lfoRadius, { .color = dimGrayCol });
                int dotX = lfoCx + (int)(std::cos(rotAngle) * lfoRadius);
                int dotY = lfoCy + (int)(std::sin(rotAngle) * lfoRadius);
                d.pixel({ dotX, dotY }, grayCol);
            }

            int dstX = cx;
            int dstY = cy;

            if (currentRoute.dest == DriftSynth1::DST_FILTER) {
                dstX = cutX;
                dstY = graphY + graphH - 22;
            } else if (currentRoute.dest == DriftSynth1::DST_PITCH) {
                dstX = cx;
                dstY = graphY + graphH - 10;
            }

            float lineLen = std::hypot(dstX - lfoCx, dstY - lfoCy);
            int dashStep = 5;
            for (float dPos = 0.0f; dPos < lineLen; dPos += dashStep * 2) {
                float t0 = dPos / lineLen;
                float t1 = std::min(lineLen, dPos + dashStep) / lineLen;
                int x0 = lfoCx + (int)((dstX - lfoCx) * t0);
                int y0 = lfoCy + (int)((dstY - lfoCy) * t0);
                int x1 = lfoCx + (int)((dstX - lfoCx) * t1);
                int y1 = lfoCy + (int)((dstY - lfoCy) * t1);
                d.line({ x0, y0 }, { x1, y1 }, { .color = dimGrayCol });
            }

            float pktProgress = std::fmod(animTime * spinHz * 0.8f, 1.0f);
            int px = lfoCx + (int)((dstX - lfoCx) * pktProgress);
            int py = lfoCy + (int)((dstY - lfoCy) * pktProgress);
            Color pktCol = { 255, 255, 255, 255 };
            d.pixel({ px, py }, pktCol);
            d.pixel({ px + 1, py }, pktCol);
        }

        // Holographic SVF Spectral Wave Modulated by Filter Envelope (envAmt) & LFO
        float envModAmt = studio.synth1.envAmt.value;
        float filterModOffset = (currentRoute.dest == DriftSynth1::DST_FILTER) ? modAmount * 0.35f : 0.0f;
        float modulatedCut = std::clamp(cutVal + (gridState.synth1PulseLevel * envModAmt * 0.45f) + filterModOffset, 0.02f, 0.98f);

        cutX = graphX + 6 + (int)(modulatedCut * innerW);
        int baseY = graphY + graphH - 10;
        int passbandH = 14 + (int)(gridState.synth1PulseLevel * envModAmt * 6.0f);

        float fMorph = studio.synth1.filterMorph.value;

        std::vector<Point> svfPoints;
        int stepPx = 4;
        int peakY = baseY - passbandH;

        for (int gx = graphX + 6; gx <= graphX + graphW - 6; gx += stepPx) {
            float freqNorm = (float)(gx - (graphX + 6)) / (float)innerW;
            float dist = freqNorm - modulatedCut;

            float lpResp = 1.0f / (1.0f + std::pow(freqNorm / std::max(0.04f, modulatedCut), 4.0f));
            float hpResp = 1.0f - lpResp;
            float bpResp = std::exp(-dist * dist * (25.0f + resVal * 50.0f));

            float baseCurve = 0.0f;
            if (fMorph < 0.5f) {
                baseCurve = lpResp * (1.0f - fMorph * 2.0f) + bpResp * (fMorph * 2.0f);
            } else {
                baseCurve = bpResp * (1.0f - (fMorph - 0.5f) * 2.0f) + hpResp * ((fMorph - 0.5f) * 2.0f);
            }

            float resonancePeak = bpResp * (resVal * 2.2f);
            float totalResp = baseCurve + resonancePeak;

            int drawH = (int)(totalResp * passbandH);
            drawH = std::clamp(drawH, 0, graphH - 20);
            int sy = baseY - drawH;
            svfPoints.push_back({ gx, sy });

            if (std::abs(gx - cutX) <= stepPx) {
                peakY = sy;
            }
        }

        if (svfPoints.size() >= 2) {
            uint8_t fillAlpha = (uint8_t)(25 + gridState.synth1PulseLevel * envModAmt * 35.0f);
            std::vector<Point> svfPoly = svfPoints;
            svfPoly.push_back({ graphX + graphW - 6, baseY });
            svfPoly.push_back({ graphX + 6, baseY });
            d.filledPolygon(svfPoly, { .color = { 0, 255, 220, fillAlpha } });
        }

        // Resonance Peak Laser Beam & Pulsing Glow Halo
        if (resVal > 0.01f) {
            for (int h = 0; h < 2; h++) {
                float haloPulse = std::sin(animTime * 8.0f + h * 1.5f) * 1.5f;
                int r = (int)(4 + h * 5 + resVal * 6.0f + haloPulse);
                uint8_t hAlpha = (uint8_t)(std::clamp(180.0f * resVal - h * 50.0f, 0.0f, 255.0f));
                d.circle({ cutX, peakY }, r, { .color = { 0, 255, 220, hAlpha } });
            }
        }

        // Pitch & Frequency Ribbon + Readout Overlay + LFO Pitch Modulation
        float pitchModOffset = (currentRoute.dest == DriftSynth1::DST_PITCH) ? modAmount * 12.0f : 0.0f;
        float pitchHz = 440.0f * std::pow(2.0f, (pitchMidi + pitchModOffset - 69.0f) / 12.0f);

        int freqY = graphY + graphH - 10;
        std::vector<Point> pitchWave;
        float cycScale = (pitchHz / 110.0f) * 0.15f;
        for (int gx = 0; gx < innerW; gx++) {
            float t = (float)gx / (float)innerW;
            float wave = std::sin(t * (cycScale * 25.0f) + animTime * 4.0f) * 3.0f;
            pitchWave.push_back({ graphX + 6 + gx, freqY + (int)wave });
        }

        // Trailing Horizon Delay Echo Ripples
        if (dlyAmt > 0.01f) {
            int numRipples = (dlyAmt > 0.6f) ? 3 : ((dlyAmt > 0.3f) ? 2 : 1);
            for (int r = 1; r <= numRipples; r++) {
                float phaseOffset = r * 0.55f;
                uint8_t rippleAlpha = (uint8_t)(dlyAmt * (140.0f / (r * 1.25f)));
                int rippleY = freqY - r * 2;

                std::vector<Point> rippleWave;
                for (int gx = 0; gx < innerW; gx++) {
                    float t = (float)gx / (float)innerW;
                    float wave = std::sin(t * (cycScale * 25.0f) + animTime * 4.0f - phaseOffset) * (3.0f - r * 0.6f);
                    rippleWave.push_back({ graphX + 6 + gx, rippleY + (int)wave });
                }
                d.lines(rippleWave, { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, rippleAlpha }, .thickness = 1 });
            }
        }

        d.lines(pitchWave, { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, 255 } });
    }
};
