#pragma once

#include "draw/draw.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"
#include "audio/engines/DriftSynth1.h"
#include "audio/engines/ImpactSuper.h"
#include "zicGridImpact/studio.h"
#include "zicGridImpact/gridState.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <iomanip>
#include <sstream>

class Synth1View {
private:
    static constexpr Color THEME_COLOR = { 0, 240, 190, 255 }; // Neon Synth Cyan/Teal
    float animTime = 0.0f;

public:
    void updateEncoders() {
        if (gridState.isShiftPressed) {
            const char* engineName = (studio.synth1EngineIdx == 0) ? "DriftSynth1" : "ImpactSuper";
            gridState.setEncoder(0, "Engine", (float)studio.synth1EngineIdx, 0.0f, 1.0f, 1.0f, engineName, THEME_COLOR);
            for (int i = 1; i < 12; i++) {
                gridState.setEncoder(i, "", 0.0f, 0.0f, 0.0f, 0.0f, "", THEME_COLOR);
            }
            return;
        }

        if (studio.synth1EngineIdx == 0) {
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
        } else {
            gridState.setEncoderParam(0, studio.impactSuper.pitch, THEME_COLOR);
            gridState.setEncoderParam(1, studio.impactSuper.detune, THEME_COLOR);
            gridState.setEncoderParam(2, studio.impactSuper.subOct, THEME_COLOR);
            gridState.setEncoderParam(3, studio.impactSuper.waveform, THEME_COLOR);
            gridState.setEncoderParam(4, studio.impactSuper.cutoff, THEME_COLOR);
            gridState.setEncoderParam(5, studio.impactSuper.resonance, THEME_COLOR);
            gridState.setEncoderParam(6, studio.impactSuper.envAmt, THEME_COLOR);
            gridState.setEncoderParam(7, studio.impactSuper.release, THEME_COLOR);

            static const char* superModTypeStrings[] = {
                "ENV Cutoff", "ENV Pitch", "ENV Detune", "ENV Wave",
                "LFO Tri Cut", "LFO Tri Pit", "LFO Tri Det", "LFO Tri Wave", "LFO Tri Lvl",
                "LFO Saw Cut", "LFO Saw Pit", "LFO Saw Det", "LFO Saw Wave",
                "LFO S&H Cut", "LFO S&H Pit", "LFO S&H Det"
            };
            int superModIdx = std::clamp((int)std::round(studio.impactSuper.modType.value), 0, 15);
            gridState.setEncoder(8, "Mod Type", studio.impactSuper.modType.value, 0.0f, 15.0f, 1.0f, superModTypeStrings[superModIdx], THEME_COLOR);
            gridState.setEncoderParam(9, studio.impactSuper.modDepth, THEME_COLOR);
            gridState.setEncoderParam(10, studio.impactSuper.modSpeed, THEME_COLOR);
            gridState.setEncoderParam(11, studio.impactSuper.delaySend, THEME_COLOR);
        }
    }

