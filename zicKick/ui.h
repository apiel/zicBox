#pragma once

#include "audioWorker.h"
#include "draw/draw.h"
#include "ui/uiParams.h"
#include "log.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

class UiKick {
public:
    AudioWorker& worker;

    uint8_t hoverParamIndex = 0;
    uint8_t selectedStep = 0;
    bool isSeqEditMode = false;
    char saveBannerText[128] = "";
    int saveBannerTimer = 0;

    UiKick(AudioWorker& worker)
        : worker(worker)
    {
    }

    void triggerSaveWavetableFrame(bool& needRedraw)
    {
        bool ok = worker.kickEngine.saveCurrentWavetableFrame();
        if (ok) {
            snprintf(saveBannerText, sizeof(saveBannerText), "SAVED FRAME");
        } else {
            snprintf(saveBannerText, sizeof(saveBannerText), "SAVE FAILED");
        }
        saveBannerTimer = 120;
        needRedraw = true;
    }

    int getParamIndexAt(int mx, int my)
    {
        int paramX = 12;
        int paramY = 46;
        int cellW = 170;
        int cellH = UiParams::ROW_H;

        for (int i = 0; i < 6; ++i) {
            int r = i / 2;
            int c = i % 2;
            int x1 = paramX + c * cellW;
            int y1 = paramY + r * cellH;
            int x2 = x1 + cellW;
            int y2 = y1 + cellH;

            if (mx >= x1 && mx < x2 && my >= y1 && my < y2) {
                return i;
            }
        }
        return -1;
    }

    int getStepIndexAt(int mx, int my)
    {
        int seqX = 12;
        int seqY = 226;
        int seqW = 640 - 24;
        int gridY = seqY + 22;
        int stepW = (seqW - 20 - (15 * 3)) / 16;
        int stepH = 20;

        for (uint8_t s = 0; s < 64; ++s) {
            int r = s / 16;
            int c = s % 16;
            int sx = seqX + 10 + c * (stepW + 3);
            int sy = gridY + r * (stepH + 3);

            if (mx >= sx && mx < sx + stepW && my >= sy && my < sy + stepH) {
                return s;
            }
        }
        return -1;
    }

    void handleMouseClick(int mx, int my, bool& needRedraw)
    {
        // Top Header Auto Morph Button (Q)
        if (my >= 5 && my <= 31 && mx >= 425 && mx <= 515) {
            worker.toggleAutoMorph();
            needRedraw = true;
            return;
        }

        int step = getStepIndexAt(mx, my);
        if (step >= 0 && step < 64) {
            worker.toggleStep((uint8_t)step);
            selectedStep = (uint8_t)step;
            needRedraw = true;
        }
    }

    void handleMouseScroll(int mx, int my, int delta, bool isShiftHeld, bool& needRedraw)
    {
        int idx = getParamIndexAt(mx, my);
        if (idx >= 0 && idx < 6) {
            hoverParamIndex = (uint8_t)idx;

            // Manual change of wavetable param (idx 0) disables auto morph mode
            if (idx == 0) {
                worker.autoMorphEnabled = false;
            }

            Param& param = worker.kickEngine.params[idx];
            float stepVal = (param.step > 0.0f) ? param.step : 1.0f;
            if (idx == 0 && isShiftHeld) {
                stepVal = worker.kickEngine.isSingleCycleFile() ? 10.0f : 64.0f;
            }
            float newVal = param.value + delta * stepVal;
            if (idx == 0) {
                if (newVal > param.max) newVal = param.min;
                else if (newVal < param.min) newVal = param.max;
            } else {
                newVal = std::clamp(newVal, param.min, param.max);
            }
            param.set(newVal);
            needRedraw = true;
        }
    }

