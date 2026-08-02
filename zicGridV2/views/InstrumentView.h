#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <mutex>
#include <string>
#include <vector>

#include "draw/draw.h"
#include "zicGridV2/ViewManager.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/uiComponents.h"

class InstrumentView : public View {
private:
    int baseNote = 48;
    int currentPage = 0;
    float notePulseLevel = 0.0f;
    uint8_t lastTriggeredNote = 0;
    int lastSeqStep = -1;

    // Smooth state for Panel B waveform shape detector
    float smoothedSineWeight = 1.0f;
    float smoothedTriWeight = 0.0f;
    float smoothedSawWeight = 0.0f;
    float smoothedSqWeight = 0.0f;
    float smoothedNoiseWeight = 0.0f;

public:
    InstrumentView() : View("INSTRUMENT & SYNTH") {}

    int getTotalPages() const
    {
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];
        if (t && t->engine) {
            size_t paramCount = t->engine->getParamCount();
            if (paramCount == 0) return 1;
            return (int)((paramCount + TOTAL_ENCODERS - 1) / TOTAL_ENCODERS);
        }
        return 1;
    }

    std::pair<int, int> getViewPageInfo() const override
    {
        int totalPages = getTotalPages();
        int pageIdx = std::min(currentPage + 1, totalPages);
        return { pageIdx, totalPages };
    }

    void onTrackSelect(int trk, bool isSameTrack) override
    {
        if (isSameTrack) {
            int totalPages = getTotalPages();
            if (totalPages > 1) {
                currentPage = (currentPage + 1) % totalPages;
            }
        } else {
            currentPage = 0;
        }
    }

    void changePage(int delta) override
    {
        int totalPages = getTotalPages();
        if (totalPages > 1) {
            currentPage = (currentPage + delta + totalPages) % totalPages;
        }
    }

    void onActivate() override
    {
        updatePadLeds();
        updateEncoderLabels();
    }

    void updatePadLeds() override
    {
        int octave = gridState.utility.currentOctave;
        baseNote = 12 + octave * 12;

        int activeTrk = studio.selTrack;
        Color theme = studio.tracks[activeTrk]->themeColor;

        for (int r = 0; r < PAD_ROWS; ++r) {
            for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
                int noteOffset = (3 - r) * DYNAMIC_PAD_COLS + c;
                int note = baseNote + noteOffset;

                auto& pad = gridState.pads[c][r];
                pad.note = (uint8_t)note;
                pad.selected = false;

                bool isRoot = (note % 12 == 0);
                bool isAccidental = (note % 12 == 1 || note % 12 == 3 || note % 12 == 6 || note % 12 == 8 || note % 12 == 10);

                if (pad.pressed) {
                    pad.color = { 255, 255, 255, 255 };
                } else if (isRoot) {
                    pad.color = { 255, 200, 50, 255 };
                } else if (isAccidental) {
                    pad.color = { 60, 70, 100, 255 };
                } else {
                    pad.color = theme;
                }

                pad.label = getNoteName(note);
            }
        }
    }

    void updateEncoderLabels() override
    {
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];
        Color c = t->themeColor;

        if (gridState.utility.shiftActive) {
            gridState.setEncoder(0, "Synth", t->currentEngineIdx, 0, ENGINE_REGISTRY_COUNT - 1, 1, engineRegistry[t->currentEngineIdx].name, c);
            gridState.setEncoder(1, "Volume", (int)(t->volume * 100.0f), 0, 100, 1, nullptr, c, "%");

            for (int i = 2; i < TOTAL_ENCODERS; ++i) {
                gridState.setEncoderParam(i, Param{}, c);
            }
            return;
        }

        int totalPages = getTotalPages();
        if (currentPage >= totalPages) {
            currentPage = 0;
        }

        if (t && t->engine) {
            size_t paramCount = t->engine->getParamCount();
            auto* params = t->engine->getParams();
            for (int i = 0; i < TOTAL_ENCODERS; ++i) {
                int actualParamIdx = currentPage * TOTAL_ENCODERS + i;
                if ((size_t)actualParamIdx < paramCount && params) {
                    gridState.setEncoderParam(i, params[actualParamIdx], c);
                } else {
                    gridState.setEncoderParam(i, Param{}, c);
                }
            }
        } else {
            for (int i = 0; i < TOTAL_ENCODERS; ++i) {
                gridState.setEncoderParam(i, Param{}, c);
            }
        }
    }

    void render(Draw& d, int x, int y, int w, int h) override
    {
        int selTrkIdx = studio.selTrack;
        auto& trk = studio.tracks[selTrkIdx];
        Color themeColor = trk->themeColor;

        // 1. View Title Header
        std::string titleStr = "VIEW: INST & KEYBOARD - T" + std::to_string(selTrkIdx + 1) + " (" + engineRegistry[trk->currentEngineIdx].name + ")";
        d.text({ x + 4, y + 2 }, titleStr, 8, { .color = themeColor, .font = &PoppinsLight_8 });

        // Draw Page Indicator Dots on the right side of the View Title Badge
        auto [pageIdx, totalPages] = getViewPageInfo();
        if (totalPages > 1) {
            int dotW = 5;
            int dotH = 3;
            int gap = 2;
            int totalDotsW = totalPages * dotW + (totalPages - 1) * gap;
            int dotsX = x + w - 6 - totalDotsW;
            int dotsY = y + (16 - dotH) / 2;
            for (int p = 0; p < totalPages; p++) {
                Color dotCol = (p + 1 == pageIdx) ? themeColor : Color { 60, 72, 95, 255 };
                d.filledRect({ dotsX + p * (dotW + gap), dotsY }, { dotW, dotH }, { .color = dotCol });
            }
        }

        if (!trk || !trk->engine) return;

        // 2. Upper Main Canvas: Adaptive Engine Waveform / Response Curve & Polyphonic Playheads
        int canvasY = y + 18;
        int canvasH = 200;
        int canvasW = w;

        // Background box & vibrant frame outline
        d.filledRect({ x, canvasY }, { canvasW, canvasH }, { .color = { 12, 16, 24, 255 } });
        d.rect({ x, canvasY }, { canvasW, canvasH }, { .color = { 35, 45, 60, 255 } });

        float lStart = trk->engine->getLoopStart();
        float lLen = trk->engine->getLoopLength();
        bool isSampleEngine = engineRegistry[trk->currentEngineIdx].showWaveform || (lLen > 0.0f);

        // Highlight Sample Loop Region if applicable (zicXYv2 style)
        if (isSampleEngine && lLen > 0.0f) {
            int loopX = x + 2 + (int)(lStart * (canvasW - 4));
            int loopW = std::max(2, (int)(lLen * (canvasW - 4)));
            Color loopBg = { themeColor.r, themeColor.g, themeColor.b, 40 };
            d.filledRect({ loopX, canvasY + 2 }, { loopW, canvasH - 4 }, { .color = loopBg });

            Color markerCol = { themeColor.r, themeColor.g, themeColor.b, 200 };
            d.filledRect({ loopX, canvasY + 2 }, { 1, canvasH - 4 }, { .color = markerCol });
            d.filledRect({ std::min(x + canvasW - 3, loopX + loopW - 1), canvasY + 2 }, { 1, canvasH - 4 }, { .color = markerCol });

            // Loop Info Badges
            std::string loopStr = "LOOP " + std::to_string((int)(lStart * 100)) + "% - " + std::to_string((int)((lStart + lLen) * 100)) + "%";
            d.text({ x + 6, canvasY + 4 }, loopStr, 8, { .color = { 180, 195, 220, 255 }, .font = &PoppinsLight_8 });
        } else {
            std::string engineTypeStr = isSampleEngine ? "SAMPLE WAVEFORM" : "SYNTH RESPONSE CURVE";
            d.text({ x + 6, canvasY + 4 }, engineTypeStr, 8, { .color = { 110, 125, 150, 255 }, .font = &PoppinsLight_8 });
        }

        // Render Engine Waveform / Response Curve using engine->draw(x)
        int centerY = canvasY + canvasH / 2;
        int maxAmplitude = (canvasH - 24) / 2;
        int drawW = canvasW - 4;
        int prevX = x + 2;
        int prevY = centerY;

        for (int px = 0; px < drawW; px += 2) {
            float phase = (float)px / (float)drawW;
            float sampleVal = trk->engine->draw(phase);
            int ptY = centerY - (int)(sampleVal * (float)maxAmplitude);
            ptY = std::clamp(ptY, canvasY + 16, canvasY + canvasH - 4);
            int drawX = x + 2 + px;

            // Translucent wave area fill
            Color fillCol = { themeColor.r, themeColor.g, themeColor.b, 25 };
            if (ptY >= centerY) {
                d.filledRect({ drawX, centerY }, { 2, ptY - centerY + 1 }, { .color = fillCol });
            } else {
                d.filledRect({ drawX, ptY }, { 2, centerY - ptY + 1 }, { .color = fillCol });
            }

            // Vibrant wave line
            d.line({ prevX, prevY }, { drawX, ptY }, { .color = themeColor });
            prevX = drawX;
            prevY = ptY;
        }

        // Render Live Polyphonic Voice Playheads
        int voiceCount = trk->engine->getVoiceCount();
        int activeVoiceCount = 0;

        for (int v = 0; v < voiceCount; ++v) {
            float ph = trk->engine->getPlayhead(v);
            if (ph >= 0.0f && ph <= 1.0f) {
                activeVoiceCount++;
                int playheadX = x + 2 + (int)(ph * (canvasW - 4));
                playheadX = std::clamp(playheadX, x + 2, x + canvasW - 4);

                // Laser playhead line
                d.filledRect({ playheadX, canvasY + 16 }, { 2, canvasH - 20 }, { .color = { 255, 255, 255, 240 } });
                d.filledCircle({ playheadX + 1, canvasY + 16 }, 3, { .color = { 255, 255, 255, 255 } });
            }
        }

        // Auto-trigger note pulse on sequencer step hit
        if (studio.isPlaying) {
            int curStep = studio.currentStep % SEQ_STEPS;
            if (curStep != lastSeqStep) {
                lastSeqStep = curStep;
                if (curStep < (int)trk->sequence.size() && trk->sequence[curStep].active) {
                    notePulseLevel = 1.0f;
                    lastTriggeredNote = trk->sequence[curStep].note;
                }
            }
        }

        // Note Trigger Pulse Shockwave Animation (Sleek 2-ring pulse)
        notePulseLevel = std::max(0.0f, notePulseLevel - 0.045f);
        if (notePulseLevel > 0.01f) {
            int cx = x + canvasW / 2;
            int cy = canvasY + canvasH / 2;
            for (int r = 0; r < 2; r++) {
                float pFactor = notePulseLevel - (r * 0.25f);
                if (pFactor > 0.0f) {
                    int radius = (int)(22.0f + (1.0f - pFactor) * 32.0f + r * 6);
                    uint8_t alpha = (uint8_t)(pFactor * 160.0f);
                    d.circle({ cx, cy }, radius, { .color = { themeColor.r, themeColor.g, themeColor.b, alpha } });
                }
            }
        }

        // 3. Lower Section: Dual Visualizer Panels (Oscilloscope & Context Monitor)
        int panelY = canvasY + canvasH + 4;
        int panelH = h - (panelY - y) - 2;

        if (panelH >= 40) {
            // Panel A: Real-Time Audio Output Oscilloscope (Left, Width ~276px)
            int waveW = 276;
            d.filledRect({ x, panelY }, { waveW, panelH }, { .color = { 14, 18, 26, 255 } });
            d.rect({ x, panelY }, { waveW, panelH }, { .color = { 35, 45, 60, 255 } });

            d.text({ x + 6, panelY + 4 }, "LIVE OSCILLOSCOPE", 8, { .color = { 110, 125, 150, 255 }, .font = &PoppinsLight_8 });

            std::vector<float> trkHistory;
            {
                std::lock_guard<std::mutex> hl(trk->historyMtx);
                trkHistory.assign(trk->history.begin(), trk->history.end());
            }
            int historySize = (int)trkHistory.size();

            int oscCenterY = panelY + panelH / 2 + 2;
            int oscAmp = (panelH - 20) / 2;
            int prevOscX = x + 2;
            int prevOscY = oscCenterY;

            auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
            float idlePhase = (nowMs % 2500) / 2500.0f * 2.0f * M_PI;

            for (int px = 0; px < waveW - 4; px += 2) {
                float sampleVal = 0.0f;
                if (historySize > 0) {
                    int histIdx = std::clamp((int)(px * ((float)historySize / (float)waveW)), 0, historySize - 1);
                    sampleVal = trkHistory[histIdx];
                } else {
                    float normX = (float)px / (float)waveW;
                    sampleVal = 0.25f * std::sin(normX * 4.0f * M_PI + idlePhase);
                }

                int ptY = oscCenterY - (int)(sampleVal * (float)oscAmp);
                ptY = std::clamp(ptY, panelY + 16, panelY + panelH - 4);
                int drawX = x + 2 + px;

                Color areaCol = { themeColor.r, themeColor.g, themeColor.b, 20 };
                if (ptY >= oscCenterY) {
                    d.filledRect({ drawX, oscCenterY }, { 2, ptY - oscCenterY + 1 }, { .color = areaCol });
                } else {
                    d.filledRect({ drawX, ptY }, { 2, oscCenterY - ptY + 1 }, { .color = areaCol });
                }

                d.line({ prevOscX, prevOscY }, { drawX, ptY }, { .color = themeColor });
                prevOscX = drawX;
                prevOscY = ptY;
            }

            // Panel B: Waveform Detector & Morphing Geometry (Right Panel, Width w - 280)
            int monitorX = x + 280;
            int monitorW = w - 280;
            d.filledRect({ monitorX, panelY }, { monitorW, panelH }, { .color = { 14, 18, 26, 255 } });
            d.rect({ monitorX, panelY }, { monitorW, panelH }, { .color = { 35, 45, 60, 255 } });

            d.text({ monitorX + 6, panelY + 4 }, "WAVE DETECTOR", 8, { .color = { 110, 125, 150, 255 }, .font = &PoppinsLight_8 });

            // Analyze trkHistory for waveform detection
            float targetSine = 1.0f;
            float targetTri = 0.0f;
            float targetSaw = 0.0f;
            float targetSq = 0.0f;
            float targetNoise = 0.0f;

            if (historySize >= 16) {
                float maxPeak = 0.0f;
                float sumSq = 0.0f;
                for (float s : trkHistory) {
                    float absS = std::abs(s);
                    maxPeak = std::max(maxPeak, absS);
                    sumSq += s * s;
                }

                if (maxPeak >= 0.01f) {
                    float rms = std::sqrt(sumSq / (float)historySize);
                    float crestFactor = maxPeak / (rms + 1e-5f);

                    int N = historySize;
                    float sumDiff = 0.0f;
                    float sumCurv = 0.0f;
                    int posSlopeCount = 0;
                    int negSlopeCount = 0;
                    int plateauCount = 0;

                    std::vector<float> diffs(N - 1, 0.0f);
                    for (int i = 0; i < N - 1; i++) {
                        diffs[i] = trkHistory[i + 1] - trkHistory[i];
                        float absD = std::abs(diffs[i]);
                        sumDiff += absD;
                        if (diffs[i] > 0.001f) posSlopeCount++;
                        else if (diffs[i] < -0.001f) negSlopeCount++;

                        if (std::abs(trkHistory[i]) > 0.75f * maxPeak && absD < 0.02f * maxPeak) {
                            plateauCount++;
                        }
                    }
                    float avgAbsSlope = sumDiff / (float)(N - 1);

                    for (int i = 0; i < N - 2; i++) {
                        sumCurv += std::abs(diffs[i + 1] - diffs[i]);
                    }
                    float noiseIndex = sumCurv / (avgAbsSlope * (float)(N - 2) + 1e-5f);
                    float plateauRatio = (float)plateauCount / (float)(N - 1);
                    float slopeAsym = std::abs((float)(posSlopeCount - negSlopeCount)) / (float)(posSlopeCount + negSlopeCount + 1);

                    // Estimate noise energy
                    if (noiseIndex > 2.2f || (crestFactor > 2.8f && plateauRatio < 0.1f)) {
                        targetNoise = std::clamp((noiseIndex - 1.8f) * 0.8f, 0.0f, 1.0f);
                    }

                    float remainingSignal = 1.0f - targetNoise;

                    // Waveform shape weights
                    float rawSq = std::clamp(plateauRatio * 2.5f + (crestFactor < 1.3f ? 0.4f : 0.0f), 0.0f, 1.0f);
                    float rawSaw = std::clamp(slopeAsym * 2.2f, 0.0f, 1.0f);

                    // Triangle has linear slope (low curvature) and moderate crest factor (~1.73), low asymmetry
                    float rawTri = std::clamp((1.0f - slopeAsym * 1.5f) * (crestFactor > 1.5f ? 0.8f : 0.4f) * (1.0f - rawSq), 0.0f, 1.0f);

                    // Sine has smooth curvature, low plateau, low asymmetry, crest factor ~1.41
                    float rawSine = std::clamp((1.0f - rawSq) * (1.0f - rawSaw) * (1.0f - rawTri * 0.7f), 0.0f, 1.0f);

                    float totalRaw = rawSine + rawTri + rawSaw + rawSq + 1e-5f;
                    targetSine = (rawSine / totalRaw) * remainingSignal;
                    targetTri = (rawTri / totalRaw) * remainingSignal;
                    targetSaw = (rawSaw / totalRaw) * remainingSignal;
                    targetSq = (rawSq / totalRaw) * remainingSignal;
                }
            }

            // Exponential decay smoothing to avoid UI flicker
            float alpha = 0.15f;
            smoothedSineWeight = smoothedSineWeight * (1.0f - alpha) + targetSine * alpha;
            smoothedTriWeight = smoothedTriWeight * (1.0f - alpha) + targetTri * alpha;
            smoothedSawWeight = smoothedSawWeight * (1.0f - alpha) + targetSaw * alpha;
            smoothedSqWeight = smoothedSqWeight * (1.0f - alpha) + targetSq * alpha;
            smoothedNoiseWeight = smoothedNoiseWeight * (1.0f - alpha) + targetNoise * alpha;

            float wSum = smoothedSineWeight + smoothedTriWeight + smoothedSawWeight + smoothedSqWeight + 1e-5f;
            float wSine = smoothedSineWeight / wSum;
            float wTri = smoothedTriWeight / wSum;
            float wSaw = smoothedSawWeight / wSum;
            float wSq = smoothedSqWeight / wSum;

            // Geometry rendering parameters
            int shapeCX = monitorX + monitorW / 2;
            int shapeCY = panelY + panelH / 2 + 2;
            float rBase = std::min(monitorW, panelH) * 0.28f;

            const int NUM_PTS = 24;
            std::vector<Point> morphedShape;
            morphedShape.reserve(NUM_PTS);
            std::vector<float> rMorphed(NUM_PTS, 0.0f);
            std::vector<float> ptAngles(NUM_PTS, 0.0f);

            for (int k = 0; k < NUM_PTS; k++) {
                float a = (float)k * (2.0f * M_PI / (float)NUM_PTS) - M_PI_2;
                ptAngles[k] = a;

                // 1. Sine radius (Circle)
                float rSine = rBase;

                // 2. Triangle radius (Symmetrical Triangle: Top peak at 0,-rBase; bottom corners at +/- rBase, rBase)
                float cosA = std::cos(a);
                float sinA = std::sin(a);
                float rTri = rBase;
                if (sinA > 0.333f) {
                    rTri = rBase / std::max(0.01f, sinA);
                } else {
                    float absCos = std::abs(cosA);
                    rTri = rBase / std::max(0.01f, (absCos * 0.866f - sinA * 0.5f) + sinA);
                }
                rTri = std::clamp(rTri, rBase * 0.5f, rBase * 1.35f);

                // 3. Saw radius (Right-angled Triangle with vertical right edge)
                float rSaw = rBase;
                if (cosA > 0.0f && sinA > -0.5f) {
                    rSaw = rBase / std::max(0.01f, cosA);
                } else if (sinA > 0.333f) {
                    rSaw = rBase / std::max(0.01f, sinA);
                } else {
                    rSaw = rBase / std::max(0.01f, std::abs(cosA + sinA) * 0.707f);
                }
                rSaw = std::clamp(rSaw, rBase * 0.5f, rBase * 1.35f);

                // 4. Square / Rectangle radius
                float rSq = rBase / std::max(std::abs(cosA), std::abs(sinA));
                rSq = std::clamp(rSq, rBase * 0.5f, rBase * 1.414f);

                // Morph blend
                float rBlended = wSine * rSine + wTri * rTri + wSaw * rSaw + wSq * rSq;
                rMorphed[k] = rBlended;

                int px = shapeCX + (int)(rBlended * cosA);
                int py = shapeCY + (int)(rBlended * sinA);
                morphedShape.push_back({ px, py });
            }

            // Draw filled morphed polygon & outline
            Color shapeFillCol = { themeColor.r, themeColor.g, themeColor.b, 40 };
            d.filledPolygon(morphedShape, { .color = shapeFillCol });
            d.lines(morphedShape, { .color = themeColor, .thickness = 1 });
            d.line(morphedShape.back(), morphedShape.front(), { .color = themeColor, .thickness = 1 });

            // Draw Noise particle swarm if noise detected
            if (smoothedNoiseWeight > 0.05f) {
                int dotCount = (int)(smoothedNoiseWeight * 20.0f);
                for (int i = 0; i < dotCount; i++) {
                    float pAngle = i * 0.52f + idlePhase * 4.0f;
                    float pDist = 3.0f + std::fmod((float)(i * 7 + idlePhase * 50.0f), rBase * 1.25f);
                    int dotX = shapeCX + (int)(std::cos(pAngle) * pDist);
                    int dotY = shapeCY + (int)(std::sin(pAngle) * pDist);
                    uint8_t dotAlpha = (uint8_t)(90 + (i * 19) % 150);
                    d.pixel({ dotX, dotY }, { .color = { 255, 245, 170, dotAlpha } });
                }
            }

            // Draw Instrument Trigger Pulse Shockwave (Conforming to detected shape)
            if (notePulseLevel > 0.01f) {
                for (int r = 0; r < 2; r++) {
                    float pFactor = notePulseLevel - (r * 0.25f);
                    if (pFactor > 0.0f) {
                        float pulseScale = 1.0f + (1.0f - pFactor) * 0.65f + r * 0.22f;
                        std::vector<Point> pulseRing;
                        pulseRing.reserve(NUM_PTS);
                        for (int k = 0; k < NUM_PTS; k++) {
                            float pr = rMorphed[k] * pulseScale;
                            float a = ptAngles[k];
                            pulseRing.push_back({ shapeCX + (int)(pr * std::cos(a)), shapeCY + (int)(pr * std::sin(a)) });
                        }
                        uint8_t pulseAlpha = (uint8_t)(pFactor * 160.0f);
                        d.lines(pulseRing, { .color = { themeColor.r, themeColor.g, themeColor.b, pulseAlpha }, .thickness = 1 });
                        d.line(pulseRing.back(), pulseRing.front(), { .color = { themeColor.r, themeColor.g, themeColor.b, pulseAlpha }, .thickness = 1 });
                    }
                }
            }

            // Draw Detected Waveform Label Text
            const char* labelText = "SINE";
            if (smoothedNoiseWeight > 0.45f) {
                labelText = "NOISE";
            } else if (wSq > 0.5f) {
                labelText = "SQUARE";
            } else if (wSaw > 0.45f) {
                labelText = "SAW";
            } else if (wTri > 0.45f) {
                labelText = "TRIANGLE";
            } else if (wSine > 0.45f) {
                labelText = "SINE";
            } else {
                labelText = "HYBRID";
            }
            d.text({ monitorX + 6, panelY + panelH - 12 }, labelText, 8, { .color = { themeColor.r, themeColor.g, themeColor.b, 200 }, .font = &PoppinsLight_8 });
        }
    }

    void handleDynamicPadPress(int col, int row, bool pressed) override
    {
        auto& pad = gridState.pads[col][row];
        pad.pressed = pressed;

        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];

        if (pressed) {
            notePulseLevel = 1.0f;
            lastTriggeredNote = pad.note;
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            t->engine->noteOn(pad.note, 0.9f);
        } else {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            t->engine->noteOff(pad.note);
        }

        updatePadLeds();
    }

    void handleEncoder(int encoderId, int delta) override
    {
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];

        if (gridState.utility.shiftActive) {
            if (encoderId == 1) { // Synth engine selection
                int nextEngine = std::clamp((int)t->currentEngineIdx + delta, 0, ENGINE_REGISTRY_COUNT - 1);
                if (nextEngine != t->currentEngineIdx) {
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    t->setEngine(nextEngine);
                    currentPage = 0;
                }
            } else if (encoderId == 2) { // Track volume
                t->volume = std::clamp(t->volume + delta * 0.05f, 0.0f, 1.0f);
            }
        } else if (t && t->engine) {
            int pIdx = encoderId - 1;
            if (pIdx >= 0 && pIdx < TOTAL_ENCODERS) {
                int actualParamIdx = currentPage * TOTAL_ENCODERS + pIdx;
                if ((size_t)actualParamIdx < t->engine->getParamCount()) {
                    auto& p = t->engine->getParams()[actualParamIdx];
                    p.set(p.value + delta * p.step);
                }
            }
        }

        updatePadLeds();
        updateEncoderLabels();
    }

private:
    std::string getNoteName(int note)
    {
        static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        int octave = (note / 12) - 1;
        return std::string(names[note % 12]) + std::to_string(octave);
    }
};