    void handleEncoder(int idx, int delta) {
        if (gridState.isShiftPressed) {
            if (idx == 0) {
                int count = 2; // DriftSynth1 (0) & ImpactSuper (1)
                studio.synth1EngineIdx = (studio.synth1EngineIdx + delta) % count;
                if (studio.synth1EngineIdx < 0) studio.synth1EngineIdx += count;
            }
            updateEncoders();
            return;
        }

        float step = gridState.encoders[idx].step;
        float change = delta * step;

        if (studio.synth1EngineIdx == 0) {
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
        } else {
            switch (idx) {
                case 0: studio.impactSuper.pitch.set(studio.impactSuper.pitch.value + change); break;
                case 1: studio.impactSuper.detune.set(studio.impactSuper.detune.value + change); break;
                case 2: studio.impactSuper.subOct.set(studio.impactSuper.subOct.value + change); break;
                case 3: studio.impactSuper.waveform.set(studio.impactSuper.waveform.value + change); break;
                case 4: studio.impactSuper.cutoff.set(studio.impactSuper.cutoff.value + change); break;
                case 5: studio.impactSuper.resonance.set(studio.impactSuper.resonance.value + change); break;
                case 6: studio.impactSuper.envAmt.set(studio.impactSuper.envAmt.value + change); break;
                case 7: studio.impactSuper.release.set(studio.impactSuper.release.value + change); break;
                case 8: studio.impactSuper.modType.set(studio.impactSuper.modType.value + change); break;
                case 9: studio.impactSuper.modDepth.set(studio.impactSuper.modDepth.value + change); break;
                case 10: studio.impactSuper.modSpeed.set(studio.impactSuper.modSpeed.value + change); break;
                case 11: studio.impactSuper.delaySend.set(studio.impactSuper.delaySend.value + change); break;
            }
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

        const char* titleText = "DRIFT SYNTH 1 ENGINE";
        if (gridState.isShiftPressed) {
            titleText = "SELECT ENGINE [SHIFT]";
        } else if (studio.synth1EngineIdx == 1) {
            titleText = "IMPACT SUPER ENGINE";
        }
        d.text({ graphX + 12, graphY + 8 }, titleText, 12, { .color = THEME_COLOR, .font = &PoppinsLight_12 });

        if (studio.synth1EngineIdx == 1) {
            renderImpactSuperCanvas(d, graphX, graphY, graphW, graphH);
            return;
        }

        renderDriftSynth1Canvas(d, graphX, graphY, graphW, graphH);
    }

private:
    void renderImpactSuperCanvas(Draw& d, int graphX, int graphY, int graphW, int graphH) {
        int cx = graphX + graphW / 2;
        int cy = graphY + (graphH / 2) - 8;
        int halfH = std::clamp((int)(graphH * 0.22f), 22, 55);
        int halfW = (int)(halfH * (28.0f / 22.0f));

        float detuneVal = studio.impactSuper.detune.value * 0.01f;
        float cutVal = studio.impactSuper.cutoff.value;
        float resVal = studio.impactSuper.resonance.value;
        float envAmtVal = studio.impactSuper.envAmt.value * 0.01f;
        float waveMorph = studio.impactSuper.waveform.value * 0.01f;
        float modD = studio.impactSuper.modDepth.value * 0.01f;
        float modS = studio.impactSuper.modSpeed.value * 0.01f;
        float dlyAmt = studio.impactSuper.delaySend.value * 0.01f;

        float relMs = std::clamp(studio.impactSuper.release.value, 10.0f, 3000.0f);
        float sDecayRate = 12.0f / (relMs + 40.0f);
        gridState.synth1PulseLevel = std::max(0.0f, gridState.synth1PulseLevel - sDecayRate);

        int numTraces = 5 + (int)(detuneVal * 4.0f);
        for (int i = 0; i < numTraces; i++) {
            float phaseOffset = (i - numTraces / 2.0f) * (0.05f + detuneVal * 0.25f);
            std::vector<Point> trace;
            int innerW = halfW * 2;
            int startX = cx - halfW;

            for (int px = 0; px <= innerW; px += 3) {
                float normX = (float)px / (float)innerW;
                float ph = std::fmod(normX * (2.0f + waveMorph * 2.0f) + animTime * (0.5f + i * 0.1f) + phaseOffset, 1.0f);
                float saw = 2.0f * ph - 1.0f;
                float sq = (ph < 0.5f) ? 0.75f : -0.75f;
                float wave = (1.0f - waveMorph) * saw + waveMorph * sq;

                int py = cy + (int)(wave * halfH * 0.7f);
                trace.push_back({ startX + px, py });
            }

            uint8_t alpha = (uint8_t)(160 - std::abs(i - numTraces / 2) * 20);
            if (gridState.synth1PulseLevel > 0.01f) {
                alpha = (uint8_t)std::min(255.0f, alpha + gridState.synth1PulseLevel * 90.0f);
            }
            Color traceCol = (i == numTraces / 2) ? THEME_COLOR : Color { 0, 210, 255, alpha };
            d.lines(trace, { .color = traceCol, .thickness = 1 });
        }

        // Holographic Filter Curve
        int innerW = graphW - 12;
        float modulatedCut = std::clamp(cutVal + (gridState.synth1PulseLevel * envAmtVal * 0.45f), 0.02f, 0.98f);
        int cutX = graphX + 6 + (int)(modulatedCut * innerW);
        int baseY = graphY + graphH - 10;
        int passbandH = 14 + (int)(gridState.synth1PulseLevel * envAmtVal * 6.0f);

        std::vector<Point> svfPoints;
        for (int gx = graphX + 6; gx <= graphX + graphW - 6; gx += 4) {
            float freqNorm = (float)(gx - (graphX + 6)) / (float)innerW;
            float dist = freqNorm - modulatedCut;
            float lpResp = 1.0f / (1.0f + std::pow(freqNorm / std::max(0.04f, modulatedCut), 4.0f));
            float bpResp = std::exp(-dist * dist * (25.0f + resVal * 50.0f));
            float totalResp = lpResp + bpResp * (resVal * 2.2f);
            int drawH = std::clamp((int)(totalResp * passbandH), 0, graphH - 20);
            svfPoints.push_back({ gx, baseY - drawH });
        }

        if (svfPoints.size() >= 2) {
            uint8_t fillAlpha = (uint8_t)(25 + gridState.synth1PulseLevel * envAmtVal * 35.0f);
            std::vector<Point> svfPoly = svfPoints;
            svfPoly.push_back({ graphX + graphW - 6, baseY });
            svfPoly.push_back({ graphX + 6, baseY });
            d.filledPolygon(svfPoly, { .color = { 0, 255, 220, fillAlpha } });
            d.lines(svfPoints, { .color = THEME_COLOR, .thickness = 1 });
        }

        if (resVal > 0.01f) {
            d.circle({ cutX, baseY - passbandH }, (int)(4 + resVal * 6.0f), { .color = { 0, 255, 220, 200 } });
        }

        // Sub & Delay Send Indicator
        if (dlyAmt > 0.01f) {
            int numRipples = (dlyAmt > 0.5f) ? 2 : 1;
            for (int r = 1; r <= numRipples; r++) {
                uint8_t rippleAlpha = (uint8_t)(dlyAmt * (120.0f / r));
                d.rect({ cx - halfW - r * 4, cy - halfH - r * 2 }, { halfW * 2 + r * 8, halfH * 2 + r * 4 }, { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, rippleAlpha } });
            }
        }
    }