    void handleEncoderTurn(int direction, bool isShiftHeld, bool& needRedraw)
    {
        needRedraw = true;
        if (isSeqEditMode) {
            int newStep = (int)selectedStep + direction;
            if (newStep < 0) newStep = 63;
            if (newStep > 63) newStep = 0;
            selectedStep = (uint8_t)newStep;
        } else {
            if (hoverParamIndex < 6) {
                if (hoverParamIndex == 0) {
                    worker.autoMorphEnabled = false;
                }
                Param& param = worker.kickEngine.params[hoverParamIndex];
                float stepVal = (param.step > 0.0f) ? param.step : 1.0f;
                if (hoverParamIndex == 0 && isShiftHeld) {
                    stepVal = worker.kickEngine.isSingleCycleFile() ? 10.0f : 64.0f;
                }
                float newVal = param.value + direction * stepVal;
                if (hoverParamIndex == 0) {
                    if (newVal > param.max) newVal = param.min;
                    else if (newVal < param.min) newVal = param.max;
                } else {
                    newVal = std::clamp(newVal, param.min, param.max);
                }
                param.set(newVal);
            }
        }
    }

    void handleEncoderPush(bool& needRedraw)
    {
        needRedraw = true;
        if (isSeqEditMode) {
            worker.toggleStep(selectedStep);
        }
    }

    void handleButton1(bool& needRedraw)
    {
        needRedraw = true;
        worker.triggerKick();
    }

    void handleButton2(bool& needRedraw)
    {
        needRedraw = true;
        worker.togglePlay();
    }

    void handleButton3(bool& needRedraw)
    {
        needRedraw = true;
        isSeqEditMode = !isSeqEditMode;
    }

