#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <mutex>
#include <string>
#include <vector>

#include "audio/Wavetable.h"
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

    // Smooth state for Panel B geometric waveform shape detector
    float smoothedMorphVal = 0.0f; // 0.0 = Symmetrical Triangle, 0.5 = Saw (Right Triangle), 1.0 = Square
    float smoothedSineWeight = 0.0f; // 1.0 = Pure Circle, 0.0 = Morph Polygon
    float smoothedNoiseFactor = 0.0f; // 0.0 = Clean signal, 1.0 = Pure Noise particle cloud
    float smoothedSignalLevel = 0.0f; // 0.0 = Silent/Idle ambient spin, 1.0 = Active audio shape

    bool showEngineParams = false;

public:
    InstrumentView()
        : View("INSTRUMENT & SYNTH")
    {
    }

    bool isShowingSubParams() const override { return showEngineParams; }
    void toggleSubParams(bool active) override { showEngineParams = active; }

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
        if (!isSameTrack) {
            currentPage = 0;
            showEngineParams = false;
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

    void onDeactivate() override
    {
        showEngineParams = false;
        for (int r = 0; r < PAD_ROWS; ++r) {
            for (int c = 0; c < PAD_COLS; ++c) {
                gridState.pads[c][r].selected = false;
                gridState.pads[c][r].pressed = false;
                gridState.pads[c][r].active = false;
            }
        }
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
                pad.active = pad.pressed;

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

        // Row 3 Global Utility Pads (Cols 8..11 - Page Left, Page Right, Oct-, Oct+)
        gridState.pads[8][3].label = "&icon::arrowLeft::filled";
        gridState.pads[8][3].color = { 255, 160, 40, 255 };
        gridState.pads[9][3].label = "&icon::arrowRight::filled";
        gridState.pads[9][3].color = { 255, 160, 40, 255 };
        gridState.pads[10][3].label = "Oct-";
        gridState.pads[10][3].color = { 100, 120, 255, 255 };
        gridState.pads[11][3].label = "Oct+";
        gridState.pads[11][3].color = { 100, 120, 255, 255 };

        // Reset default utility labels
        for (int c = 0; c < 4; ++c) {
            gridState.pads[8 + c][0].label = "T" + std::to_string(c + 1);
            gridState.pads[8 + c][1].label = "T" + std::to_string(c + 5);
        }
        gridState.pads[8][2].label = "Instr.";

        // Secret combination hinting & latched sub-params "Engine" label updates
        if (showEngineParams) {
            gridState.pads[8][2].label = "Engine";
            int activeTrk = studio.selTrack;
            if (activeTrk >= 0 && activeTrk < 8) {
                int col = 8 + (activeTrk % 4);
                int row = activeTrk / 4;
                gridState.pads[col][row].label = "Engine";
            }
        } else {
            if (isAnyTrackPadPressed()) {
                gridState.pads[8][2].label = "Engine";
            }
            if (gridState.pads[8][2].pressed) {
                int activeTrk = studio.selTrack;
                if (activeTrk >= 0 && activeTrk < 8) {
                    int col = 8 + (activeTrk % 4);
                    int row = activeTrk / 4;
                    gridState.pads[col][row].label = "Engine";
                }
            }
        }
    }

    void updateEncoderLabels() override
    {
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];
        Color c = t->themeColor;

        if (showEngineParams || (gridState.pads[8][2].pressed && isAnyTrackPadPressed())) {
            gridState.setEncoder(0, "Synth", t->currentEngineIdx, 0, ENGINE_REGISTRY_COUNT - 1, 1, engineRegistry[t->currentEngineIdx].name, c);
            gridState.setEncoder(1, "Volume", (int)(t->volume * 100.0f), 0, 100, 1, nullptr, c, "%");

            for (int i = 2; i < TOTAL_ENCODERS; ++i) {
                gridState.setEncoderParam(i, Param {}, c);
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
                    gridState.setEncoderParam(i, Param {}, c);
                }
            }
        } else {
            for (int i = 0; i < TOTAL_ENCODERS; ++i) {
                gridState.setEncoderParam(i, Param {}, c);
            }
        }
    }

    void render(Draw& d, int x, int y, int w, int h) override
    {
        int selTrkIdx = studio.selTrack;
        auto& trk = studio.tracks[selTrkIdx];
        Color themeColor = trk->themeColor;

        if (!trk || !trk->engine) return;

        int canvasY = y + 4;
        int canvasH = 200;
        int canvasW = w;

        // Render Page Indicators (Squares on upper right of waveform canvas, under params and over waveform)
        int totalPages = getTotalPages();
        int sqW = 8;
        int sqH = 4;
        int sqGap = 4;
        int totalSqW = totalPages * sqW + (totalPages - 1) * sqGap;
        int sqStartX = x + canvasW - 10 - totalSqW;
        int sqY = y - 3;

        for (int p = 0; p < totalPages; ++p) {
            int px = sqStartX + p * (sqW + sqGap);
            if (p == currentPage) {
                d.filledRect({ px, sqY }, { sqW, sqH }, { .color = themeColor });
                // d.rect({ px, sqY }, { sqW, sqH }, { .color = { 255, 255, 255, 255 } });
            } else {
                d.filledRect({ px, sqY }, { sqW, sqH }, { .color = { 25, 35, 50, 255 } });
                d.rect({ px, sqY }, { sqW, sqH }, { .color = { 65, 80, 105, 255 } });
            }
        }

        // 2. Upper Main Canvas: Adaptive Engine Waveform / Response Curve & Polyphonic Playheads

        // Background box & vibrant frame outline
        d.filledRect({ x, canvasY }, { canvasW, canvasH }, { .color = { 12, 16, 24, 255 } });
        d.rect({ x, canvasY }, { canvasW, canvasH }, { .color = { 35, 45, 60, 255 } });

        float lStart = trk->engine->getLoopStart();
        float lLen = trk->engine->getLoopLength();
        bool isSampleEngine = engineRegistry[trk->currentEngineIdx].showWaveform || (lLen > 0.0f);
        Wavetable* wt = trk->engine->getWavetable();

        if (wt != nullptr && wt->samples() != nullptr) {
            // ── 3D WAVETABLE MESH VISUALIZER ──────────────────────────────────────
            float morphPos = trk->engine->getWavetableMorph();
            int activeFrameIdx = std::clamp((int)(morphPos * 63.0f), 0, 63);

            std::string engineTypeStr = "3D WAVETABLE MESH";
            d.text({ x + 6, canvasY + 4 }, engineTypeStr, 8, { .color = { 180, 195, 220, 255 }, .font = &PoppinsLight_8 });

            std::string morphStr = "MORPH: " + std::to_string(activeFrameIdx + 1) + "/64 (" + std::to_string((int)(morphPos * 100.0f)) + "%)";
            d.text({ sqStartX - 130, canvasY + 4 }, morphStr, 8, { .color = themeColor, .font = &PoppinsLight_8 });

            // Keyframe depth slices subset (morph 0 in front, morph 63 in back)
            std::vector<int> sliceFrames = { 0, 8, 16, 24, 32, 40, 48, 56, 63 };
            if (std::find(sliceFrames.begin(), sliceFrames.end(), activeFrameIdx) == sliceFrames.end()) {
                sliceFrames.push_back(activeFrameIdx);
            }
            std::sort(sliceFrames.begin(), sliceFrames.end(), std::greater<int>());

            int numSlices = (int)sliceFrames.size();
            int innerW = canvasW - 20;
            int baseSliceW = innerW - 48;
            int originX = x + 14;
            int originY = canvasY + canvasH - 70;

            std::vector<std::vector<Point>> allSlicePoints(numSlices);

            for (int i = 0; i < numSlices; i++) {
                int frameIdx = sliceFrames[i];
                float z = 1.0f - ((float)frameIdx / 63.0f); // 1.0 at front (frame 0), 0.0 at back (frame 63)

                int sliceOffsetX = (int)((1.0f - z) * 48.0f);
                int sliceOffsetY = (int)(-(1.0f - z) * 36.0f);
                int sliceW = (int)(baseSliceW * (0.70f + z * 0.30f));
                int sliceH = (int)(26.0f * (0.50f + z * 0.50f));

                int sx0 = originX + sliceOffsetX;
                int sy0 = originY + sliceOffsetY;

                int ptsCount = 32;
                for (int p = 0; p <= ptsCount; p++) {
                    float t = (float)p / (float)ptsCount;
                    float rawWave = wt->getSampleAt(frameIdx, t);
                    int px = sx0 + (int)(t * sliceW);
                    int py = sy0 - (int)(rawWave * sliceH);
                    allSlicePoints[i].push_back({ px, py });
                }
            }

            // Render Perspective Connecting Lattice Wireframe Mesh Lines
            for (int i = 0; i < numSlices - 1; i++) {
                int frameIdx = sliceFrames[i];
                float z = 1.0f - ((float)frameIdx / 63.0f);
                uint8_t meshAlpha = (uint8_t)(40 + z * 90.0f);
                Color meshCol = Color { themeColor.r, themeColor.g, themeColor.b, meshAlpha };

                size_t step = 4;
                for (size_t p = 0; p < allSlicePoints[i].size(); p += step) {
                    d.line(allSlicePoints[i][p], allSlicePoints[i + 1][p], { .color = meshCol });
                }
            }

            // Render 3D Slice Curves (Back-to-Front)
            for (int i = 0; i < numSlices; i++) {
                int frameIdx = sliceFrames[i];
                float z = 1.0f - ((float)frameIdx / 63.0f);
                const auto& slicePts = allSlicePoints[i];

                if (frameIdx == activeFrameIdx) {
                    int sliceW = (int)(baseSliceW * (0.70f + z * 0.30f));
                    int sliceOffsetX = (int)((1.0f - z) * 48.0f);
                    int sliceOffsetY = (int)(-(1.0f - z) * 36.0f);
                    int sx0 = originX + sliceOffsetX;
                    int sy0 = originY + sliceOffsetY;

                    std::vector<Point> fillPoly = slicePts;
                    fillPoly.push_back({ sx0 + sliceW, sy0 });
                    fillPoly.push_back({ sx0, sy0 });
                    d.filledPolygon(fillPoly, { .color = { themeColor.r, themeColor.g, themeColor.b, 75 } });

                    d.lines(slicePts, { .color = { 255, 255, 255, 255 }, .thickness = 1 });
                } else {
                    uint8_t lineAlpha = (uint8_t)(60 + z * 140.0f);
                    d.lines(slicePts, { .color = { themeColor.r, themeColor.g, themeColor.b, lineAlpha } });
                }
            }
        } else {
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

            // Draw grid reference lines for Synth Response Curve mode
            if (!isSampleEngine) {
                d.line({ x + 2, centerY }, { x + canvasW - 2, centerY }, { .color = { 45, 60, 80, 140 } });
                d.line({ x + 2, centerY - maxAmplitude / 2 }, { x + canvasW - 2, centerY - maxAmplitude / 2 }, { .color = { 30, 42, 58, 80 } });
                d.line({ x + 2, centerY + maxAmplitude / 2 }, { x + canvasW - 2, centerY + maxAmplitude / 2 }, { .color = { 30, 42, 58, 80 } });

                for (int mark = 1; mark <= 3; mark++) {
                    int gridX = x + 2 + (drawW * mark) / 4;
                    d.line({ gridX, canvasY + 16 }, { gridX, canvasY + canvasH - 4 }, { .color = { 30, 42, 58, 70 } });
                }
            }

            std::vector<float> curveSamples(drawW / 2);
            float maxCurveVal = 0.0f;
            for (int i = 0; i < (int)curveSamples.size(); i++) {
                float phase = (float)(i * 2) / (float)drawW;
                float val = trk->engine->draw(phase);
                curveSamples[i] = val;
                maxCurveVal = std::max(maxCurveVal, std::abs(val));
            }

            // Dynamic fallback for synth engines if draw(x) returned flat line (0.0f)
            if (!isSampleEngine && maxCurveVal < 0.001f) {
                auto lerpF = [](float a, float b, float t) { return a + t * (b - a); };
                size_t pCount = trk->engine->getParamCount();
                auto* pArr = trk->engine->getParams();
                float p0 = (pCount > 0 && pArr) ? (pArr[0].value - pArr[0].min) / std::max(0.001f, pArr[0].max - pArr[0].min) : 0.5f;
                float p1 = (pCount > 1 && pArr) ? (pArr[1].value - pArr[1].min) / std::max(0.001f, pArr[1].max - pArr[1].min) : 0.3f;
                float p2 = (pCount > 2 && pArr) ? (pArr[2].value - pArr[2].min) / std::max(0.001f, pArr[2].max - pArr[2].min) : 0.0f;

                for (int i = 0; i < (int)curveSamples.size(); i++) {
                    float phase = (float)(i * 2) / (float)drawW;
                    float saw = 2.0f * phase - 1.0f;
                    float sq = (phase < 0.5f) ? 1.0f : -1.0f;
                    float tri = 1.0f - 4.0f * std::abs(std::remainder(phase, 1.0f) - 0.5f);
                    float osc = (p0 < 0.5f) ? lerpF(tri, saw, p0 * 2.0f) : lerpF(saw, sq, (p0 - 0.5f) * 2.0f);

                    float damping = 1.0f / (1.0f + std::pow(phase, 1.5f + p1 * 3.0f) * (1.0f - p1) * 8.0f);
                    float resPeak = 1.0f + p2 * 1.8f * std::exp(-std::pow((phase - p1) * 6.0f, 2.0f));
                    curveSamples[i] = std::clamp(osc * damping * resPeak, -1.0f, 1.0f);
                }
            }

            int prevX = x + 2;
            int prevY = centerY - (int)(curveSamples[0] * (float)maxAmplitude);

            for (int i = 0; i < (int)curveSamples.size(); i++) {
                float sampleVal = curveSamples[i];
                int ptY = centerY - (int)(sampleVal * (float)maxAmplitude);
                ptY = std::clamp(ptY, canvasY + 16, canvasY + canvasH - 4);
                int drawX = x + 2 + i * 2;

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
                std::chrono::steady_clock::now().time_since_epoch())
                             .count();
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

            // Analyze trkHistory for waveform classification
            float targetMorphVal = smoothedMorphVal;
            float targetSineWeight = 0.0f;
            float targetNoiseFactor = 0.0f;
            float targetSignalLevel = 0.0f;
            float maxPeak = 0.0f;

            if (historySize >= 16) {
                float sumSq = 0.0f;
                for (float s : trkHistory) {
                    float absS = std::abs(s);
                    maxPeak = std::max(maxPeak, absS);
                    sumSq += s * s;
                }

                if (maxPeak >= 0.01f) {
                    targetSignalLevel = 1.0f;
                    float rms = std::sqrt(sumSq / (float)historySize);
                    float crestFactor = maxPeak / (rms + 1e-5f);

                    int N = historySize;
                    float sumDiff = 0.0f;
                    float sumCurv = 0.0f;
                    int posSlopeCount = 0;
                    int negSlopeCount = 0;
                    int plateauCount = 0;

                    for (int i = 0; i < N - 1; i++) {
                        float diff = trkHistory[i + 1] - trkHistory[i];
                        float absD = std::abs(diff);
                        sumDiff += absD;
                        if (diff > 0.001f) posSlopeCount++;
                        else if (diff < -0.001f) negSlopeCount++;

                        if (std::abs(trkHistory[i]) > 0.70f * maxPeak && absD < 0.03f * maxPeak) {
                            plateauCount++;
                        }

                        if (i < N - 2) {
                            float diffNext = trkHistory[i + 2] - trkHistory[i + 1];
                            sumCurv += std::abs(diffNext - diff);
                        }
                    }
                    float avgAbsSlope = sumDiff / (float)(N - 1);
                    float noiseIndex = sumCurv / (avgAbsSlope * (float)(N - 2) + 1e-5f);
                    float plateauRatio = (float)plateauCount / (float)(N - 1);
                    float slopeAsym = std::abs((float)(posSlopeCount - negSlopeCount)) / (float)(posSlopeCount + negSlopeCount + 1);

                    // Estimate noise factor
                    if (noiseIndex > 2.0f || (crestFactor > 2.7f && plateauRatio < 0.1f)) {
                        targetNoiseFactor = std::clamp((noiseIndex - 1.5f) * 0.7f, 0.1f, 1.0f);
                    }

                    // Definitive Classification:
                    // 1. Square / Pulse: flat top/bottom plateau ratio or low crest factor
                    if (plateauRatio > 0.15f || crestFactor < 1.32f) {
                        targetMorphVal = 1.0f; // Square / Rectangle
                        targetSineWeight = 0.0f;
                    }
                    // 2. Saw (Right Triangle): high slope asymmetry
                    else if (slopeAsym > 0.26f) {
                        targetMorphVal = 0.5f; // Saw ("triangle rectangle")
                        targetSineWeight = 0.0f;
                    }
                    // 3. Triangle: linear slope with higher crest factor (C >= 1.63)
                    else if (crestFactor >= 1.63f) {
                        targetMorphVal = 0.0f; // Symmetrical Triangle
                        targetSineWeight = 0.0f;
                    }
                    // 4. Sine Circle: default for smooth symmetrical wave (C < 1.63)
                    else {
                        targetSineWeight = 1.0f; // Pure Sine Circle
                        targetMorphVal = 0.0f;
                    }
                }
            }

            // Exponential decay smoothing (fast response alpha = 0.30f)
            float alpha = 0.30f;
            smoothedMorphVal = smoothedMorphVal * (1.0f - alpha) + targetMorphVal * alpha;
            smoothedSineWeight = smoothedSineWeight * (1.0f - alpha) + targetSineWeight * alpha;
            smoothedNoiseFactor = smoothedNoiseFactor * (1.0f - alpha) + targetNoiseFactor * alpha;
            smoothedSignalLevel = smoothedSignalLevel * 0.80f + targetSignalLevel * 0.20f;

            // Geometry bounding box parameters
            int shapeCX = monitorX + monitorW / 2;
            int shapeCY = panelY + panelH / 2 + 2;
            int maxHalfW = (int)(monitorW * 0.22f);
            int maxHalfH = (int)(panelH * 0.28f);

            float animTime = (nowMs % 100000) * 0.001f;

            // 1. Spinning Orbital Geometry (ALWAYS present in Panel B!)
            float spinSpeed = 2.2f + smoothedSignalLevel * 1.5f;
            float idleSpinAngle = animTime * spinSpeed;
            uint8_t shellAlpha = (uint8_t)(80 + smoothedSignalLevel * 50.0f);

            // Orbiting 5-point rotating FM shell (ALWAYS rendered!)
            int numShellPts = 5;
            std::vector<Point> ambientShell;
            for (int i = 0; i < numShellPts; i++) {
                float a = idleSpinAngle + i * (6.28318f / numShellPts);
                float rW = (maxHalfW + 7.0f) + std::sin(a * 3.0f + animTime * 3.5f) * (4.0f + smoothedSignalLevel * 3.0f);
                float rH = (maxHalfH + 7.0f) + std::cos(a * 2.0f + animTime * 3.0f) * (4.0f + smoothedSignalLevel * 3.0f);
                int mx = shapeCX + (int)(std::cos(a) * rW);
                int my = shapeCY + (int)(std::sin(a) * rH);
                ambientShell.push_back({ mx, my });
            }
            d.lines(ambientShell, { .color = { themeColor.r, themeColor.g, themeColor.b, shellAlpha }, .thickness = 1 });
            d.line(ambientShell.back(), ambientShell.front(), { .color = { themeColor.r, themeColor.g, themeColor.b, shellAlpha }, .thickness = 1 });

            // Orbiting star particles (ALWAYS rendered!)
            for (int i = 0; i < 8; i++) {
                float a = idleSpinAngle * 0.8f + i * (6.28318f / 8.0f);
                float r = maxHalfW * 0.85f + std::sin(a * 2.0f + animTime * 4.0f) * 5.0f;
                int px = shapeCX + (int)(std::cos(a) * r);
                int py = shapeCY + (int)(std::sin(a) * r);
                d.pixel({ px, py }, { .color = { themeColor.r, themeColor.g, themeColor.b, (uint8_t)(shellAlpha * 0.85f) } });
            }

            // 2. Active Audio Waveform Shape (expands geometrically from center core when audio is active)
            if (smoothedSignalLevel > 0.01f) {
                int halfW = std::max(2, (int)(maxHalfW * smoothedSignalLevel));
                int halfH = std::max(2, (int)(maxHalfH * smoothedSignalLevel));

                Point pBL = { shapeCX - halfW, shapeCY + halfH };
                Point pBR = { shapeCX + halfW, shapeCY + halfH };
                Point pTL, pTR;

                if (smoothedMorphVal <= 0.5f) {
                    float t = smoothedMorphVal / 0.5f;
                    int topX = shapeCX + (int)(t * halfW);
                    pTL = { topX, shapeCY - halfH };
                    pTR = { topX, shapeCY - halfH };
                } else {
                    float t = (smoothedMorphVal - 0.5f) / 0.5f;
                    int tlX = (shapeCX + halfW) - (int)(t * 2.0f * halfW);
                    pTR = { shapeCX + halfW, shapeCY - halfH };
                    pTL = { tlX, shapeCY - halfH };
                }

                std::vector<Point> basePoly;
                if (std::abs(pTL.x - pTR.x) <= 1) {
                    basePoly = { pBL, pTR, pBR };
                } else {
                    basePoly = { pBL, pTL, pTR, pBR };
                }

                auto getPolyPoint = [](const std::vector<Point>& poly, float t) -> Point {
                    int M = poly.size();
                    if (M == 0) return { 0, 0 };
                    std::vector<float> segLens(M);
                    float totalLen = 0.0f;
                    for (int i = 0; i < M; i++) {
                        Point p1 = poly[i];
                        Point p2 = poly[(i + 1) % M];
                        float dx = (float)(p2.x - p1.x);
                        float dy = (float)(p2.y - p1.y);
                        segLens[i] = std::sqrt(dx * dx + dy * dy);
                        totalLen += segLens[i];
                    }
                    if (totalLen < 1e-3f) return poly[0];

                    float targetD = t * totalLen;
                    float accumulated = 0.0f;
                    for (int i = 0; i < M; i++) {
                        if (targetD <= accumulated + segLens[i] || i == M - 1) {
                            float segT = (targetD - accumulated) / std::max(0.001f, segLens[i]);
                            segT = std::clamp(segT, 0.0f, 1.0f);
                            Point p1 = poly[i];
                            Point p2 = poly[(i + 1) % M];
                            int rx = p1.x + (int)(segT * (p2.x - p1.x));
                            int ry = p1.y + (int)(segT * (p2.y - p1.y));
                            return { rx, ry };
                        }
                        accumulated += segLens[i];
                    }
                    return poly[0];
                };

                std::vector<Point> morphedShape;
                if (smoothedSineWeight <= 0.03f) {
                    morphedShape = basePoly;
                } else {
                    const int NUM_PTS = 16;
                    float circleRadius = std::min(halfW, halfH) * 1.15f;
                    for (int k = 0; k < NUM_PTS; k++) {
                        float frac = (float)k / (float)NUM_PTS;
                        float a = frac * 2.0f * M_PI - M_PI_2;
                        Point circlePt = { shapeCX + (int)(circleRadius * std::cos(a)), shapeCY + (int)(circleRadius * std::sin(a)) };
                        Point polyPt = getPolyPoint(basePoly, frac);

                        int mx = (int)(polyPt.x * (1.0f - smoothedSineWeight) + circlePt.x * smoothedSineWeight);
                        int my = (int)(polyPt.y * (1.0f - smoothedSineWeight) + circlePt.y * smoothedSineWeight);
                        morphedShape.push_back({ mx, my });
                    }
                }

                uint8_t shapeAlpha = (uint8_t)(255.0f * smoothedSignalLevel);
                uint8_t fillAlpha = (uint8_t)(60.0f * (1.0f - smoothedNoiseFactor * 0.5f) * smoothedSignalLevel);

                // Sub-Bass / Harmonic Ghost Echo Shape
                int rOffsetX = (int)(std::sin(animTime * 3.0f) * 8.0f);
                int rOffsetY = (int)(std::cos(animTime * 2.5f) * 3.0f);
                std::vector<Point> ghostShape;
                for (const auto& pt : morphedShape) {
                    ghostShape.push_back({ pt.x + rOffsetX, pt.y + rOffsetY });
                }
                d.filledPolygon(ghostShape, { .color = { themeColor.r, themeColor.g, themeColor.b, (uint8_t)(fillAlpha * 0.35f) } });
                d.lines(ghostShape, { .color = { themeColor.r, themeColor.g, themeColor.b, (uint8_t)(shapeAlpha * 0.3f) }, .thickness = 1 });
                d.line(ghostShape.back(), ghostShape.front(), { .color = { themeColor.r, themeColor.g, themeColor.b, (uint8_t)(shapeAlpha * 0.3f) }, .thickness = 1 });

                // Render Central Audio Shape
                d.filledPolygon(morphedShape, { .color = { themeColor.r, themeColor.g, themeColor.b, fillAlpha } });
                d.lines(morphedShape, { .color = { themeColor.r, themeColor.g, themeColor.b, shapeAlpha }, .thickness = 1 });
                d.line(morphedShape.back(), morphedShape.front(), { .color = { themeColor.r, themeColor.g, themeColor.b, shapeAlpha }, .thickness = 1 });

                // Note Trigger Pulse & Expanding Halo Shockwaves
                if (notePulseLevel > 0.01f) {
                    for (int r = 0; r < 3; r++) {
                        float pFactor = notePulseLevel - (r * 0.22f);
                        if (pFactor > 0.0f) {
                            float scale = 1.05f + (1.0f - pFactor) * 0.65f + r * 0.18f;
                            uint8_t pulseAlpha = (uint8_t)(pFactor * 160.0f * smoothedSignalLevel);

                            std::vector<Point> pulseShape;
                            for (const auto& pt : morphedShape) {
                                int px = shapeCX + (int)((pt.x - shapeCX) * scale);
                                int py = shapeCY + (int)((pt.y - shapeCY) * scale);
                                pulseShape.push_back({ px, py });
                            }

                            d.lines(pulseShape, { .color = { themeColor.r, themeColor.g, themeColor.b, pulseAlpha }, .thickness = 1 });
                            d.line(pulseShape.back(), pulseShape.front(), { .color = { themeColor.r, themeColor.g, themeColor.b, pulseAlpha }, .thickness = 1 });
                        }
                    }
                }

                // Dynamic Noise Particle Swarm
                int dotCount = (int)(smoothedNoiseFactor * 35.0f);
                if (maxPeak > 0.01f && dotCount < 10) dotCount = 10;
                for (int i = 0; i < dotCount; i++) {
                    float angle = i * 0.488f + animTime * (0.8f + (i % 4) * 0.4f);
                    float dist = 6.0f + std::fmod((float)(i * 9 + animTime * 30.0f), (float)(halfW + 14));
                    int dotX = shapeCX + (int)(std::cos(angle) * dist);
                    int dotY = shapeCY + (int)(std::sin(angle) * dist);
                    dotX = std::clamp(dotX, monitorX + 4, monitorX + monitorW - 4);
                    dotY = std::clamp(dotY, panelY + 14, panelY + panelH - 4);

                    uint8_t dotAlpha = (uint8_t)((110 + (i * 13 + (int)(animTime * 100)) % 145) * smoothedSignalLevel);
                    d.pixel({ dotX, dotY }, Color { 255, 245, 170, dotAlpha });
                    if (i % 2 == 0) {
                        d.pixel({ dotX + 1, dotY }, Color { 255, 255, 220, (uint8_t)(dotAlpha * 0.6f) });
                    }
                }
            }
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

        if (showEngineParams || (gridState.pads[8][2].pressed && isAnyTrackPadPressed())) {
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