    void renderDriftSynth1Canvas(Draw& d, int graphX, int graphY, int graphW, int graphH) {
        int cx = graphX + graphW / 2;
        int cy = graphY + (graphH / 2) - 8;
        int halfH = std::clamp((int)(graphH * 0.22f), 22, 55);
        int halfW = (int)(halfH * (28.0f / 22.0f));

        float pitchMidi = studio.synth1.pitch.value;
        float wf = studio.synth1.waveform.value;
        float cutVal = studio.synth1.cutoff.value;
        float resVal = studio.synth1.resonance.value;

        float modD = studio.synth1.modDepth.value * 0.01f;
        float modS = studio.synth1.modSpeed.value * 0.01f;
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

        float levelMod = (currentRoute.dest == DriftSynth1::DST_LEVEL) ? std::clamp(1.0f + modAmount * 0.5f, 0.1f, 1.8f) : 1.0f;
        uint8_t lineAlpha = (uint8_t)(std::clamp(255.0f * (1.0f - noiseFactor * 0.85f) * levelMod, 10.0f, 255.0f));
        uint8_t fillAlpha = (uint8_t)(std::clamp(60.0f * (1.0f - noiseFactor) * levelMod, 5.0f, 180.0f));

        if (lineAlpha > 15) {
            d.filledPolygon(morphedShape, { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, fillAlpha } });
            d.lines(morphedShape, { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, lineAlpha }, .thickness = 1 });
            d.line(morphedShape.back(), morphedShape.front(), { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, lineAlpha }, .thickness = 1 });
        }

        float envModAmt = studio.synth1.envAmt.value;
        float filterModOffset = (currentRoute.dest == DriftSynth1::DST_FILTER) ? modAmount * 0.35f : 0.0f;
        float modulatedCut = std::clamp(cutVal + (gridState.synth1PulseLevel * envModAmt * 0.45f) + filterModOffset, 0.02f, 0.98f);
        cutX = graphX + 6 + (int)(modulatedCut * innerW);
        int baseY = graphY + graphH - 10;
        int passbandH = 14 + (int)(gridState.synth1PulseLevel * envModAmt * 6.0f);
        float fMorph = studio.synth1.filterMorph.value;

        std::vector<Point> svfPoints;
        for (int gx = graphX + 6; gx <= graphX + graphW - 6; gx += 4) {
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

            float totalResp = baseCurve + bpResp * (resVal * 2.2f);
            int drawH = std::clamp((int)(totalResp * passbandH), 0, graphH - 20);
            svfPoints.push_back({ gx, baseY - drawH });
        }

        if (svfPoints.size() >= 2) {
            uint8_t fAlpha = (uint8_t)(25 + gridState.synth1PulseLevel * envModAmt * 35.0f);
            std::vector<Point> svfPoly = svfPoints;
            svfPoly.push_back({ graphX + graphW - 6, baseY });
            svfPoly.push_back({ graphX + 6, baseY });
            d.filledPolygon(svfPoly, { .color = { 0, 255, 220, fAlpha } });
        }

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
        d.lines(pitchWave, { .color = { THEME_COLOR.r, THEME_COLOR.g, THEME_COLOR.b, 255 } });
    }
};