    bool drawUI(Draw& d, int width, int height, bool& needFullRedraw)
    {
        (void)needFullRedraw;

        // Dark background fill
        d.filledRect({ 0, 0 }, { width, height }, { .color = { 15, 17, 26, 255 } });

        // ── Top Header Bar ──
        d.filledRect({ 0, 0 }, { width, 36 }, { .color = { 22, 25, 38, 255 } });
        d.line({ 0, 36 }, { width, 36 }, { .color = { 0, 220, 255, 120 } });

        d.text({ 14, 8 }, "zicKick", 16, { .color = { 0, 220, 255, 255 }, .font = &PoppinsLight_16 });
        d.text({ 85, 13 }, "DRIFT KICK WAVETABLE (64-STEP • 170 BPM)", 8, { .color = { 140, 165, 195, 255 }, .font = &PoppinsLight_8 });

        int statusX = width - 85;

        // Playback Status Indicator
        if (worker.playing) {
            d.filledRect({ statusX, 7 }, { 75, 22 }, { .color = { 0, 200, 120, 255 } });
            d.text({ statusX + 14, 13 }, "PLAYING", 8, { .color = { 10, 14, 20, 255 }, .font = &PoppinsLight_8 });
        } else {
            d.filledRect({ statusX, 7 }, { 75, 22 }, { .color = { 50, 58, 75, 255 } });
            d.text({ statusX + 14, 13 }, "STOPPED", 8, { .color = { 200, 212, 230, 255 }, .font = &PoppinsLight_8 });
        }
        statusX -= 75;

        // Gate Mute status indicator
        if (worker.isMuted) {
            d.filledRect({ statusX, 7 }, { 68, 22 }, { .color = { 220, 50, 60, 255 } });
            d.text({ statusX + 8, 13 }, "GATE MUTE", 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
            statusX -= 68;
        }

        // Repeat status indicator
        if (worker.isRepeat) {
            d.filledRect({ statusX, 7 }, { 60, 22 }, { .color = { 255, 160, 30, 255 } });
            d.text({ statusX + 10, 13 }, "REPEAT", 8, { .color = { 12, 14, 20, 255 }, .font = &PoppinsLight_8 });
        }

        // Auto Morph status/toggle button
        int autoMorphX = 425;
        if (worker.autoMorphEnabled) {
            d.filledRect({ autoMorphX, 7 }, { 90, 22 }, { .color = { 210, 0, 180, 255 } });
            d.text({ autoMorphX + 8, 13 }, "AUTO MORPH [Q]", 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
        } else {
            d.filledRect({ autoMorphX, 7 }, { 90, 22 }, { .color = { 40, 48, 65, 255 } });
            d.text({ autoMorphX + 8, 13 }, "AUTO MORPH [Q]", 8, { .color = { 150, 165, 190, 255 }, .font = &PoppinsLight_8 });
        }

        // Save Frame notification banner
        if (saveBannerTimer > 0) {
            saveBannerTimer--;
            d.filledRect({ 330, 7 }, { 88, 22 }, { .color = { 0, 200, 100, 255 } });
            d.text({ 336, 13 }, saveBannerText, 8, { .color = { 10, 14, 20, 255 }, .font = &PoppinsLight_8 });
        }

        // ── Left Side: Render 7 Parameters using UiParams::param ──
        int paramX = 12;
        int paramY = 46;
        int cellW = 170;

        UiParams::Style pStyle = {
            .labelColor = { 210, 222, 240, 255 },
            .valueColor = { 140, 210, 255, 255 },
            .barBgColor = { 30, 36, 54, 255 },
            .inactiveSegColor = { 50, 58, 80, 255 },
            .midLineColor = { 90, 105, 140, 255 },
            .borderColor = Color { 0, 0, 0, 0 }
        };

        for (uint8_t i = 0; i < 6; ++i) {
            int r = i / 2;
            int c = i % 2;
            int x = paramX + c * cellW;
            int y = paramY + r * UiParams::ROW_H;

            Color cardBg = { 24, 28, 42, 255 };
            Color pColor = { 0, 220, 255, 255 };

            UiParams::param(d, worker.kickEngine.params[i], cellW, width, x, y, cardBg, pColor, pStyle);
        }

        // ── Right Side: Pitch Sweep & Wavetable Visualizers ──
        int previewX = 368;
        int previewY = 46;
        int previewW = width - previewX - 12;
        int previewH = 168;

        d.filledRect({ previewX, previewY }, { previewW, previewH }, { .color = { 20, 24, 36, 255 } });
        d.rect({ previewX, previewY }, { previewW, previewH }, { .color = { 45, 55, 80, 255 } });

        // Upper Section: Pitch Envelope & FM Waveform Preview
        d.text({ previewX + 10, previewY + 6 }, "PITCH & DRIVE WAVEFORM", 8, { .color = { 255, 160, 40, 255 }, .font = &PoppinsLight_8 });

        float pitchShape = worker.kickEngine.pitchModShape.value * 0.01f;
        float fmDepthVal = worker.kickEngine.fmDepth.value * 0.01f;
        float driveVal = worker.kickEngine.drive.value * 0.01f;
        float baseFreqVal = worker.kickEngine.baseFreq.value;

        int graphXStart = previewX + 12;
        int graphYCenter = previewY + 50;
        int graphW = previewW - 24;

        d.line({ graphXStart, graphYCenter }, { graphXStart + graphW, graphYCenter }, { .color = { 50, 60, 85, 255 } });

        int prevPx = graphXStart;
        int prevPy = graphYCenter;

        for (int px = 0; px < graphW; px += 2) {
            float t = (float)px / (float)graphW;
            float ampEnv = (1.0f - t) * (1.0f - t);
            float rawFmEnv = std::exp(-t / 0.15f);
            float pitchEnv = std::pow(rawFmEnv, 1.0f + pitchShape * 5.0f);
            float freq = baseFreqVal * (1.0f + pitchEnv * (1.0f + fmDepthVal * 2.5f));

            float phaseVal = t * (freq * 0.08f);
            float val = std::sin(phaseVal * 6.283185f) * ampEnv;

            if (driveVal > 0.0f) {
                float boost = 1.0f + driveVal * 4.0f;
                val = std::tanh(val * boost);
            }

            int py = graphYCenter - static_cast<int>(val * 28.0f);
            if (px > 0) {
                d.line({ prevPx, prevPy }, { graphXStart + px, py }, { .color = { 0, 220, 255, 255 } });
            }
            prevPx = graphXStart + px;
            prevPy = py;
        }

        // Lower Section: Current Wavetable Morph Frame Oscilloscope Visualizer
        d.text({ previewX + 10, previewY + 92 }, "WAVETABLE MORPH FRAME", 8, { .color = { 0, 200, 150, 255 }, .font = &PoppinsLight_8 });

        int wtGraphX = previewX + 12;
        int wtGraphYCenter = previewY + 124;
        int wtGraphW = previewW - 24;

        d.line({ wtGraphX, wtGraphYCenter }, { wtGraphX + wtGraphW, wtGraphYCenter }, { .color = { 40, 50, 70, 255 } });

        int prevWtX = wtGraphX;
        int prevWtY = wtGraphYCenter;

        for (int px = 0; px < wtGraphW; px += 2) {
            float cycleLen = worker.kickEngine.getActiveCycleSampleCount();
            float phasePos = ((float)px / (float)wtGraphW) * cycleLen;
            float sampleVal = worker.kickEngine.readWaveformSample(worker.kickEngine.currentMorphVal, phasePos);

            int py = wtGraphYCenter - static_cast<int>(sampleVal * 18.0f);
            if (px > 0) {
                d.line({ prevWtX, prevWtY }, { wtGraphX + px, py }, { .color = { 0, 230, 180, 255 } });
            }
            prevWtX = wtGraphX + px;
            prevWtY = py;
        }

        // Active Wavetable File Name pill at bottom
        d.filledRect({ previewX + 10, previewY + previewH - 22 }, { previewW - 20, 16 }, { .color = { 28, 34, 52, 255 } });
        d.text({ previewX + 16, previewY + previewH - 18 }, "WT:", 8, { .color = { 170, 185, 205, 255 }, .font = &PoppinsLight_8 });
        d.text({ previewX + 44, previewY + previewH - 18 }, worker.kickEngine.wtName, 8, { .color = { 0, 200, 150, 255 }, .font = &PoppinsLight_8 });

        // ── Bottom Panel: 64-Step Sequencer Grid (4 rows x 16 steps) ──
        int seqX = 12;
        int seqY = 226;
        int seqW = width - 24;
        int seqH = 122;

        d.filledRect({ seqX, seqY }, { seqW, seqH }, { .color = { 20, 24, 36, 255 } });
        d.rect({ seqX, seqY }, { seqW, seqH }, { .color = { 45, 55, 80, 255 } });

        d.text({ seqX + 10, seqY + 6 }, "64-STEP SEQUENCER MATRIX", 8, { .color = { 210, 222, 240, 255 }, .font = &PoppinsLight_8 });

        int gridY = seqY + 22;
        int stepW = (seqW - 20 - (15 * 3)) / 16;
        int stepH = 20;

        for (uint8_t s = 0; s < 64; ++s) {
            int r = s / 16;
            int c = s % 16;
            int sx = seqX + 10 + c * (stepW + 3);
            int sy = gridY + r * (stepH + 3);

            bool isActive = worker.isStepActive(s);
            bool isCurrent = (worker.playing && worker.currentStep == s);
            bool isSelectedStep = (isSeqEditMode && selectedStep == s);

            Color stepCol = isCurrent ? Color { 255, 255, 255, 255 }
                          : (isActive ? Color { 0, 220, 255, 255 }
                          : Color { 32, 38, 56, 255 });

            d.filledRect({ sx, sy }, { stepW, stepH }, { .color = stepCol });

            if (isSelectedStep) {
                d.rect({ sx - 1, sy - 1 }, { stepW + 2, stepH + 2 }, { .color = { 255, 160, 40, 255 } });
            }
        }

        return true;
    }
};
