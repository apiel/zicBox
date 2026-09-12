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
        int cellW = 114;
        int cellH = UiParams::ROW_H;

        for (int i = 0; i < 12; ++i) {
            int r = i / 3;
            int c = i % 3;
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
        int seqY = 202;
        int seqW = 640 - 24;
        int gridY = seqY + 22;
        int stepW = (seqW - 20 - (15 * 3)) / 16;
        int stepH = 50;

        for (uint8_t s = 0; s < 64; ++s) {
            int r = s / 16;
            int c = s % 16;
            int sx = seqX + 10 + c * (stepW + 3);
            int sy = gridY + r * (stepH + 4);

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
        if (idx >= 0 && idx < 12) {
            hoverParamIndex = (uint8_t)idx;

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
            if (hoverParamIndex < 12) {
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

    void handleDeleteKey(bool& needRedraw)
    {
        auto& engine = worker.kickEngine;
        if (engine.wavetable.fileBrowser.count <= 0) return;

        int totalVal = (int)engine.wavetableParam.value;
        int fileCount = engine.wavetable.fileBrowser.count;
        int fileIdx = 0;
        if (engine.isSingleCycleFile()) {
            fileIdx = (totalVal % fileCount + fileCount) % fileCount;
        } else {
            int totalMax = fileCount * 64;
            totalVal = (totalVal % totalMax + totalMax) % totalMax;
            fileIdx = totalVal / 64;
        }

        std::string filePath = engine.wavetable.fileBrowser.getFilePath(fileIdx + 1);
        if (!filePath.empty()) {
            std::error_code ec;
            if (std::filesystem::exists(filePath, ec)) {
                std::filesystem::remove(filePath, ec);
                if (!ec) {
                    snprintf(saveBannerText, sizeof(saveBannerText), "DELETED FILE");
                    saveBannerTimer = 120;
                    logInfo("Deleted wavetable file: %s", filePath.c_str());
                } else {
                    snprintf(saveBannerText, sizeof(saveBannerText), "DELETE ERROR");
                    saveBannerTimer = 120;
                    logError("Failed to delete file %s: %s", filePath.c_str(), ec.message().c_str());
                }
            }
        }

        std::string folderPath = engine.findKickWavetableFolder();
        engine.wavetable.fileBrowser.openFolder(folderPath);

        int newCount = engine.wavetable.fileBrowser.count;
        if (newCount > 0) {
            int targetIdx = std::clamp(fileIdx, 0, newCount - 1);
            if (engine.isSingleCycleFile()) {
                engine.wavetableParam.max = std::max(0.0f, (float)(newCount - 1));
                engine.wavetableParam.value = (float)targetIdx;
            } else {
                engine.wavetableParam.max = std::max(0.0f, (float)(newCount * 64 - 1));
                engine.wavetableParam.value = (float)(targetIdx * 64);
            }
            engine.wavetable.open(targetIdx + 1, true);
            std::string fname = engine.wavetable.fileBrowser.getFileWithoutExtension(targetIdx + 1);
            snprintf(engine.wtName, sizeof(engine.wtName), "%s", fname.c_str());
        } else {
            engine.wavetableParam.max = 0.0f;
            engine.wavetableParam.value = 0.0f;
            snprintf(engine.wtName, sizeof(engine.wtName), "EMPTY");
        }
        needRedraw = true;
    }

    void handleWavetableStep(int direction, bool isShiftHeld, bool& needRedraw)
    {
        worker.autoMorphEnabled = false;
        Param& param = worker.kickEngine.wavetableParam;
        float stepVal = (param.step > 0.0f) ? param.step : 1.0f;
        if (isShiftHeld) {
            stepVal = worker.kickEngine.isSingleCycleFile() ? 10.0f : 64.0f;
        }
        float newVal = param.value + direction * stepVal;
        if (newVal > param.max) newVal = param.min;
        else if (newVal < param.min) newVal = param.max;

        param.set(newVal);
        needRedraw = true;
    }

    bool drawUI(Draw& d, int width, int height, bool& needFullRedraw)
    {
        (void)needFullRedraw;

        // Dark background fill
        d.filledRect({ 0, 0 }, { width, height }, { .color = { 15, 17, 26, 255 } });

        // ── Top Header Bar ──
        d.filledRect({ 0, 0 }, { width, 36 }, { .color = { 22, 25, 38, 255 } });
        d.line({ 0, 36 }, { width, 36 }, { .color = { 0, 220, 255, 120 } });

        d.text({ 14, 8 }, "zicKick2", 16, { .color = { 255, 160, 40, 255 }, .font = &PoppinsLight_16 });
        d.text({ 90, 13 }, "POTENTIOMETER MODE (OPTION 2)", 8, { .color = { 140, 165, 195, 255 }, .font = &PoppinsLight_8 });

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

        // ── Left Side: Render 12 Parameters in 3 Cols x 4 Rows ──
        int paramX = 12;
        int paramY = 46;
        int cellW = 114;

        UiParams::Style pStyle = {
            .labelColor = { 210, 222, 240, 255 },
            .valueColor = { 140, 210, 255, 255 },
            .barBgColor = { 30, 36, 54, 255 },
            .inactiveSegColor = { 50, 58, 80, 255 },
            .midLineColor = { 90, 105, 140, 255 },
            .borderColor = Color { 0, 0, 0, 0 }
        };

        for (uint8_t i = 0; i < 12; ++i) {
            int r = i / 3;
            int c = i % 3;
            int x = paramX + c * cellW;
            int y = paramY + r * UiParams::ROW_H;

            Color cardBg = (hoverParamIndex == i) ? Color { 36, 44, 68, 255 } : Color { 24, 28, 42, 255 };
            Color pColor = { 255, 160, 40, 255 };

            UiParams::param(d, worker.kickEngine.params[i], cellW, width, x, y, cardBg, pColor, pStyle);
        }

        // ── Right Side: Pitch Sweep & Waveform Visualizers ──
        int previewX = 364;
        int previewY = 46;
        int previewW = width - previewX - 12;
        int previewH = 144;

        d.filledRect({ previewX, previewY }, { previewW, previewH }, { .color = { 20, 24, 36, 255 } });
        d.rect({ previewX, previewY }, { previewW, previewH }, { .color = { 45, 55, 80, 255 } });

        // Upper Section: Pitch Envelope & FM Waveform Preview
        d.text({ previewX + 8, previewY + 4 }, "PITCH & DRIVE WAVEFORM", 8, { .color = { 255, 160, 40, 255 }, .font = &PoppinsLight_8 });

        float pitchShape = worker.kickEngine.pitchModShape.value * 0.01f;
        float fmDepthVal = worker.kickEngine.fmDepth.value * 0.01f;
        float driveVal = worker.kickEngine.drive.value * 0.01f;
        float baseFreqVal = worker.kickEngine.baseFreq.value;

        int graphXStart = previewX + 8;
        int graphYCenter = previewY + 38;
        int graphW = previewW - 16;

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

            int py = graphYCenter - static_cast<int>(val * 20.0f);
            if (px > 0) {
                d.line({ prevPx, prevPy }, { graphXStart + px, py }, { .color = { 0, 220, 255, 255 } });
            }
            prevPx = graphXStart + px;
            prevPy = py;
        }

        // Lower Section: Synthesized Parametric Waveform Visualizer
        d.text({ previewX + 8, previewY + 74 }, "PARAMETRIC SYNTH WAVEFORM", 8, { .color = { 255, 160, 40, 255 }, .font = &PoppinsLight_8 });

        int wtGraphX = previewX + 8;
        int wtGraphYCenter = previewY + 104;
        int wtGraphW = previewW - 16;

        d.line({ wtGraphX, wtGraphYCenter }, { wtGraphX + wtGraphW, wtGraphYCenter }, { .color = { 40, 50, 70, 255 } });

        int prevWtX = wtGraphX;
        int prevWtY = wtGraphYCenter;

        for (int px = 0; px < wtGraphW; px += 2) {
            float phase = (float)px / (float)wtGraphW;
            float sampleVal = worker.kickEngine.synthesizeParametricSample(phase);

            int py = wtGraphYCenter - static_cast<int>(sampleVal * 18.0f);
            if (px > 0) {
                d.line({ prevWtX, prevWtY }, { wtGraphX + px, py }, { .color = { 255, 160, 40, 255 } });
            }
            prevWtX = wtGraphX + px;
            prevWtY = py;
        }

        // Active Wavetable File Name pill at bottom
        d.filledRect({ previewX + 6, previewY + previewH - 18 }, { previewW - 12, 14 }, { .color = { 28, 34, 52, 255 } });
        d.text({ previewX + 10, previewY + previewH - 15 }, "PRESET:", 8, { .color = { 170, 185, 205, 255 }, .font = &PoppinsLight_8 });
        d.text({ previewX + 54, previewY + previewH - 15 }, worker.kickEngine.wtName, 8, { .color = { 255, 160, 40, 255 }, .font = &PoppinsLight_8 });

        // ── Bottom Panel: 64-Step Sequencer Grid (4 rows x 16 steps) ──
        int seqX = 12;
        int seqY = 202;
        int seqW = width - 24;
        int seqH = height - seqY - 12;

        d.filledRect({ seqX, seqY }, { seqW, seqH }, { .color = { 20, 24, 36, 255 } });
        d.rect({ seqX, seqY }, { seqW, seqH }, { .color = { 45, 55, 80, 255 } });

        d.text({ seqX + 10, seqY + 6 }, "64-STEP SEQUENCER MATRIX", 8, { .color = { 210, 222, 240, 255 }, .font = &PoppinsLight_8 });

        int gridY = seqY + 22;
        int stepW = (seqW - 20 - (15 * 3)) / 16;
        int stepH = 50;

        for (uint8_t s = 0; s < 64; ++s) {
            int r = s / 16;
            int c = s % 16;
            int sx = seqX + 10 + c * (stepW + 3);
            int sy = gridY + r * (stepH + 4);

            bool isActive = worker.isStepActive(s);
            bool isCurrent = (worker.playing && worker.currentStep == s);
            bool isSelectedStep = (isSeqEditMode && selectedStep == s);

            Color stepCol = isCurrent ? Color { 255, 255, 255, 255 }
                          : (isActive ? Color { 255, 160, 40, 255 }
                          : Color { 32, 38, 56, 255 });

            d.filledRect({ sx, sy }, { stepW, stepH }, { .color = stepCol });

            if (isSelectedStep) {
                d.rect({ sx - 1, sy - 1 }, { stepW + 2, stepH + 2 }, { .color = { 0, 220, 255, 255 } });
            }
        }

        return true;
    }
};
