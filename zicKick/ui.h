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

    UiKick(AudioWorker& worker)
        : worker(worker)
    {
    }

    int getParamIndexAt(int mx, int my)
    {
        int paramX = 12;
        int paramY = 46;
        int cellW = 170;
        int cellH = UiParams::ROW_H;

        for (int i = 0; i < 7; ++i) {
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
        int step = getStepIndexAt(mx, my);
        if (step >= 0 && step < 64) {
            worker.toggleStep((uint8_t)step);
            selectedStep = (uint8_t)step;
            needRedraw = true;
        }
    }

    void handleMouseScroll(int mx, int my, int delta, bool& needRedraw)
    {
        int idx = getParamIndexAt(mx, my);
        if (idx >= 0 && idx < 7) {
            hoverParamIndex = (uint8_t)idx;
            Param& param = worker.kickEngine.params[idx];
            float stepVal = (param.step > 0.0f) ? param.step : 1.0f;
            float newVal = param.value + delta * stepVal;
            newVal = std::clamp(newVal, param.min, param.max);
            param.set(newVal);
            needRedraw = true;
        }
    }

    void handleEncoderTurn(int direction, bool& needRedraw)
    {
        needRedraw = true;
        if (isSeqEditMode) {
            int newStep = (int)selectedStep + direction;
            if (newStep < 0) newStep = 63;
            if (newStep > 63) newStep = 0;
            selectedStep = (uint8_t)newStep;
        } else {
            // Update currently hovered parameter directly
            if (hoverParamIndex < 7) {
                Param& param = worker.kickEngine.params[hoverParamIndex];
                float stepVal = (param.step > 0.0f) ? param.step : 1.0f;
                float newVal = param.value + direction * stepVal;
                newVal = std::clamp(newVal, param.min, param.max);
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
        statusX -= 60;

        // Mute status indicator
        if (worker.isMuted) {
            d.filledRect({ statusX, 7 }, { 52, 22 }, { .color = { 220, 50, 60, 255 } });
            d.text({ statusX + 8, 13 }, "MUTED", 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
            statusX -= 68;
        }

        // Repeat status indicator
        if (worker.isRepeat) {
            d.filledRect({ statusX, 7 }, { 60, 22 }, { .color = { 255, 160, 30, 255 } });
            d.text({ statusX + 10, 13 }, "REPEAT", 8, { .color = { 12, 14, 20, 255 }, .font = &PoppinsLight_8 });
        }

        // ── Left Side: Render all 7 Parameters using UiParams::param ──
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

        for (uint8_t i = 0; i < 7; ++i) {
            int r = i / 2;
            int c = i % 2;
            int x = paramX + c * cellW;
            int y = paramY + r * UiParams::ROW_H;

            Color cardBg = { 24, 28, 42, 255 };
            Color pColor = { 0, 220, 255, 255 };

            UiParams::param(d, worker.kickEngine.params[i], cellW, width, x, y, cardBg, pColor, pStyle);
        }

        // ── Right Side: Pitch & Waveform Visual Preview ──
        int previewX = 368;
        int previewY = 46;
        int previewW = width - previewX - 12;
        int previewH = 168;

        d.filledRect({ previewX, previewY }, { previewW, previewH }, { .color = { 20, 24, 36, 255 } });
        d.rect({ previewX, previewY }, { previewW, previewH }, { .color = { 45, 55, 80, 255 } });

        d.text({ previewX + 10, previewY + 8 }, "PITCH & DRIVE WAVEFORM", 8, { .color = { 255, 160, 40, 255 }, .font = &PoppinsLight_8 });

        float fmDepthVal = worker.kickEngine.fmDepth.value * 0.01f;
        float driveVal = worker.kickEngine.drive.value * 0.01f;
        float baseFreqVal = worker.kickEngine.baseFreq.value;

        int graphXStart = previewX + 12;
        int graphYCenter = previewY + 85;
        int graphW = previewW - 24;

        d.line({ graphXStart, graphYCenter }, { graphXStart + graphW, graphYCenter }, { .color = { 50, 60, 85, 255 } });

        int prevPx = graphXStart;
        int prevPy = graphYCenter;

        for (int px = 0; px < graphW; px += 2) {
            float t = (float)px / (float)graphW;
            float ampEnv = (1.0f - t) * (1.0f - t);
            float fmEnv = std::exp(-t / 0.15f);
            float freq = baseFreqVal * (1.0f + fmDepthVal * fmEnv * 2.5f);

            float phaseVal = t * (freq * 0.08f);
            float val = std::sin(phaseVal * 6.283185f) * ampEnv;

            if (driveVal > 0.0f) {
                float boost = 1.0f + driveVal * 4.0f;
                val = std::tanh(val * boost);
            }

            int py = graphYCenter - static_cast<int>(val * 50.0f);
            if (px > 0) {
                d.line({ prevPx, prevPy }, { graphXStart + px, py }, { .color = { 0, 220, 255, 255 } });
            }
            prevPx = graphXStart + px;
            prevPy = py;
        }

        // Active Wavetable file name pill
        d.filledRect({ previewX + 10, previewY + previewH - 26 }, { previewW - 20, 18 }, { .color = { 28, 34, 52, 255 } });
        d.text({ previewX + 16, previewY + previewH - 21 }, "Wavetable:", 8, { .color = { 170, 185, 205, 255 }, .font = &PoppinsLight_8 });
        d.text({ previewX + 80, previewY + previewH - 21 }, worker.kickEngine.wtName, 8, { .color = { 0, 200, 150, 255 }, .font = &PoppinsLight_8 });

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
