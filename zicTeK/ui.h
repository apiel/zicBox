#pragma once

#include "audio/sequencer/Generator.h"
#include "draw/draw.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"
#include "helpers/clamp.h"
#include "helpers/midiNote.h"
#include "zicTeK/studio.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

enum DragMode {
    DRAG_NONE,
    // Kick Drag Modes
    DRAG_SWEEP_XY,
    DRAG_VCO_MORPH_BODY,
    DRAG_VCO_MORPH_BAR,
    DRAG_CLICK_XY,
    DRAG_FM_XY,
    DRAG_FM_RATIO_BAR,
    DRAG_DRIVE_XY,
    DRAG_FOLD_BAR,
    DRAG_DURATION_BAR,
    DRAG_SEMITONE_BAR_KICK,
    DRAG_MM_FILTER_XY,
    DRAG_KICK_SUB_FREQ,
    DRAG_VOLUME_KICK,
    // Synth Drag Modes
    DRAG_SYNTH_CUTOFF_RESO_XY,
    DRAG_SYNTH_MORPH_CHAOS_XY,
    DRAG_SYNTH_ENV_XY,
    DRAG_SYNTH_MOD_TYPE,
    DRAG_SYNTH_LFO_CHAOS_XY,
    DRAG_SYNTH_FX_XY,
    DRAG_SYNTH_DELAY_XY,
    DRAG_SYNTH_CRUSH_COLOR_XY,
    DRAG_SYNTH_SEMITONE_BAR,
    DRAG_SYNTH_VOLUME,
    // Sequencer / Global
    DRAG_STEP_NOTE_KICK,
    DRAG_STEP_NOTE_SYNTH,
    DRAG_BPM
};

struct BoxRect {
    int x, y, w, h;
    bool contains(int mx, int my) const
    {
        return mx >= x && mx <= x + w && my >= y && my <= y + h;
    }
};

class UiZicTeK {
public:
    DragMode activeDrag = DRAG_NONE;
    int dragStepIdx = -1;
    int dragStartY = 0;
    int dragStartX = 0;
    float dragStartValX = 0.0f;
    int dragStartNote = 60;
    float animTime = 0.0f;
    float kickPulseLevel = 0.0f;
    float synthPulseLevel = 0.0f;

    // Interactive Widget Hit Boxes (Global)
    BoxRect playBtnRect;
    BoxRect bpmRect;

    // Track 0 (Kick) Widgets
    BoxRect sweepCurveRect;
    BoxRect vcoMorphRect;
    BoxRect clickXyRect;
    BoxRect fmXyRect;
    BoxRect fmRatioBarRect;
    BoxRect driveXyRect;
    BoxRect foldBarRect;
    BoxRect durationBarRect;
    BoxRect semitoneBarKickRect;
    BoxRect mmFilterXyRect;
    BoxRect kickSubFreqBarRect;
    BoxRect kickHardClickRect;
    BoxRect kickSubDropRect;
    BoxRect volumeKickSliderRect;

    // Track 1 (Synth) Widgets
    BoxRect synthCutoffResoXyRect;
    BoxRect synthMorphChaosXyRect;
    BoxRect synthEnvXyRect;
    BoxRect synthModTypeRect;
    BoxRect synthLfoChaosXyRect;
    BoxRect synthFxXyRect;
    BoxRect synthDelayXyRect;
    BoxRect synthColorCrushXyRect;
    BoxRect semitoneBarSynthRect;
    BoxRect volumeSynthSliderRect;

    // Sequencer Hit Boxes (Track 0: Kick)
    BoxRect rowCheckRects0[4];
    BoxRect rowGenRects0[4];
    BoxRect colEnableRects0[16];
    BoxRect stepRects0[SEQ_STEPS_TEK];

    // Sequencer Hit Boxes (Track 1: Synth)
    BoxRect rowCheckRects1[4];
    BoxRect rowGenRects1[4];
    BoxRect colEnableRects1[16];
    BoxRect stepRects1[SEQ_STEPS_TEK];

    UiZicTeK()
    {
    }

    void generateRowPattern(int trackIdx, int row)
    {
        if (row < 0 || row >= 4) return;
        int baseIdx = row * 16;

        std::vector<Step> genSeq(64);
        float p1 = Generator::rand01();
        float p2 = Generator::rand01();
        float p3 = Generator::rand01();
        Generator::generateKick(genSeq, p1, p2, p3);

        auto& targetSeq = (trackIdx == 0) ? studio.track0.sequence : studio.track1.sequence;
        for (int i = 0; i < 16; i++) {
            targetSeq[baseIdx + i] = genSeq[i];
        }
    }

    void drawHeader(Draw& d, int winW, int winH)
    {
        int headH = 38;
        d.filledRect({ 0, 0 }, { winW, headH }, { .color = { 14, 18, 26, 255 } });
        d.line({ 0, headH - 1 }, { winW, headH - 1 }, { .color = { 0, 195, 255, 255 } });
        d.line({ 0, headH - 2 }, { winW, headH - 2 }, { .color = { 0, 90, 140, 150 } });

        // Branding Title
        d.text({ 14, 8 }, "zicTeK", 16, { .color = { 0, 230, 255, 255 }, .font = &PoppinsLight_16 });
        d.text({ 94, 14 }, "2-TRACK TEKNO WORKSTATION (KICK + TEKSYNTH)", 8, { .color = { 140, 165, 195, 255 }, .font = &PoppinsLight_8 });

        // Play / Pause Transport Button
        playBtnRect = { winW - 175, 6, 68, 26 };
        Color playBg = studio.isPlaying ? Color { 0, 180, 110, 255 } : Color { 32, 42, 58, 255 };
        Color playBorder = studio.isPlaying ? Color { 0, 255, 160, 255 } : Color { 70, 90, 120, 255 };
        d.filledRect({ playBtnRect.x, playBtnRect.y }, { playBtnRect.w, playBtnRect.h }, { .color = playBg });
        d.rect({ playBtnRect.x, playBtnRect.y }, { playBtnRect.w, playBtnRect.h }, { .color = playBorder });
        d.textCentered({ playBtnRect.x + playBtnRect.w / 2, playBtnRect.y + 7 }, studio.isPlaying ? "RUNNING" : "STOPPED", 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        // Tempo / BPM Box
        bpmRect = { winW - 95, 6, 82, 26 };
        d.filledRect({ bpmRect.x, bpmRect.y }, { bpmRect.w, bpmRect.h }, { .color = { 24, 30, 44, 255 } });
        d.rect({ bpmRect.x, bpmRect.y }, { bpmRect.w, bpmRect.h }, { .color = { 255, 180, 50, 255 } });

        std::ostringstream bpmStr;
        bpmStr << std::fixed << std::setprecision(1) << studio.bpm.load() << " BPM";
        d.textCentered({ bpmRect.x + bpmRect.w / 2, bpmRect.y + 7 }, bpmStr.str(), 8, { .color = { 255, 210, 60, 255 }, .font = &PoppinsLight_8 });
    }

    static float getShapedPitch(float p, float shape)
    {
        if (shape < 0.25f) {
            float t = shape * 4.0f;
            float expDrop = std::pow(p, 3.0f);
            return CLAMP(expDrop * (1.0f - t) + p * t, 0.0f, 1.0f);
        } else if (shape < 0.50f) {
            float t = (shape - 0.25f) * 4.0f;
            float sCurve = p * p * (3.0f - 2.0f * p);
            return CLAMP(p * (1.0f - t) + sCurve * t, 0.0f, 1.0f);
        } else if (shape < 0.75f) {
            float t = (shape - 0.50f) * 4.0f;
            float sCurve = p * p * (3.0f - 2.0f * p);
            float subDive = std::pow(p, 0.4f);
            return CLAMP(sCurve * (1.0f - t) + subDive * t, 0.0f, 1.0f);
        } else {
            float t = (shape - 0.75f) * 4.0f;
            float subDive = std::pow(p, 0.4f);
            float bounce = (p * p) + 0.35f * std::sin(3.14159f * p) * std::sqrt(p);
            return CLAMP(subDive * (1.0f - t) + bounce * t, 0.0f, 1.0f);
        }
    }

    // --- TRACK 0 (MASSIVE KICK) PANEL RENDERING ---
    void drawTrack0Panel(Draw& d, int px, int py, int pw, int ph)
    {
        static auto startTime = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        animTime = std::chrono::duration<float>(now - startTime).count() * 1.5f;

        if (studio.kickPulseTrigger.exchange(false)) {
            kickPulseLevel = 1.0f;
        }

        // Panel Background & Glowing Cyan Border
        d.filledRect({ px, py }, { pw, ph }, { .color = { 12, 14, 20, 255 } });
        d.rect({ px, py }, { pw, ph }, { .color = { 0, 195, 255, 255 } });

        // Header Bar
        d.filledRect({ px + 1, py + 1 }, { pw - 2, 26 }, { .color = { 20, 28, 44, 255 } });
        d.line({ px, py + 27 }, { px + pw, py + 27 }, { .color = { 0, 195, 255, 255 } });
        d.text({ px + 10, py + 6 }, "TRACK 1: TEKKICK ENGINE", 12, { .color = { 0, 230, 255, 255 }, .font = &PoppinsLight_12 });

        // Volume Level Slider in Header
        int volW = 90;
        volumeKickSliderRect = { px + pw - volW - 10, py + 5, volW, 16 };
        d.filledRect({ volumeKickSliderRect.x, volumeKickSliderRect.y }, { volumeKickSliderRect.w, volumeKickSliderRect.h }, { .color = { 28, 38, 56, 255 } });
        int fillW = (int)(volumeKickSliderRect.w * CLAMP(studio.track0.volume, 0.0f, 1.0f));
        d.filledRect({ volumeKickSliderRect.x, volumeKickSliderRect.y }, { fillW, volumeKickSliderRect.h }, { .color = { 0, 210, 160, 255 } });
        d.rect({ volumeKickSliderRect.x, volumeKickSliderRect.y }, { volumeKickSliderRect.w, volumeKickSliderRect.h }, { .color = { 70, 95, 130, 255 } });
        std::ostringstream volStr;
        volStr << "VOL " << (int)(studio.track0.volume * 100.0f) << "%";
        d.textCentered({ volumeKickSliderRect.x + volW / 2, volumeKickSliderRect.y + 2 }, volStr.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        // Sequencer Sticking to Bottom
        int seqH = 124;
        int seqY = py + ph - seqH - 4;

        int curY = py + 34;
        int contentW = pw - 20; // 10px margin on left & right
        int colGap = 8;
        int colW = (contentW - 2 * colGap) / 3; // Equal 1/3 panel width for each column (~154px)

        int col1X = px + 10;
        int col2X = col1X + colW + colGap;
        int col3X = col2X + colW + colGap;

        // Compact Pad Height (20% height = 115px)
        int stackedH = 115;
        int vcoH = 2 * stackedH + 4 - 40; // 194px (leaves 40px under VCO morph for Duration & Pitch bars)

        // --- COLUMN 2 (CENTER): VCO MORPH (40% HEIGHT SPANNING ROW 1 + ROW 2) ---
        vcoMorphRect = { col2X, curY, colW, vcoH };

        d.filledRect({ vcoMorphRect.x, vcoMorphRect.y }, { vcoMorphRect.w, vcoMorphRect.h }, { .color = { 10, 13, 20, 255 } });
        d.rect({ vcoMorphRect.x, vcoMorphRect.y }, { vcoMorphRect.w, vcoMorphRect.h }, { .color = { 0, 195, 255, 255 } });
        d.textCentered({ vcoMorphRect.x + vcoMorphRect.w / 2, vcoMorphRect.y + 4 }, "VCO MORPH", 8, { .color = { 0, 230, 255, 255 }, .font = &PoppinsLight_8 });

        int pBarX = vcoMorphRect.x + 8;
        int pBarY = vcoMorphRect.y + 16;
        int pBarW = vcoMorphRect.w - 16;
        int pBarH = 14;

        float morphVal = CLAMP(studio.track0.kick.vcoMorph.value / 100.0f, 0.0f, 1.0f);

        d.filledRect({ pBarX, pBarY }, { pBarW, pBarH }, { .color = { 18, 25, 38, 255 } });
        int pFillW = (int)(pBarW * morphVal);
        if (pFillW > 0) {
            d.filledRect({ pBarX, pBarY }, { pFillW, pBarH }, { .color = { 0, 175, 230, 255 } });
        }
        d.rect({ pBarX, pBarY }, { pBarW, pBarH }, { .color = { 0, 220, 255, 255 } });

        int barCenterY = pBarY + pBarH / 2;
        int icon0X = pBarX + 6;
        d.filledCircle({ icon0X, barCenterY }, 3, { .color = { 255, 255, 255, 230 } });

        int icon33X = pBarX + (int)(pBarW * 0.333f);
        d.filledPolygon({ { icon33X, barCenterY - 3 }, { icon33X + 3, barCenterY + 3 }, { icon33X - 3, barCenterY + 3 } }, { .color = { 255, 255, 255, 230 } });

        int icon66X = pBarX + (int)(pBarW * 0.666f);
        d.filledPolygon({ { icon66X - 3, barCenterY + 3 }, { icon66X + 3, barCenterY - 3 }, { icon66X + 3, barCenterY + 3 } }, { .color = { 255, 255, 255, 230 } });

        int icon100X = pBarX + pBarW - 6;
        d.filledRect({ icon100X - 3, barCenterY - 3 }, { 6, 6 }, { .color = { 255, 255, 255, 230 } });

        int handleX = std::clamp(pBarX + pFillW, pBarX + 1, pBarX + pBarW - 1);
        d.line({ handleX, pBarY - 1 }, { handleX, pBarY + pBarH + 1 }, { .color = { 255, 255, 100, 255 }, .thickness = 2 });

        // Center piece animation & 3-arc curvature flattening
        int cx = vcoMorphRect.x + vcoMorphRect.w / 2;
        int topPad = 34 + 14;
        int botPad = 14;
        int availW = vcoMorphRect.w - 20;
        int availH = vcoMorphRect.h - (topPad + botPad);
        int cy = vcoMorphRect.y + topPad + availH / 2;

        int maxOuterR = std::min(availW / 2, availH / 2);
        int halfSize = (int)((maxOuterR - 6) / 1.35f);
        halfSize = std::clamp(halfSize, 16, 50);
        float R = (float)halfSize;

        float clickAmt = studio.track0.kick.kickClickAmt.value;
        float durMs = studio.track0.kick.duration.value;

        float decayRate = 12.0f / (CLAMP(durMs, 50.0f, 1500.0f) + 50.0f);
        kickPulseLevel = std::max(0.0f, kickPulseLevel - decayRate);

        if (kickPulseLevel > 0.01f) {
            int baseR = (int)(R * 0.8f);
            for (int r = 0; r < 3; r++) {
                float pFactor = kickPulseLevel - (r * 0.22f);
                if (pFactor > 0.0f) {
                    int radius = (int)(baseR + (1.0f - pFactor) * (R * 0.9f) + r * 6);
                    uint8_t alpha = (uint8_t)(pFactor * 140.0f);
                    d.circle({ cx, cy }, radius, { .color = { 0, 195, 255, alpha } });
                }
            }
        }

        Point triBL = { cx - halfSize, cy + halfSize };
        Point triBR = { cx + halfSize, cy + halfSize };
        Point triTop = { cx, cy - halfSize };

        Point sqBL = { cx - halfSize, cy + halfSize };
        Point sqBR = { cx + halfSize, cy + halfSize };
        Point sqTR = { cx + halfSize, cy - halfSize };
        Point sqTL = { cx - halfSize, cy - halfSize };

        std::vector<Point> morphShape;

        if (morphVal <= 0.333f) {
            float s = morphVal / 0.333f;
            const int SAMPLES_PER_SIDE = 14;
            Point vPts[3] = { triTop, triBR, triBL };
            float vAngles[3] = { -(float)M_PI_2, -(float)M_PI_2 + (2.0f * (float)M_PI / 3.0f), -(float)M_PI_2 + (4.0f * (float)M_PI / 3.0f) };

            for (int side = 0; side < 3; side++) {
                Point pStart = vPts[side];
                Point pEnd = vPts[(side + 1) % 3];
                float aStart = vAngles[side];
                for (int k = 0; k < SAMPLES_PER_SIDE; k++) {
                    float u = (float)k / (float)SAMPLES_PER_SIDE;
                    float lineX = (1.0f - u) * pStart.x + u * pEnd.x;
                    float lineY = (1.0f - u) * pStart.y + u * pEnd.y;
                    float angle = aStart + u * (2.0f * (float)M_PI / 3.0f);
                    float circX = cx + (R * 1.15f) * std::cos(angle);
                    float circY = cy + (R * 1.15f) * std::sin(angle);
                    int mx = (int)((1.0f - s) * circX + s * lineX);
                    int my = (int)((1.0f - s) * circY + s * lineY);
                    morphShape.push_back({ mx, my });
                }
            }
        } else if (morphVal <= 0.666f) {
            float t = (morphVal - 0.333f) / 0.333f;
            int curTopX = (int)((1.0f - t) * triTop.x + t * sqTR.x);
            int curTopY = (int)((1.0f - t) * triTop.y + t * sqTR.y);
            morphShape = { triBL, { curTopX, curTopY }, triBR };
        } else {
            float u = (morphVal - 0.666f) / 0.334f;
            int curTLX = (int)((1.0f - u) * sqTR.x + u * sqTL.x);
            morphShape = { sqBL, { curTLX, sqTL.y }, sqTR, sqBR };
        }

        // FM Orbiting Shell
        float fmVal = CLAMP(studio.track0.kick.fmDepth.value / 100.0f, 0.0f, 1.0f);
        if (fmVal > 0.01f) {
            float rotAngle = animTime * (1.0f + fmVal * 8.0f);
            int numShellPts = 5;
            std::vector<Point> modShell;
            for (int i = 0; i < numShellPts; i++) {
                float a = rotAngle + i * (6.28318f / numShellPts);
                float radiusW = (R + 10.0f) + std::sin(a * 3.0f + animTime * 4.0f) * (fmVal * (R * 0.35f));
                float radiusH = (R + 10.0f) + std::cos(a * 2.0f + animTime * 3.0f) * (fmVal * (R * 0.3f));
                int mx = cx + (int)(std::cos(a) * radiusW);
                int my = cy + (int)(std::sin(a) * radiusH);
                modShell.push_back({ mx, my });
            }
            uint8_t shellAlpha = (uint8_t)(80 + fmVal * 165.0f);
            d.lines(modShell, { .color = { 0, 195, 255, shellAlpha }, .thickness = 1 });
            d.line(modShell.back(), modShell.front(), { .color = { 0, 195, 255, shellAlpha }, .thickness = 1 });
        }

        float drv = CLAMP(studio.track0.kick.drive.value / 100.0f, 0.0f, 1.0f);
        Color themeCol = { 0, 195, 255, 255 };
        Color shapeStroke = themeCol;
        if (drv > 0.01f) {
            shapeStroke = Color {
                (uint8_t)(themeCol.r * (1.0f - drv) + 255 * drv),
                (uint8_t)(themeCol.g * (1.0f - drv) + 120 * drv),
                (uint8_t)(themeCol.b * (1.0f - drv) + 40 * drv),
                255
            };
        }
        int strokeThickness = (drv > 0.35f) ? 2 : 1;
        uint8_t fillAlpha = (uint8_t)(60 + drv * 60.0f);

        d.filledPolygon(morphShape, { .color = { shapeStroke.r, shapeStroke.g, shapeStroke.b, fillAlpha } });
        d.lines(morphShape, { .color = shapeStroke, .thickness = strokeThickness });
        d.line(morphShape.back(), morphShape.front(), { .color = shapeStroke, .thickness = strokeThickness });

        // Click Noise Dot Swarm
        int dotCount = (int)(clickAmt * 0.65f);
        for (int i = 0; i < dotCount; i++) {
            float angle = i * 0.488f + animTime * (0.6f + (i % 4) * 0.3f);
            float dist = (R * 0.35f) + std::fmod((float)(i * 7 + animTime * 20.0f), R * 1.3f);
            int dotX = cx + (int)(std::cos(angle) * dist);
            int dotY = cy + (int)(std::sin(angle) * dist);
            dotX = std::clamp(dotX, vcoMorphRect.x + 6, vcoMorphRect.x + vcoMorphRect.w - 6);
            dotY = std::clamp(dotY, vcoMorphRect.y + 34, vcoMorphRect.y + vcoMorphRect.h - 18);
            uint8_t dotAlpha = (uint8_t)(110 + (i * 13 + (int)(animTime * 100)) % 145);
            d.pixel({ dotX, dotY }, Color { 255, 245, 170, dotAlpha });
        }

        // --- DURATION BAR & BIPOLAR PITCH OFFSET BAR FITTED UNDER VCO MORPH (ORIGINAL COMPACT SIZE & ALIGNED) ---
        int durY = curY + vcoH + 4;
        int pitchY = durY + 16 + 4;

        // 1. DURATION BAR (16px ORIGINAL HEIGHT)
        durationBarRect = { col2X, durY, colW, 16 };
        d.filledRect({ durationBarRect.x, durationBarRect.y }, { durationBarRect.w, durationBarRect.h }, { .color = { 16, 24, 36, 255 } });

        Param& durP = studio.track0.kick.duration;
        float durNorm = CLAMP((durP.value - durP.min) / (durP.max - durP.min), 0.0f, 1.0f);
        int durFillW = (int)(durationBarRect.w * durNorm);
        if (durFillW > 0) {
            d.filledRect({ durationBarRect.x, durationBarRect.y }, { durFillW, durationBarRect.h }, { .color = { 0, 180, 135, 255 } });
        }
        d.rect({ durationBarRect.x, durationBarRect.y }, { durationBarRect.w, durationBarRect.h }, { .color = { 0, 220, 160, 255 } });

        std::ostringstream durTxt;
        durTxt << "DURATION " << (int)durP.value << " ms";
        d.textCentered({ durationBarRect.x + durationBarRect.w / 2 + 1, durationBarRect.y + 4 }, durTxt.str(), 8, { .color = { 0, 0, 0, 255 }, .font = &PoppinsLight_8 });
        d.textCentered({ durationBarRect.x + durationBarRect.w / 2, durationBarRect.y + 3 }, durTxt.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        // 2. BIPOLAR CENTERED PITCH BAR (16px ORIGINAL HEIGHT, ALIGNED WITH BOTTOM OF CLICK AND DRIVE AT 304px!)
        semitoneBarKickRect = { col2X, pitchY, colW, 16 };
        d.filledRect({ semitoneBarKickRect.x, semitoneBarKickRect.y }, { semitoneBarKickRect.w, semitoneBarKickRect.h }, { .color = { 14, 18, 28, 255 } });
        d.rect({ semitoneBarKickRect.x, semitoneBarKickRect.y }, { semitoneBarKickRect.w, semitoneBarKickRect.h }, { .color = { 255, 180, 40, 255 } });

        int curSemi = studio.track0.kick.semitoneOffset.load();
        curSemi = std::clamp(curSemi, -12, 12);

        int sBarX = semitoneBarKickRect.x + 2;
        int sBarW = semitoneBarKickRect.w - 4;
        int sBarH = semitoneBarKickRect.h - 4;
        int sBarY = semitoneBarKickRect.y + 2;

        int centerW = 4;
        int sideW = (semitoneBarKickRect.w - 4 - centerW) / 2;
        float segW = (float)sideW / 12.0f;

        for (int i = 0; i < 12; i++) {
            int semiValForSeg = i - 12;
            int sx = sBarX + (int)(i * segW);
            int sw = std::max(1, (int)segW - 1);
            Color segCol = { 26, 32, 46, 255 };
            if (curSemi < 0 && semiValForSeg >= curSemi) {
                float t = (float)(semiValForSeg + 13) / 12.0f;
                segCol = Color { 0, (uint8_t)(160 + t * 60), (uint8_t)(210 + t * 45), 255 };
            }
            d.filledRect({ sx, sBarY }, { sw, sBarH }, { .color = segCol });
        }

        int centerX = sBarX + sideW;
        d.filledRect({ centerX, sBarY }, { centerW, sBarH }, { .color = Color { 150, 155, 170, 255 } });

        int rightStartX = centerX + centerW;
        for (int i = 0; i < 12; i++) {
            int semiValForSeg = i + 1;
            int sx = rightStartX + (int)(i * segW);
            int sw = std::max(1, (int)segW - 1);
            Color segCol = { 26, 32, 46, 255 };
            if (curSemi > 0 && semiValForSeg <= curSemi) {
                float t = (float)semiValForSeg / 12.0f;
                segCol = Color { 255, (uint8_t)(210 - t * 70), 30, 255 };
            }
            d.filledRect({ sx, sBarY }, { sw, sBarH }, { .color = segCol });
        }

        std::ostringstream semiTxt;
        semiTxt << "PITCH " << (curSemi > 0 ? "+" : "") << curSemi << " st";
        d.textCentered({ semitoneBarKickRect.x + semitoneBarKickRect.w / 2 + 1, semitoneBarKickRect.y + 4 }, semiTxt.str(), 8, { .color = { 0, 0, 0, 255 }, .font = &PoppinsLight_8 });
        d.textCentered({ semitoneBarKickRect.x + semitoneBarKickRect.w / 2, semitoneBarKickRect.y + 3 }, semiTxt.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        // --- COLUMN 1 (LEFT ROW 1 - 20% HEIGHT = 115px): SWEEP PITCH XY PAD ---
        sweepCurveRect = { col1X, curY, colW, stackedH };
        d.filledRect({ sweepCurveRect.x, sweepCurveRect.y }, { sweepCurveRect.w, sweepCurveRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ sweepCurveRect.x, sweepCurveRect.y }, { sweepCurveRect.w, sweepCurveRect.h }, { .color = { 255, 160, 40, 255 } });
        d.text({ sweepCurveRect.x + 6, sweepCurveRect.y + 4 }, "SWEEP", 8, { .color = { 255, 180, 50, 255 }, .font = &PoppinsLight_8 });

        d.line({ sweepCurveRect.x + sweepCurveRect.w / 2, sweepCurveRect.y + 14 }, { sweepCurveRect.x + sweepCurveRect.w / 2, sweepCurveRect.y + sweepCurveRect.h - 4 }, { .color = { 45, 36, 28, 255 } });
        d.line({ sweepCurveRect.x + 4, sweepCurveRect.y + 14 + (sweepCurveRect.h - 18) / 2 }, { sweepCurveRect.x + sweepCurveRect.w - 4, sweepCurveRect.y + 14 + (sweepCurveRect.h - 18) / 2 }, { .color = { 45, 36, 28, 255 } });

        float shpNorm = studio.track0.kick.sweepShp.value * 0.01f;
        float depthNorm = studio.track0.kick.sweepDepth.value * 0.01f;
        std::vector<Point> curvePts;
        int steps = 35;
        for (int i = 0; i <= steps; i++) {
            float t = (float)i / (float)steps;
            float decayFactor = std::exp(-t * (4.0f / (0.15f + depthNorm * 0.85f)));
            float pitchVal = getShapedPitch(decayFactor, shpNorm) * depthNorm;
            int cxPt = sweepCurveRect.x + 6 + (int)(t * (sweepCurveRect.w - 12));
            int cyPt = sweepCurveRect.y + sweepCurveRect.h - 8 - (int)(pitchVal * (sweepCurveRect.h - 22));
            curvePts.push_back({ cxPt, cyPt });
        }
        if (curvePts.size() > 1) {
            d.lines(curvePts, { .color = { 255, 170, 40, 255 }, .thickness = 2 });
        }

        int swpTargetX = sweepCurveRect.x + 6 + (int)(shpNorm * (sweepCurveRect.w - 12));
        int swpTargetY = sweepCurveRect.y + sweepCurveRect.h - 6 - (int)(depthNorm * (sweepCurveRect.h - 20));

        d.line({ swpTargetX - 6, swpTargetY }, { swpTargetX + 6, swpTargetY }, { .color = { 255, 180, 50, 255 } });
        d.line({ swpTargetX, swpTargetY - 6 }, { swpTargetX, swpTargetY + 6 }, { .color = { 255, 180, 50, 255 } });
        d.filledCircle({ swpTargetX, swpTargetY }, 4, { .color = { 255, 220, 90, 255 } });

        if (kickPulseLevel > 0.01f) {
            uint8_t pulseAlpha = (uint8_t)(kickPulseLevel * 220.0f);
            d.lines(curvePts, { .color = { 255, 230, 100, pulseAlpha }, .thickness = 3 });

            float progress = CLAMP(1.0f - kickPulseLevel, 0.0f, 1.0f);
            int ballIdx = (int)(progress * steps);
            ballIdx = std::clamp(ballIdx, 0, (int)curvePts.size() - 1);
            Point ballPt = curvePts[ballIdx];

            d.filledCircle(ballPt, 5, { .color = { 255, 255, 200, 255 } });
            d.circle(ballPt, 7, { .color = { 255, 180, 50, pulseAlpha } });
        }

        // --- COLUMN 1 (LEFT ROW 2 - 20% HEIGHT = 115px): CLICK RADAR XY TARGET PAD ---
        int row2Y = curY + stackedH + 4;
        clickXyRect = { col1X, row2Y, colW, stackedH };
        d.filledRect({ clickXyRect.x, clickXyRect.y }, { clickXyRect.w, clickXyRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ clickXyRect.x, clickXyRect.y }, { clickXyRect.w, clickXyRect.h }, { .color = { 255, 80, 120, 255 } });
        d.text({ clickXyRect.x + 6, clickXyRect.y + 4 }, "CLICK", 8, { .color = { 255, 100, 140, 255 }, .font = &PoppinsLight_8 });

        d.line({ clickXyRect.x + clickXyRect.w / 2, clickXyRect.y + 14 }, { clickXyRect.x + clickXyRect.w / 2, clickXyRect.y + clickXyRect.h - 4 }, { .color = { 40, 30, 48, 255 } });
        d.line({ clickXyRect.x + 4, clickXyRect.y + 14 + (clickXyRect.h - 18) / 2 }, { clickXyRect.x + clickXyRect.w - 4, clickXyRect.y + 14 + (clickXyRect.h - 18) / 2 }, { .color = { 40, 30, 48, 255 } });

        float amtNorm = studio.track0.kick.kickClickAmt.value * 0.01f;
        float decNorm = (studio.track0.kick.kickClickDecay.value - 1.0f) / 99.0f;
        int targetX = clickXyRect.x + 6 + (int)(amtNorm * (clickXyRect.w - 12));
        int targetY = clickXyRect.y + clickXyRect.h - 6 - (int)(decNorm * (clickXyRect.h - 20));

        d.line({ targetX - 6, targetY }, { targetX + 6, targetY }, { .color = { 255, 100, 140, 255 } });
        d.line({ targetX, targetY - 6 }, { targetX, targetY + 6 }, { .color = { 255, 100, 140, 255 } });
        d.filledCircle({ targetX, targetY }, 3, { .color = { 255, 180, 200, 255 } });

        std::ostringstream xyTxt;
        xyTxt << (int)studio.track0.kick.kickClickAmt.value << "%/" << (int)studio.track0.kick.kickClickDecay.value << "ms";
        d.textRight({ clickXyRect.x + clickXyRect.w - 6, clickXyRect.y + 4 }, xyTxt.str(), 8, { .color = { 255, 150, 180, 255 }, .font = &PoppinsLight_8 });

        if (kickPulseLevel > 0.01f) {
            uint8_t pulseAlpha = (uint8_t)(kickPulseLevel * 220.0f);
            d.rect({ clickXyRect.x, clickXyRect.y }, { clickXyRect.w, clickXyRect.h }, { .color = { 255, 150, 180, pulseAlpha } });

            for (int r = 0; r < 3; r++) {
                float pFactor = kickPulseLevel - (r * 0.22f);
                if (pFactor > 0.0f) {
                    int radius = (int)(4.0f + (1.0f - pFactor) * 24.0f + r * 5);
                    uint8_t rAlpha = (uint8_t)(pFactor * 190.0f);
                    d.circle({ targetX, targetY }, radius, { .color = { 255, 100, 160, rAlpha } });
                }
            }

            int particleCount = (int)(24 + amtNorm * 24.0f);
            float burstExp = 1.0f - kickPulseLevel;

            for (int i = 0; i < particleCount; i++) {
                float angle = i * 0.2618f + (i * 1.37f);
                float speed = 12.0f + (float)((i * 17) % 35);
                float dist = burstExp * speed;

                int pxPt = targetX + (int)(std::cos(angle) * dist);
                int pyPt = targetY + (int)(std::sin(angle) * dist);

                pxPt = std::clamp(pxPt, clickXyRect.x + 4, clickXyRect.x + clickXyRect.w - 4);
                pyPt = std::clamp(pyPt, clickXyRect.y + 14, clickXyRect.y + clickXyRect.h - 4);

                uint8_t pAlpha = (uint8_t)(kickPulseLevel * (140 + (i * 19) % 115));
                Color pColor = (i % 2 == 0) ? Color { 255, 240, 245, pAlpha } : Color { 255, 140, 190, pAlpha };
                d.pixel({ pxPt, pyPt }, pColor);
            }
            d.filledCircle({ targetX, targetY }, 5, { .color = { 255, 235, 245, (uint8_t)(kickPulseLevel * 255.0f) } });
        }

        // --- COLUMN 1 (LEFT ROW 3 - 20% HEIGHT = 115px UNDER CLICK): DUAL FILTER XY PAD (LEFT: 303 LP | RIGHT: HP) ---
        int row3Y = row2Y + stackedH + 4;
        mmFilterXyRect = { col1X, row3Y, colW, stackedH };
        d.filledRect({ mmFilterXyRect.x, mmFilterXyRect.y }, { mmFilterXyRect.w, mmFilterXyRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ mmFilterXyRect.x, mmFilterXyRect.y }, { mmFilterXyRect.w, mmFilterXyRect.h }, { .color = { 255, 90, 160, 255 } });
        d.text({ mmFilterXyRect.x + 6, mmFilterXyRect.y + 4 }, "FILTER (303 LP | HP)", 8, { .color = { 255, 120, 180, 255 }, .font = &PoppinsLight_8 });

        // Center line (Bypass)
        int mmCenterX = mmFilterXyRect.x + mmFilterXyRect.w / 2;
        d.line({ mmCenterX, mmFilterXyRect.y + 14 }, { mmCenterX, mmFilterXyRect.y + mmFilterXyRect.h - 4 }, { .color = { 60, 40, 70, 255 } });

        d.text({ mmFilterXyRect.x + 4, mmFilterXyRect.y + mmFilterXyRect.h - 10 }, "303 LP", 8, { .color = { 255, 120, 40, 255 }, .font = &PoppinsLight_8 });
        d.textRight({ mmFilterXyRect.x + mmFilterXyRect.w - 4, mmFilterXyRect.y + mmFilterXyRect.h - 10 }, "HP", 8, { .color = { 0, 195, 255, 255 }, .font = &PoppinsLight_8 });
        d.textCentered({ mmCenterX, mmFilterXyRect.y + 14 }, "RESO", 8, { .color = { 160, 100, 180, 255 }, .font = &PoppinsLight_8 });

        float cutVal = studio.track0.kick.filterCutoff.value; // -100 to +100
        float resoVal = studio.track0.kick.filterReso.value; // 0 to 100
        float cutNorm = (cutVal + 100.0f) / 200.0f; // 0 to 1
        float resoNorm = resoVal * 0.01f; // 0 to 1

        int mmTargetX = mmFilterXyRect.x + 6 + (int)(cutNorm * (mmFilterXyRect.w - 12));
        int mmTargetY = mmFilterXyRect.y + mmFilterXyRect.h - 6 - (int)(resoNorm * (mmFilterXyRect.h - 20));

        d.line({ mmTargetX - 6, mmTargetY }, { mmTargetX + 6, mmTargetY }, { .color = { 255, 140, 180, 255 } });
        d.line({ mmTargetX, mmTargetY - 6 }, { mmTargetX, mmTargetY + 6 }, { .color = { 255, 140, 180, 255 } });
        d.filledCircle({ mmTargetX, mmTargetY }, 4, { .color = { 255, 220, 240, 255 } });

        std::ostringstream mmTxt;
        if (cutVal < -0.5f) {
            mmTxt << "LP " << (int)std::abs(cutVal) << "%";
        } else if (cutVal > 0.5f) {
            mmTxt << "HP " << (int)cutVal << "%";
        } else {
            mmTxt << "FLAT";
        }
        d.textRight({ mmFilterXyRect.x + mmFilterXyRect.w - 6, mmFilterXyRect.y + 4 }, mmTxt.str(), 8, { .color = { 255, 170, 200, 255 }, .font = &PoppinsLight_8 });

        if (kickPulseLevel > 0.01f) {
            uint8_t pulseAlpha = (uint8_t)(kickPulseLevel * 220.0f);
            d.circle({ mmTargetX, mmTargetY }, 8, { .color = { 255, 120, 180, pulseAlpha } });
        }

        // --- COLUMN 3 (RIGHT ROW 1 - 20% HEIGHT = 115px): FM SYNTHESIS 2D XY PAD & SEGMENTED RATIO BAR ---
        fmXyRect = { col3X, curY, colW, stackedH };
        d.filledRect({ fmXyRect.x, fmXyRect.y }, { fmXyRect.w, fmXyRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ fmXyRect.x, fmXyRect.y }, { fmXyRect.w, fmXyRect.h }, { .color = { 180, 100, 255, 255 } });
        d.text({ fmXyRect.x + 6, fmXyRect.y + 4 }, "FM SYNTHESIS", 8, { .color = { 200, 130, 255, 255 }, .font = &PoppinsLight_8 });

        int padBodyH = fmXyRect.h - 18;
        d.line({ fmXyRect.x + fmXyRect.w / 2, fmXyRect.y + 14 }, { fmXyRect.x + fmXyRect.w / 2, fmXyRect.y + padBodyH - 4 }, { .color = { 38, 28, 52, 255 } });

        float fmDepthNorm = studio.track0.kick.fmDepth.value * 0.01f;
        float fmSnapNorm = (studio.track0.kick.fmSnap.value - 2.0f) / 148.0f;
        int fmTargetX = fmXyRect.x + 6 + (int)(fmDepthNorm * (fmXyRect.w - 12));
        int fmTargetY = fmXyRect.y + padBodyH - 6 - (int)(fmSnapNorm * (padBodyH - 20));

        d.filledCircle({ fmTargetX, fmTargetY }, 4, { .color = { 230, 170, 255, 255 } });

        int barX = fmXyRect.x;
        int barY = fmXyRect.y + padBodyH;
        int barW = fmXyRect.w;
        int barH = 18;
        fmRatioBarRect = { barX, barY, barW, barH };

        d.filledRect({ barX, barY }, { barW, barH }, { .color = { 12, 16, 26, 255 } });
        d.line({ barX, barY }, { barX + barW, barY }, { .color = { 180, 100, 255, 255 } });

        const int NUM_SEGMENTS = 31;
        float curRatio = studio.track0.kick.fmRatio.value;
        int activeSegmentIdx = (int)std::round((curRatio - 0.5f) / 0.25f);
        activeSegmentIdx = std::clamp(activeSegmentIdx, 0, NUM_SEGMENTS - 1);
        float segWidth = (float)(barW - 4) / (float)NUM_SEGMENTS;

        for (int seg = 0; seg < NUM_SEGMENTS; seg++) {
            int sx = barX + 2 + (int)(seg * segWidth);
            int sw = std::max(1, (int)segWidth - 1);
            Color segCol = (seg <= activeSegmentIdx) ? Color { 180, 120, 255, 255 } : Color { 28, 36, 52, 255 };
            d.filledRect({ sx, barY + 3 }, { sw, barH - 6 }, { .color = segCol });
        }

        std::ostringstream ratioTxt;
        ratioTxt << "RATIO " << std::fixed << std::setprecision(2) << curRatio << "x";
        d.textCentered({ barX + barW / 2, barY + 4 }, ratioTxt.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        // --- COLUMN 3 (RIGHT ROW 2 - 20% HEIGHT): DRIVE & BASS BOOST XY PAD & WAVEFOLD BAR ---
        driveXyRect = { col3X, row2Y, colW, stackedH };
        d.filledRect({ driveXyRect.x, driveXyRect.y }, { driveXyRect.w, driveXyRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ driveXyRect.x, driveXyRect.y }, { driveXyRect.w, driveXyRect.h }, { .color = { 255, 100, 40, 255 } });
        d.text({ driveXyRect.x + 6, driveXyRect.y + 4 }, "DRIVE & BASS", 8, { .color = { 255, 130, 60, 255 }, .font = &PoppinsLight_8 });

        int drvBodyH = driveXyRect.h - 18;
        float drvNorm = studio.track0.kick.drive.value * 0.01f;
        float boostNorm = studio.track0.kick.bassBoost.value * 0.01f;
        int driveTargetX = driveXyRect.x + 6 + (int)(drvNorm * (driveXyRect.w - 12));
        int driveTargetY = driveXyRect.y + drvBodyH - 6 - (int)(boostNorm * (drvBodyH - 20));

        d.filledCircle({ driveTargetX, driveTargetY }, 4, { .color = { 255, 200, 90, 255 } });

        int fBarX = driveXyRect.x;
        int fBarY = driveXyRect.y + drvBodyH;
        int fBarW = driveXyRect.w;
        int fBarH = 18;
        foldBarRect = { fBarX, fBarY, fBarW, fBarH };

        d.filledRect({ fBarX, fBarY }, { fBarW, fBarH }, { .color = { 22, 14, 16, 255 } });
        d.line({ fBarX, fBarY }, { fBarX + fBarW, fBarY }, { .color = { 255, 100, 40, 255 } });

        const int FOLD_SEGMENTS = 21;
        float curFold = studio.track0.kick.fold.value;
        int activeFoldIdx = (int)std::round((curFold / 100.0f) * (FOLD_SEGMENTS - 1));
        activeFoldIdx = std::clamp(activeFoldIdx, 0, FOLD_SEGMENTS - 1);
        float fSegWidth = (float)(fBarW - 4) / (float)FOLD_SEGMENTS;

        for (int seg = 0; seg < FOLD_SEGMENTS; seg++) {
            int sx = fBarX + 2 + (int)(seg * fSegWidth);
            int sw = std::max(1, (int)fSegWidth - 1);
            Color segCol = (seg <= activeFoldIdx) ? Color { 255, 120, 40, 255 } : Color { 48, 28, 30, 255 };
            d.filledRect({ sx, fBarY + 3 }, { sw, fBarH - 6 }, { .color = segCol });
        }

        std::ostringstream foldTxt;
        foldTxt << "FOLD " << (int)curFold << "%";
        d.textCentered({ fBarX + fBarW / 2, fBarY + 4 }, foldTxt.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        // --- COLUMN 3 (RIGHT ROW 3 - 20% HEIGHT): SUB FREQ & PERFORMANCE MACROS ---
        kickSubFreqBarRect = { col3X, row3Y, colW, stackedH };
        d.filledRect({ kickSubFreqBarRect.x, kickSubFreqBarRect.y }, { kickSubFreqBarRect.w, kickSubFreqBarRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ kickSubFreqBarRect.x, kickSubFreqBarRect.y }, { kickSubFreqBarRect.w, kickSubFreqBarRect.h }, { .color = { 0, 220, 160, 255 } });
        d.text({ kickSubFreqBarRect.x + 6, kickSubFreqBarRect.y + 4 }, "SUB FREQ", 8, { .color = { 0, 240, 180, 255 }, .font = &PoppinsLight_8 });

        Param& subP = studio.track0.kick.baseFreq;
        float subNorm = (subP.value - subP.min) / (subP.max - subP.min);
        int subFillW = (int)((kickSubFreqBarRect.w - 12) * subNorm);
        d.filledRect({ kickSubFreqBarRect.x + 6, kickSubFreqBarRect.y + 20 }, { kickSubFreqBarRect.w - 12, 22 }, { .color = { 18, 26, 38, 255 } });
        if (subFillW > 0) {
            d.filledRect({ kickSubFreqBarRect.x + 6, kickSubFreqBarRect.y + 20 }, { subFillW, 22 }, { .color = { 0, 200, 150, 255 } });
        }
        d.rect({ kickSubFreqBarRect.x + 6, kickSubFreqBarRect.y + 20 }, { kickSubFreqBarRect.w - 12, 22 }, { .color = { 0, 230, 170, 255 } });

        std::ostringstream subTxt;
        subTxt << (int)subP.value << " Hz";
        d.textCentered({ kickSubFreqBarRect.x + kickSubFreqBarRect.w / 2, kickSubFreqBarRect.y + 24 }, subTxt.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        kickHardClickRect = { kickSubFreqBarRect.x + 6, kickSubFreqBarRect.y + 50, (kickSubFreqBarRect.w - 16) / 2, 24 };
        bool isHard = studio.track0.kick.isHardClickActive.load();
        d.filledRect({ kickHardClickRect.x, kickHardClickRect.y }, { kickHardClickRect.w, kickHardClickRect.h }, { .color = isHard ? Color { 255, 80, 120, 255 } : Color { 28, 34, 46, 255 } });
        d.rect({ kickHardClickRect.x, kickHardClickRect.y }, { kickHardClickRect.w, kickHardClickRect.h }, { .color = isHard ? Color { 255, 150, 180, 255 } : Color { 60, 75, 95, 255 } });
        d.textCentered({ kickHardClickRect.x + kickHardClickRect.w / 2, kickHardClickRect.y + 7 }, "HARD", 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        kickSubDropRect = { kickSubFreqBarRect.x + 10 + kickHardClickRect.w, kickSubFreqBarRect.y + 50, (kickSubFreqBarRect.w - 16) / 2, 24 };
        bool isDrop = studio.track0.kick.isSubDropActive.load();
        d.filledRect({ kickSubDropRect.x, kickSubDropRect.y }, { kickSubDropRect.w, kickSubDropRect.h }, { .color = isDrop ? Color { 0, 180, 220, 255 } : Color { 28, 34, 46, 255 } });
        d.rect({ kickSubDropRect.x, kickSubDropRect.y }, { kickSubDropRect.w, kickSubDropRect.h }, { .color = isDrop ? Color { 100, 220, 255, 255 } : Color { 60, 75, 95, 255 } });
        d.textCentered({ kickSubDropRect.x + kickSubDropRect.w / 2, kickSubDropRect.y + 7 }, "SUB DIVE", 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        // --- 64-STEP SEQUENCER STICKING TO THE BOTTOM ---
        drawSequencer(d, px, seqY, pw, 0);
    }

    // --- TRACK 1 (TEKSYNTH) PANEL RENDERING (ALL 20% HEIGHT PADS IN 3x3 GRID) ---
    void drawTrack1Panel(Draw& d, int px, int py, int pw, int ph)
    {
        if (studio.synthPulseTrigger.exchange(false)) {
            synthPulseLevel = 1.0f;
        }

        // Panel Background & Orange/Gold Border
        d.filledRect({ px, py }, { pw, ph }, { .color = { 12, 14, 20, 255 } });
        d.rect({ px, py }, { pw, ph }, { .color = { 255, 170, 0, 255 } });

        // Header Bar
        d.filledRect({ px + 1, py + 1 }, { pw - 2, 26 }, { .color = { 38, 28, 14, 255 } });
        d.line({ px, py + 27 }, { px + pw, py + 27 }, { .color = { 255, 170, 0, 255 } });
        d.text({ px + 10, py + 6 }, "TRACK 2: TEKSYNTH ENGINE", 12, { .color = { 255, 190, 40, 255 }, .font = &PoppinsLight_12 });

        // Volume Level Slider in Header
        int volW = 90;
        volumeSynthSliderRect = { px + pw - volW - 10, py + 5, volW, 16 };
        d.filledRect({ volumeSynthSliderRect.x, volumeSynthSliderRect.y }, { volumeSynthSliderRect.w, volumeSynthSliderRect.h }, { .color = { 46, 34, 20, 255 } });
        int fillW = (int)(volumeSynthSliderRect.w * CLAMP(studio.track1.volume, 0.0f, 1.0f));
        d.filledRect({ volumeSynthSliderRect.x, volumeSynthSliderRect.y }, { fillW, volumeSynthSliderRect.h }, { .color = { 255, 160, 30, 255 } });
        d.rect({ volumeSynthSliderRect.x, volumeSynthSliderRect.y }, { volumeSynthSliderRect.w, volumeSynthSliderRect.h }, { .color = { 140, 95, 50, 255 } });
        std::ostringstream volStr;
        volStr << "VOL " << (int)(studio.track1.volume * 100.0f) << "%";
        d.textCentered({ volumeSynthSliderRect.x + volW / 2, volumeSynthSliderRect.y + 2 }, volStr.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        // Sequencer Sticking to Bottom
        int seqH = 124;
        int seqY = py + ph - seqH - 4;

        int curY = py + 34;
        int contentW = pw - 20;
        int colGap = 8;
        int colW = (contentW - 2 * colGap) / 3;

        int col1X = px + 10;
        int col2X = col1X + colW + colGap;
        int col3X = col2X + colW + colGap;

        int stackedH = 115;
        int r1Y = curY;
        int r2Y = r1Y + stackedH + 4;
        int r3Y = r2Y + stackedH + 4;

        // --- ROW 1 LEFT (20% H = 115px): CUTOFF & RESONANCE 2D XY PAD ---
        synthCutoffResoXyRect = { col1X, r1Y, colW, stackedH };
        d.filledRect({ synthCutoffResoXyRect.x, synthCutoffResoXyRect.y }, { synthCutoffResoXyRect.w, synthCutoffResoXyRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ synthCutoffResoXyRect.x, synthCutoffResoXyRect.y }, { synthCutoffResoXyRect.w, synthCutoffResoXyRect.h }, { .color = { 0, 210, 255, 255 } });
        d.text({ synthCutoffResoXyRect.x + 6, synthCutoffResoXyRect.y + 4 }, "CUTOFF & RESO", 8, { .color = { 0, 230, 255, 255 }, .font = &PoppinsLight_8 });

        float cutNorm = (studio.track1.synth.cutoff.value - 0.02f) / 0.96f;
        float resoNorm = studio.track1.synth.resonance.value / 0.95f;
        int sCutTargetX = synthCutoffResoXyRect.x + 6 + (int)(cutNorm * (synthCutoffResoXyRect.w - 12));
        int sCutTargetY = synthCutoffResoXyRect.y + synthCutoffResoXyRect.h - 6 - (int)(resoNorm * (synthCutoffResoXyRect.h - 20));
        d.filledCircle({ sCutTargetX, sCutTargetY }, 4, { .color = { 0, 255, 220, 255 } });

        std::ostringstream cutTxt;
        cutTxt << (int)(studio.track1.synth.cutoff.value * 100.0f) << "%/" << (int)(studio.track1.synth.resonance.value * 100.0f) << "%";
        d.textRight({ synthCutoffResoXyRect.x + synthCutoffResoXyRect.w - 6, synthCutoffResoXyRect.y + 4 }, cutTxt.str(), 8, { .color = { 120, 230, 255, 255 }, .font = &PoppinsLight_8 });

        // --- ROW 1 CENTER (20% H = 115px): WAVEFORM & CHAOS MIX 2D PAD ---
        synthMorphChaosXyRect = { col2X, r1Y, colW, stackedH };
        d.filledRect({ synthMorphChaosXyRect.x, synthMorphChaosXyRect.y }, { synthMorphChaosXyRect.w, synthMorphChaosXyRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ synthMorphChaosXyRect.x, synthMorphChaosXyRect.y }, { synthMorphChaosXyRect.w, synthMorphChaosXyRect.h }, { .color = { 255, 140, 0, 255 } });
        d.text({ synthMorphChaosXyRect.x + 6, synthMorphChaosXyRect.y + 4 }, "WAVE & CHAOS MIX", 8, { .color = { 255, 170, 40, 255 }, .font = &PoppinsLight_8 });

        float waveNorm = studio.track1.synth.waveform.value;
        float chaosNorm = studio.track1.synth.chaosMix.value * 0.01f;
        int sWavTargetX = synthMorphChaosXyRect.x + 6 + (int)(waveNorm * (synthMorphChaosXyRect.w - 12));
        int sWavTargetY = synthMorphChaosXyRect.y + synthMorphChaosXyRect.h - 6 - (int)(chaosNorm * (synthMorphChaosXyRect.h - 20));
        d.filledCircle({ sWavTargetX, sWavTargetY }, 4, { .color = { 255, 210, 80, 255 } });

        std::ostringstream wavTxt;
        wavTxt << "MIX " << (int)studio.track1.synth.chaosMix.value << "%";
        d.textRight({ synthMorphChaosXyRect.x + synthMorphChaosXyRect.w - 6, synthMorphChaosXyRect.y + 4 }, wavTxt.str(), 8, { .color = { 255, 200, 100, 255 }, .font = &PoppinsLight_8 });

        // --- ROW 1 RIGHT (20% H = 115px): RELEASE & ENV AMOUNT PAD ---
        synthEnvXyRect = { col3X, r1Y, colW, stackedH };
        d.filledRect({ synthEnvXyRect.x, synthEnvXyRect.y }, { synthEnvXyRect.w, synthEnvXyRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ synthEnvXyRect.x, synthEnvXyRect.y }, { synthEnvXyRect.w, synthEnvXyRect.h }, { .color = { 200, 80, 255, 255 } });
        d.text({ synthEnvXyRect.x + 6, synthEnvXyRect.y + 4 }, "RELEASE & ENV AMT", 8, { .color = { 220, 120, 255, 255 }, .font = &PoppinsLight_8 });

        float relNorm = (studio.track1.synth.release.value - 10.0f) / 1990.0f;
        float envNorm = studio.track1.synth.envAmt.value;
        int sEnvTargetX = synthEnvXyRect.x + 6 + (int)(relNorm * (synthEnvXyRect.w - 12));
        int sEnvTargetY = synthEnvXyRect.y + synthEnvXyRect.h - 6 - (int)(envNorm * (synthEnvXyRect.h - 20));
        d.filledCircle({ sEnvTargetX, sEnvTargetY }, 4, { .color = { 240, 160, 255, 255 } });

        std::ostringstream envTxt;
        envTxt << (int)studio.track1.synth.release.value << "ms";
        d.textRight({ synthEnvXyRect.x + synthEnvXyRect.w - 6, synthEnvXyRect.y + 4 }, envTxt.str(), 8, { .color = { 230, 150, 255, 255 }, .font = &PoppinsLight_8 });

        // --- ROW 2 LEFT (20% H = 115px): MODULATION MATRIX BOX ---
        synthModTypeRect = { col1X, r2Y, colW, stackedH };
        d.filledRect({ synthModTypeRect.x, synthModTypeRect.y }, { synthModTypeRect.w, synthModTypeRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ synthModTypeRect.x, synthModTypeRect.y }, { synthModTypeRect.w, synthModTypeRect.h }, { .color = { 255, 80, 150, 255 } });
        d.text({ synthModTypeRect.x + 6, synthModTypeRect.y + 4 }, "MOD MATRIX", 8, { .color = { 255, 120, 180, 255 }, .font = &PoppinsLight_8 });

        d.textCentered({ synthModTypeRect.x + synthModTypeRect.w / 2, synthModTypeRect.y + 24 }, studio.track1.synth.modTypeName, 8, { .color = { 255, 230, 100, 255 }, .font = &PoppinsLight_8 });

        int mBarX = synthModTypeRect.x + 8;
        int mBarY = synthModTypeRect.y + synthModTypeRect.h - 22;
        int mBarW = synthModTypeRect.w - 16;
        int mBarH = 14;
        d.filledRect({ mBarX, mBarY }, { mBarW, mBarH }, { .color = { 28, 20, 32, 255 } });
        float mDepthNorm = (studio.track1.synth.modDepth.value + 100.0f) / 200.0f;
        int mFillW = (int)(mBarW * mDepthNorm);
        d.filledRect({ mBarX, mBarY }, { mFillW, mBarH }, { .color = { 255, 90, 160, 255 } });
        d.rect({ mBarX, mBarY }, { mBarW, mBarH }, { .color = { 255, 140, 190, 255 } });
        std::ostringstream mDepTxt;
        mDepTxt << "DEPTH " << (int)studio.track1.synth.modDepth.value << "%";
        d.textCentered({ mBarX + mBarW / 2, mBarY + 3 }, mDepTxt.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        // --- ROW 2 CENTER (20% H = 115px): CHAOS LFO & CHAOS FM XY PAD ---
        synthLfoChaosXyRect = { col2X, r2Y, colW, stackedH };
        d.filledRect({ synthLfoChaosXyRect.x, synthLfoChaosXyRect.y }, { synthLfoChaosXyRect.w, synthLfoChaosXyRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ synthLfoChaosXyRect.x, synthLfoChaosXyRect.y }, { synthLfoChaosXyRect.w, synthLfoChaosXyRect.h }, { .color = { 0, 220, 140, 255 } });
        d.text({ synthLfoChaosXyRect.x + 6, synthLfoChaosXyRect.y + 4 }, "CHAOS LFO / FM", 8, { .color = { 0, 240, 160, 255 }, .font = &PoppinsLight_8 });

        float lfoNorm = studio.track1.synth.lfoSpeed.value * 0.01f;
        float chaosFmNorm = studio.track1.synth.fmDepth.value * 0.01f;
        int sLfoTargetX = synthLfoChaosXyRect.x + 6 + (int)(lfoNorm * (synthLfoChaosXyRect.w - 12));
        int sLfoTargetY = synthLfoChaosXyRect.y + synthLfoChaosXyRect.h - 6 - (int)(chaosFmNorm * (synthLfoChaosXyRect.h - 20));
        d.filledCircle({ sLfoTargetX, sLfoTargetY }, 4, { .color = { 100, 255, 200, 255 } });

        // --- ROW 2 RIGHT (20% H = 115px): DRIVE & REVERB XY PAD ---
        synthFxXyRect = { col3X, r2Y, colW, stackedH };
        d.filledRect({ synthFxXyRect.x, synthFxXyRect.y }, { synthFxXyRect.w, synthFxXyRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ synthFxXyRect.x, synthFxXyRect.y }, { synthFxXyRect.w, synthFxXyRect.h }, { .color = { 255, 200, 0, 255 } });
        d.text({ synthFxXyRect.x + 6, synthFxXyRect.y + 4 }, "DRIVE & REVERB", 8, { .color = { 255, 220, 50, 255 }, .font = &PoppinsLight_8 });

        float drvFxNorm = studio.track1.synth.drive.value * 0.01f;
        float rvbNorm = studio.track1.synth.reverbMix.value * 0.01f;
        int sFxTargetX = synthFxXyRect.x + 6 + (int)(drvFxNorm * (synthFxXyRect.w - 12));
        int sFxTargetY = synthFxXyRect.y + synthFxXyRect.h - 6 - (int)(rvbNorm * (synthFxXyRect.h - 20));
        d.filledCircle({ sFxTargetX, sFxTargetY }, 4, { .color = { 255, 240, 120, 255 } });

        // --- ROW 3 LEFT (20% H = 115px): BASE SYNTH PITCH OFFSET ---
        semitoneBarSynthRect = { col1X, r3Y, colW, stackedH };
        d.filledRect({ semitoneBarSynthRect.x, semitoneBarSynthRect.y }, { semitoneBarSynthRect.w, semitoneBarSynthRect.h }, { .color = { 24, 18, 12, 255 } });
        d.rect({ semitoneBarSynthRect.x, semitoneBarSynthRect.y }, { semitoneBarSynthRect.w, semitoneBarSynthRect.h }, { .color = { 255, 170, 0, 255 } });
        d.text({ semitoneBarSynthRect.x + 6, semitoneBarSynthRect.y + 4 }, "BASE PITCH", 8, { .color = { 255, 190, 40, 255 }, .font = &PoppinsLight_8 });

        float pNorm = (studio.track1.synth.pitch.value - 24.0f) / 72.0f;
        int pFillW = (int)((semitoneBarSynthRect.w - 12) * pNorm);
        d.filledRect({ semitoneBarSynthRect.x + 6, semitoneBarSynthRect.y + 24 }, { semitoneBarSynthRect.w - 12, 22 }, { .color = { 40, 28, 16, 255 } });
        if (pFillW > 0) {
            d.filledRect({ semitoneBarSynthRect.x + 6, semitoneBarSynthRect.y + 24 }, { pFillW, 22 }, { .color = { 255, 160, 20, 255 } });
        }
        d.rect({ semitoneBarSynthRect.x + 6, semitoneBarSynthRect.y + 24 }, { semitoneBarSynthRect.w - 12, 22 }, { .color = { 255, 190, 40, 255 } });

        std::ostringstream sPitchTxt;
        sPitchTxt << (int)studio.track1.synth.pitch.value << " (C" << ((int)studio.track1.synth.pitch.value / 12 - 1) << ")";
        d.textCentered({ semitoneBarSynthRect.x + semitoneBarSynthRect.w / 2, semitoneBarSynthRect.y + 30 }, sPitchTxt.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        // --- ROW 3 CENTER (20% H = 115px): DELAY TIME & MIX XY PAD ---
        synthDelayXyRect = { col2X, r3Y, colW, stackedH };
        d.filledRect({ synthDelayXyRect.x, synthDelayXyRect.y }, { synthDelayXyRect.w, synthDelayXyRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ synthDelayXyRect.x, synthDelayXyRect.y }, { synthDelayXyRect.w, synthDelayXyRect.h }, { .color = { 0, 180, 255, 255 } });
        d.text({ synthDelayXyRect.x + 6, synthDelayXyRect.y + 4 }, "DELAY TIME & MIX", 8, { .color = { 0, 200, 255, 255 }, .font = &PoppinsLight_8 });

        float dlyTimeNorm = (studio.track1.synth.dlyTime.value - 10.0f) / 990.0f;
        float dlyMixNorm = studio.track1.synth.dlyMix.value * 0.01f;
        int dlyTargetX = synthDelayXyRect.x + 6 + (int)(dlyTimeNorm * (synthDelayXyRect.w - 12));
        int dlyTargetY = synthDelayXyRect.y + synthDelayXyRect.h - 6 - (int)(dlyMixNorm * (synthDelayXyRect.h - 20));
        d.filledCircle({ dlyTargetX, dlyTargetY }, 4, { .color = { 100, 220, 255, 255 } });

        std::ostringstream dlyTxt;
        dlyTxt << (int)studio.track1.synth.dlyTime.value << "ms/" << (int)studio.track1.synth.dlyMix.value << "%";
        d.textRight({ synthDelayXyRect.x + synthDelayXyRect.w - 6, synthDelayXyRect.y + 4 }, dlyTxt.str(), 8, { .color = { 120, 210, 255, 255 }, .font = &PoppinsLight_8 });

        // --- ROW 3 RIGHT (20% H = 115px): COLOR SVF & BITCRUSHER XY PAD ---
        synthColorCrushXyRect = { col3X, r3Y, colW, stackedH };
        d.filledRect({ synthColorCrushXyRect.x, synthColorCrushXyRect.y }, { synthColorCrushXyRect.w, synthColorCrushXyRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ synthColorCrushXyRect.x, synthColorCrushXyRect.y }, { synthColorCrushXyRect.w, synthColorCrushXyRect.h }, { .color = { 255, 90, 160, 255 } });
        d.text({ synthColorCrushXyRect.x + 6, synthColorCrushXyRect.y + 4 }, "COLOR & BITCRUSH", 8, { .color = { 255, 120, 180, 255 }, .font = &PoppinsLight_8 });

        float colNorm = studio.track1.synth.color.value * 0.01f;
        float crushNorm = (studio.track1.synth.crushFm.value + 100.0f) / 200.0f;
        int crushTargetX = synthColorCrushXyRect.x + 6 + (int)(colNorm * (synthColorCrushXyRect.w - 12));
        int crushTargetY = synthColorCrushXyRect.y + synthColorCrushXyRect.h - 6 - (int)(crushNorm * (synthColorCrushXyRect.h - 20));
        d.filledCircle({ crushTargetX, crushTargetY }, 4, { .color = { 255, 170, 210, 255 } });

        // --- 64-STEP SEQUENCER STICKING TO THE BOTTOM ---
        drawSequencer(d, px, seqY, pw, 1);
    }

    // --- REUSABLE SEQUENCER COMPONENT STICKING TO THE BOTTOM ---
    void drawSequencer(Draw& d, int px, int seqY, int pw, int trackIdx)
    {
        int seqW = pw - 16;
        int leftCtrlW = 36;
        int gridX = px + 8 + leftCtrlW;
        int gridW = seqW - leftCtrlW;
        int stepBoxW = (gridW - (16 - 1) * 2) / 16;
        int stepBoxH = 20;

        auto& sequence = (trackIdx == 0) ? studio.track0.sequence : studio.track1.sequence;
        auto& rowEnabled = (trackIdx == 0) ? studio.track0.rowEnabled : studio.track1.rowEnabled;
        BoxRect* colRects = (trackIdx == 0) ? colEnableRects0 : colEnableRects1;
        BoxRect* rowChkRects = (trackIdx == 0) ? rowCheckRects0 : rowCheckRects1;
        BoxRect* rowGRects = (trackIdx == 0) ? rowGenRects0 : rowGenRects1;
        BoxRect* stpRects = (trackIdx == 0) ? stepRects0 : stepRects1;

        // Top Column Enable Buttons
        int colBtnY = seqY;
        for (int c = 0; c < 16; c++) {
            int cx = gridX + c * (stepBoxW + 2);
            colRects[c] = { cx, colBtnY, stepBoxW, 10 };

            bool allColActive = true;
            for (int r = 0; r < 4; r++) {
                if (!sequence[r * 16 + c].active) {
                    allColActive = false;
                    break;
                }
            }

            Color colBg = allColActive ? Color { 220, 130, 0, 255 } : Color { 38, 30, 20, 255 };
            Color colBdr = allColActive ? Color { 255, 200, 40, 255 } : Color { 120, 90, 40, 255 };
            d.filledRect({ cx, colBtnY }, { stepBoxW, 10 }, { .color = colBg });
            d.rect({ cx, colBtnY }, { stepBoxW, 10 }, { .color = colBdr });

            std::ostringstream cStr;
            cStr << (c + 1);
            d.textCentered({ cx + stepBoxW / 2, colBtnY + 1 }, cStr.str(), 8, { .color = allColActive ? Color { 255, 255, 255, 255 } : Color { 180, 150, 80, 255 }, .font = &PoppinsLight_8 });
        }

        // 4 Rows of 16 Steps
        int rowStartY = colBtnY + 12;
        int playHead = studio.currentStep.load();

        for (int r = 0; r < 4; r++) {
            int ry = rowStartY + r * (stepBoxH + 2);
            bool isRowOn = rowEnabled[r];

            // 1. Left Checkbox [X]
            rowChkRects[r] = { px + 8, ry + (stepBoxH - 12) / 2, 13, 12 };
            Color cbBg = isRowOn ? (trackIdx == 0 ? Color { 0, 160, 120, 255 } : Color { 220, 120, 0, 255 }) : Color { 24, 28, 36, 255 };
            Color cbBdr = isRowOn ? Color { 255, 255, 255, 255 } : Color { 60, 70, 90, 255 };
            d.filledRect({ rowChkRects[r].x, rowChkRects[r].y }, { 13, 12 }, { .color = cbBg });
            d.rect({ rowChkRects[r].x, rowChkRects[r].y }, { 13, 12 }, { .color = cbBdr });
            d.textCentered({ rowChkRects[r].x + 6, rowChkRects[r].y + 2 }, isRowOn ? "X" : "-", 8, { .color = isRowOn ? Color { 255, 255, 255, 255 } : Color { 100, 110, 130, 255 }, .font = &PoppinsLight_8 });

            // 2. Left "G" Generate Button
            rowGRects[r] = { px + 8 + 15, ry + (stepBoxH - 12) / 2, 14, 12 };
            d.filledRect({ rowGRects[r].x, rowGRects[r].y }, { 14, 12 }, { .color = Color { 36, 46, 64, 255 } });
            d.rect({ rowGRects[r].x, rowGRects[r].y }, { 14, 12 }, { .color = Color { 255, 180, 40, 255 } });
            d.textCentered({ rowGRects[r].x + 7, rowGRects[r].y + 2 }, "G", 8, { .color = Color { 255, 210, 60, 255 }, .font = &PoppinsLight_8 });

            // 3. 16 Step Boxes for Row r
            for (int c = 0; c < 16; c++) {
                int i = r * 16 + c;
                int sx = gridX + c * (stepBoxW + 2);
                stpRects[i] = { sx, ry, stepBoxW, stepBoxH };

                auto& stp = sequence[i];
                bool isCurrent = (playHead == i && studio.isPlaying);

                Color stepBg;
                if (!isRowOn) {
                    stepBg = stp.active ? Color { 50, 60, 70, 180 } : Color { 14, 18, 26, 255 };
                } else if (stp.active) {
                    if (trackIdx == 0) {
                        stepBg = (c % 4 == 0) ? Color { 0, 190, 230, 255 } : Color { 0, 150, 190, 255 };
                    } else {
                        stepBg = (c % 4 == 0) ? Color { 255, 160, 0, 255 } : Color { 220, 120, 0, 255 };
                    }
                } else {
                    stepBg = (c % 4 == 0) ? Color { 26, 34, 50, 255 } : Color { 18, 24, 36, 255 };
                }

                d.filledRect({ sx, ry }, { stepBoxW, stepBoxH }, { .color = stepBg });

                Color borderCol = isCurrent ? Color { 255, 240, 0, 255 } : (isRowOn ? Color { 45, 60, 85, 255 } : Color { 28, 36, 48, 255 });
                d.rect({ sx, ry }, { stepBoxW, stepBoxH }, { .color = borderCol });
                if (isCurrent) {
                    d.rect({ sx + 1, ry + 1 }, { stepBoxW - 2, stepBoxH - 2 }, { .color = { 255, 255, 100, 255 } });
                }

                // Step text
                if (stp.active) {
                    std::string noteStr = (stp.note >= 0 && stp.note < 132) ? MIDI_NOTES_STR[stp.note] : "C4";
                    d.textCentered({ sx + stepBoxW / 2, ry + 5 }, noteStr, 8, { .color = isRowOn ? Color { 255, 255, 255, 255 } : Color { 140, 160, 180, 255 }, .font = &PoppinsLight_8 });
                } else {
                    std::ostringstream numStr;
                    numStr << (c + 1);
                    d.textCentered({ sx + stepBoxW / 2, ry + 5 }, numStr.str(), 8, { .color = isRowOn ? Color { 80, 100, 130, 255 } : Color { 45, 55, 70, 255 }, .font = &PoppinsLight_8 });
                }
            }
        }
    }

    void draw(Draw& d)
    {
        int W = d.screenSize.w;
        int H = d.screenSize.h;

        // Clear background
        d.filledRect({ 0, 0 }, { W, H }, { .color = { 10, 12, 18, 255 } });

        // Header
        drawHeader(d, W, H);

        int headH = 38;
        int gridH = H - headH;
        int trackW = W / 2;

        // Track 0 (Left Side: Massive Kick)
        drawTrack0Panel(d, 0, headH, trackW, gridH);

        // Track 1 (Right Side: TeKSynth)
        drawTrack1Panel(d, trackW, headH, trackW, gridH);
    }

    // Interactive Input Handlers
    void onMouseDown(int mx, int my)
    {
        std::lock_guard<std::mutex> lock(studio.audioMutex);

        if (playBtnRect.contains(mx, my)) {
            studio.isPlaying = !studio.isPlaying;
            return;
        }

        if (bpmRect.contains(mx, my)) {
            activeDrag = DRAG_BPM;
            dragStartY = my;
            return;
        }

        // --- TRACK 0 (KICK) MOUSE DOWN ---
        if (volumeKickSliderRect.contains(mx, my)) {
            activeDrag = DRAG_VOLUME_KICK;
            float norm = CLAMP((float)(mx - volumeKickSliderRect.x) / (float)volumeKickSliderRect.w, 0.0f, 1.0f);
            studio.track0.volume = norm;
            return;
        }

        if (mmFilterXyRect.contains(mx, my)) {
            activeDrag = DRAG_MM_FILTER_XY;
            float cutNorm = CLAMP((float)(mx - (mmFilterXyRect.x + 6)) / (float)(mmFilterXyRect.w - 12), 0.0f, 1.0f);
            float resoNorm = CLAMP((float)(mmFilterXyRect.y + mmFilterXyRect.h - 6 - my) / (float)(mmFilterXyRect.h - 20), 0.0f, 1.0f);
            studio.track0.kick.filterCutoff.value = -100.0f + cutNorm * 200.0f;
            studio.track0.kick.filterReso.value = resoNorm * 100.0f;

            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        if (sweepCurveRect.contains(mx, my)) {
            activeDrag = DRAG_SWEEP_XY;
            float shpNorm = CLAMP((float)(mx - (sweepCurveRect.x + 6)) / (float)(sweepCurveRect.w - 12), 0.0f, 1.0f);
            float depthNorm = CLAMP((float)(sweepCurveRect.y + sweepCurveRect.h - 6 - my) / (float)(sweepCurveRect.h - 20), 0.0f, 1.0f);
            studio.track0.kick.sweepShp.value = shpNorm * 100.0f;
            studio.track0.kick.sweepDepth.value = depthNorm * 100.0f;

            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        if (vcoMorphRect.contains(mx, my)) {
            BoxRect pBarRect = { vcoMorphRect.x + 8, vcoMorphRect.y + 16, vcoMorphRect.w - 16, 14 };
            if (pBarRect.contains(mx, my)) {
                activeDrag = DRAG_VCO_MORPH_BAR;
                float norm = CLAMP((float)(mx - pBarRect.x) / (float)pBarRect.w, 0.0f, 1.0f);
                studio.track0.kick.vcoMorph.value = norm * 100.0f;
            } else {
                activeDrag = DRAG_VCO_MORPH_BODY;
                dragStartX = mx;
                dragStartValX = studio.track0.kick.vcoMorph.value;
            }
            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        if (clickXyRect.contains(mx, my)) {
            activeDrag = DRAG_CLICK_XY;
            float amtNorm = CLAMP((float)(mx - (clickXyRect.x + 6)) / (float)(clickXyRect.w - 12), 0.0f, 1.0f);
            float decNorm = CLAMP((float)(clickXyRect.y + clickXyRect.h - 6 - my) / (float)(clickXyRect.h - 20), 0.0f, 1.0f);
            studio.track0.kick.kickClickAmt.value = amtNorm * 100.0f;
            studio.track0.kick.kickClickDecay.value = 1.0f + decNorm * 99.0f;

            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        if (fmXyRect.contains(mx, my)) {
            activeDrag = DRAG_FM_XY;
            int padBodyH = fmXyRect.h - 18;
            float depthNorm = CLAMP((float)(mx - (fmXyRect.x + 6)) / (float)(fmXyRect.w - 12), 0.0f, 1.0f);
            float snapNorm = CLAMP((float)(fmXyRect.y + padBodyH - 6 - my) / (float)(padBodyH - 20), 0.0f, 1.0f);
            studio.track0.kick.fmDepth.value = depthNorm * 100.0f;
            studio.track0.kick.fmSnap.value = 2.0f + snapNorm * 148.0f;

            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        if (fmRatioBarRect.contains(mx, my)) {
            activeDrag = DRAG_FM_RATIO_BAR;
            float norm = CLAMP((float)(mx - fmRatioBarRect.x) / (float)fmRatioBarRect.w, 0.0f, 1.0f);
            int seg = (int)std::round(norm * 30.0f);
            studio.track0.kick.fmRatio.value = 0.5f + seg * 0.25f;

            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        if (driveXyRect.contains(mx, my)) {
            activeDrag = DRAG_DRIVE_XY;
            int padBodyH = driveXyRect.h - 18;
            float drvNorm = CLAMP((float)(mx - (driveXyRect.x + 6)) / (float)(driveXyRect.w - 12), 0.0f, 1.0f);
            float boostNorm = CLAMP((float)(driveXyRect.y + padBodyH - 6 - my) / (float)(padBodyH - 20), 0.0f, 1.0f);
            studio.track0.kick.drive.value = drvNorm * 100.0f;
            studio.track0.kick.bassBoost.value = boostNorm * 100.0f;

            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        if (foldBarRect.contains(mx, my)) {
            activeDrag = DRAG_FOLD_BAR;
            float norm = CLAMP((float)(mx - foldBarRect.x) / (float)foldBarRect.w, 0.0f, 1.0f);
            int seg = (int)std::round(norm * 20.0f);
            studio.track0.kick.fold.value = seg * 5.0f;

            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        if (durationBarRect.contains(mx, my)) {
            activeDrag = DRAG_DURATION_BAR;
            float norm = CLAMP((float)(mx - durationBarRect.x) / (float)durationBarRect.w, 0.0f, 1.0f);
            Param& p = studio.track0.kick.duration;
            p.value = p.min + norm * (p.max - p.min);

            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        if (semitoneBarKickRect.contains(mx, my)) {
            activeDrag = DRAG_SEMITONE_BAR_KICK;
            int pBarX = semitoneBarKickRect.x + 2;
            int pBarW = semitoneBarKickRect.w - 4;
            int centerW = 4;
            int sideW = (pBarW - centerW) / 2;
            int centerX = pBarX + sideW;

            int semi = 0;
            if (mx < centerX) {
                float normLeft = CLAMP((float)(mx - pBarX) / (float)sideW, 0.0f, 1.0f);
                semi = -12 + (int)std::floor(normLeft * 12.0f);
            } else if (mx >= centerX + centerW) {
                float normRight = CLAMP((float)(mx - (centerX + centerW)) / (float)sideW, 0.0f, 1.0f);
                semi = 1 + (int)std::floor(normRight * 12.0f);
            } else {
                semi = 0;
            }
            semi = std::clamp(semi, -12, 12);
            studio.track0.kick.semitoneOffset.store(semi);

            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        if (kickSubFreqBarRect.contains(mx, my)) {
            activeDrag = DRAG_KICK_SUB_FREQ;
            float norm = CLAMP((float)(mx - (kickSubFreqBarRect.x + 6)) / (float)(kickSubFreqBarRect.w - 12), 0.0f, 1.0f);
            Param& p = studio.track0.kick.baseFreq;
            p.value = p.min + norm * (p.max - p.min);

            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        if (kickHardClickRect.contains(mx, my)) {
            bool isHard = studio.track0.kick.isHardClickActive.load();
            studio.track0.kick.isHardClickActive.store(!isHard);
            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        if (kickSubDropRect.contains(mx, my)) {
            bool isDrop = studio.track0.kick.isSubDropActive.load();
            studio.track0.kick.isSubDropActive.store(!isDrop);
            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        // --- TRACK 1 (TEKSYNTH) MOUSE DOWN ---
        if (volumeSynthSliderRect.contains(mx, my)) {
            activeDrag = DRAG_SYNTH_VOLUME;
            float norm = CLAMP((float)(mx - volumeSynthSliderRect.x) / (float)volumeSynthSliderRect.w, 0.0f, 1.0f);
            studio.track1.volume = norm;
            return;
        }

        if (synthCutoffResoXyRect.contains(mx, my)) {
            activeDrag = DRAG_SYNTH_CUTOFF_RESO_XY;
            float cutNorm = CLAMP((float)(mx - (synthCutoffResoXyRect.x + 6)) / (float)(synthCutoffResoXyRect.w - 12), 0.0f, 1.0f);
            float resoNorm = CLAMP((float)(synthCutoffResoXyRect.y + synthCutoffResoXyRect.h - 6 - my) / (float)(synthCutoffResoXyRect.h - 20), 0.0f, 1.0f);
            studio.track1.synth.cutoff.value = 0.02f + cutNorm * 0.96f;
            studio.track1.synth.resonance.value = resoNorm * 0.95f;

            studio.track1.synth.noteOn(studio.track1.synth.pitch.value, 0.9f);
            studio.synthPulseTrigger.store(true);
            return;
        }

        if (synthMorphChaosXyRect.contains(mx, my)) {
            activeDrag = DRAG_SYNTH_MORPH_CHAOS_XY;
            float waveNorm = CLAMP((float)(mx - (synthMorphChaosXyRect.x + 6)) / (float)(synthMorphChaosXyRect.w - 12), 0.0f, 1.0f);
            float chaosNorm = CLAMP((float)(synthMorphChaosXyRect.y + synthMorphChaosXyRect.h - 6 - my) / (float)(synthMorphChaosXyRect.h - 20), 0.0f, 1.0f);
            studio.track1.synth.waveform.value = waveNorm;
            studio.track1.synth.chaosMix.value = chaosNorm * 100.0f;

            studio.track1.synth.noteOn(studio.track1.synth.pitch.value, 0.9f);
            studio.synthPulseTrigger.store(true);
            return;
        }

        if (synthEnvXyRect.contains(mx, my)) {
            activeDrag = DRAG_SYNTH_ENV_XY;
            float relNorm = CLAMP((float)(mx - (synthEnvXyRect.x + 6)) / (float)(synthEnvXyRect.w - 12), 0.0f, 1.0f);
            float envNorm = CLAMP((float)(synthEnvXyRect.y + synthEnvXyRect.h - 6 - my) / (float)(synthEnvXyRect.h - 20), 0.0f, 1.0f);
            studio.track1.synth.release.value = 10.0f + relNorm * 1990.0f;
            studio.track1.synth.envAmt.value = envNorm;

            studio.track1.synth.noteOn(studio.track1.synth.pitch.value, 0.9f);
            studio.synthPulseTrigger.store(true);
            return;
        }

        if (synthModTypeRect.contains(mx, my)) {
            float val = studio.track1.synth.modType.value + 1.0f;
            if (val > 15.0f) val = 0.0f;
            studio.track1.synth.modType.set(val);

            studio.track1.synth.noteOn(studio.track1.synth.pitch.value, 0.9f);
            studio.synthPulseTrigger.store(true);
            return;
        }

        if (synthLfoChaosXyRect.contains(mx, my)) {
            activeDrag = DRAG_SYNTH_LFO_CHAOS_XY;
            float lfoNorm = CLAMP((float)(mx - (synthLfoChaosXyRect.x + 6)) / (float)(synthLfoChaosXyRect.w - 12), 0.0f, 1.0f);
            float chaosFmNorm = CLAMP((float)(synthLfoChaosXyRect.y + synthLfoChaosXyRect.h - 6 - my) / (float)(synthLfoChaosXyRect.h - 20), 0.0f, 1.0f);
            studio.track1.synth.lfoSpeed.value = lfoNorm * 100.0f;
            studio.track1.synth.fmDepth.value = chaosFmNorm * 100.0f;

            studio.track1.synth.noteOn(studio.track1.synth.pitch.value, 0.9f);
            studio.synthPulseTrigger.store(true);
            return;
        }

        if (synthFxXyRect.contains(mx, my)) {
            activeDrag = DRAG_SYNTH_FX_XY;
            float drvNorm = CLAMP((float)(mx - (synthFxXyRect.x + 6)) / (float)(synthFxXyRect.w - 12), 0.0f, 1.0f);
            float rvbNorm = CLAMP((float)(synthFxXyRect.y + synthFxXyRect.h - 6 - my) / (float)(synthFxXyRect.h - 20), 0.0f, 1.0f);
            studio.track1.synth.drive.value = drvNorm * 100.0f;
            studio.track1.synth.reverbMix.value = rvbNorm * 100.0f;

            studio.track1.synth.noteOn(studio.track1.synth.pitch.value, 0.9f);
            studio.synthPulseTrigger.store(true);
            return;
        }

        if (semitoneBarSynthRect.contains(mx, my)) {
            activeDrag = DRAG_SYNTH_SEMITONE_BAR;
            float norm = CLAMP((float)(mx - (semitoneBarSynthRect.x + 6)) / (float)(semitoneBarSynthRect.w - 12), 0.0f, 1.0f);
            int newPitch = 24 + (int)std::round(norm * 72.0f);
            studio.track1.synth.pitch.value = (float)newPitch;

            studio.track1.synth.noteOn((uint8_t)newPitch, 0.9f);
            studio.synthPulseTrigger.store(true);
            return;
        }

        if (synthDelayXyRect.contains(mx, my)) {
            activeDrag = DRAG_SYNTH_DELAY_XY;
            float dlyTimeNorm = CLAMP((float)(mx - (synthDelayXyRect.x + 6)) / (float)(synthDelayXyRect.w - 12), 0.0f, 1.0f);
            float dlyMixNorm = CLAMP((float)(synthDelayXyRect.y + synthDelayXyRect.h - 6 - my) / (float)(synthDelayXyRect.h - 20), 0.0f, 1.0f);
            studio.track1.synth.dlyTime.value = 10.0f + dlyTimeNorm * 990.0f;
            studio.track1.synth.dlyMix.value = dlyMixNorm * 100.0f;

            studio.track1.synth.noteOn(studio.track1.synth.pitch.value, 0.9f);
            studio.synthPulseTrigger.store(true);
            return;
        }

        if (synthColorCrushXyRect.contains(mx, my)) {
            activeDrag = DRAG_SYNTH_CRUSH_COLOR_XY;
            float colNorm = CLAMP((float)(mx - (synthColorCrushXyRect.x + 6)) / (float)(synthColorCrushXyRect.w - 12), 0.0f, 1.0f);
            float crushNorm = CLAMP((float)(synthColorCrushXyRect.y + synthColorCrushXyRect.h - 6 - my) / (float)(synthColorCrushXyRect.h - 20), 0.0f, 1.0f);
            studio.track1.synth.color.value = colNorm * 100.0f;
            studio.track1.synth.crushFm.value = -100.0f + crushNorm * 200.0f;

            studio.track1.synth.noteOn(studio.track1.synth.pitch.value, 0.9f);
            studio.synthPulseTrigger.store(true);
            return;
        }

        // --- SEQUENCER CLICK HANDLERS (TRACK 0 & TRACK 1) ---
        for (int r = 0; r < 4; r++) {
            if (rowCheckRects0[r].contains(mx, my)) {
                studio.track0.rowEnabled[r] = !studio.track0.rowEnabled[r];
                return;
            }
            if (rowGenRects0[r].contains(mx, my)) {
                generateRowPattern(0, r);
                return;
            }
            if (rowCheckRects1[r].contains(mx, my)) {
                studio.track1.rowEnabled[r] = !studio.track1.rowEnabled[r];
                return;
            }
            if (rowGenRects1[r].contains(mx, my)) {
                generateRowPattern(1, r);
                return;
            }
        }

        for (int c = 0; c < 16; c++) {
            if (colEnableRects0[c].contains(mx, my)) {
                bool allActive = true;
                for (int r = 0; r < 4; r++) {
                    if (!studio.track0.sequence[r * 16 + c].active) allActive = false;
                }
                bool newState = !allActive;
                for (int r = 0; r < 4; r++) studio.track0.sequence[r * 16 + c].active = newState;
                return;
            }
            if (colEnableRects1[c].contains(mx, my)) {
                bool allActive = true;
                for (int r = 0; r < 4; r++) {
                    if (!studio.track1.sequence[r * 16 + c].active) allActive = false;
                }
                bool newState = !allActive;
                for (int r = 0; r < 4; r++) studio.track1.sequence[r * 16 + c].active = newState;
                return;
            }
        }

        for (int i = 0; i < SEQ_STEPS_TEK; i++) {
            if (stepRects0[i].contains(mx, my)) {
                studio.track0.sequence[i].active = !studio.track0.sequence[i].active;
                activeDrag = DRAG_STEP_NOTE_KICK;
                dragStepIdx = i;
                dragStartY = my;
                dragStartNote = studio.track0.sequence[i].note;

                if (studio.track0.sequence[i].active && !studio.isPlaying) {
                    studio.track0.kick.noteOn(studio.track0.sequence[i].note, studio.track0.sequence[i].velocity);
                    studio.kickPulseTrigger.store(true);
                }
                return;
            }

            if (stepRects1[i].contains(mx, my)) {
                studio.track1.sequence[i].active = !studio.track1.sequence[i].active;
                activeDrag = DRAG_STEP_NOTE_SYNTH;
                dragStepIdx = i;
                dragStartY = my;
                dragStartNote = studio.track1.sequence[i].note;

                if (studio.track1.sequence[i].active && !studio.isPlaying) {
                    studio.track1.synth.noteOn(studio.track1.sequence[i].note, studio.track1.sequence[i].velocity);
                    studio.synthPulseTrigger.store(true);
                }
                return;
            }
        }
    }

    void onMouseDrag(int mx, int my)
    {
        std::lock_guard<std::mutex> lock(studio.audioMutex);

        if (activeDrag == DRAG_BPM) {
            int dy = dragStartY - my;
            float newBpm = CLAMP(studio.bpm.load() + dy * 0.5f, 40.0f, 280.0f);
            studio.bpm.store(newBpm);
            studio.updateClock();
            dragStartY = my;
        } else if (activeDrag == DRAG_VOLUME_KICK) {
            float norm = CLAMP((float)(mx - volumeKickSliderRect.x) / (float)volumeKickSliderRect.w, 0.0f, 1.0f);
            studio.track0.volume = norm;
        } else if (activeDrag == DRAG_SYNTH_VOLUME) {
            float norm = CLAMP((float)(mx - volumeSynthSliderRect.x) / (float)volumeSynthSliderRect.w, 0.0f, 1.0f);
            studio.track1.volume = norm;
        } else if (activeDrag == DRAG_MM_FILTER_XY) {
            float cutNorm = CLAMP((float)(mx - (mmFilterXyRect.x + 6)) / (float)(mmFilterXyRect.w - 12), 0.0f, 1.0f);
            float resoNorm = CLAMP((float)(mmFilterXyRect.y + mmFilterXyRect.h - 6 - my) / (float)(mmFilterXyRect.h - 20), 0.0f, 1.0f);
            studio.track0.kick.filterCutoff.value = -100.0f + cutNorm * 200.0f;
            studio.track0.kick.filterReso.value = resoNorm * 100.0f;
        } else if (activeDrag == DRAG_SWEEP_XY) {
            float shpNorm = CLAMP((float)(mx - (sweepCurveRect.x + 6)) / (float)(sweepCurveRect.w - 12), 0.0f, 1.0f);
            float depthNorm = CLAMP((float)(sweepCurveRect.y + sweepCurveRect.h - 6 - my) / (float)(sweepCurveRect.h - 20), 0.0f, 1.0f);
            studio.track0.kick.sweepShp.value = shpNorm * 100.0f;
            studio.track0.kick.sweepDepth.value = depthNorm * 100.0f;
        } else if (activeDrag == DRAG_VCO_MORPH_BAR) {
            float norm = CLAMP((float)(mx - (vcoMorphRect.x + 8)) / (float)(vcoMorphRect.w - 16), 0.0f, 1.0f);
            studio.track0.kick.vcoMorph.value = norm * 100.0f;
        } else if (activeDrag == DRAG_VCO_MORPH_BODY) {
            int dx = mx - dragStartX;
            float newMorph = CLAMP(dragStartValX + (float)dx * 0.6f, 0.0f, 100.0f);
            studio.track0.kick.vcoMorph.value = newMorph;
        } else if (activeDrag == DRAG_CLICK_XY) {
            float amtNorm = CLAMP((float)(mx - (clickXyRect.x + 6)) / (float)(clickXyRect.w - 12), 0.0f, 1.0f);
            float decNorm = CLAMP((float)(clickXyRect.y + clickXyRect.h - 6 - my) / (float)(clickXyRect.h - 20), 0.0f, 1.0f);
            studio.track0.kick.kickClickAmt.value = amtNorm * 100.0f;
            studio.track0.kick.kickClickDecay.value = 1.0f + decNorm * 99.0f;
        } else if (activeDrag == DRAG_FM_XY) {
            int padBodyH = fmXyRect.h - 18;
            float depthNorm = CLAMP((float)(mx - (fmXyRect.x + 6)) / (float)(fmXyRect.w - 12), 0.0f, 1.0f);
            float snapNorm = CLAMP((float)(fmXyRect.y + padBodyH - 6 - my) / (float)(padBodyH - 20), 0.0f, 1.0f);
            studio.track0.kick.fmDepth.value = depthNorm * 100.0f;
            studio.track0.kick.fmSnap.value = 2.0f + snapNorm * 148.0f;
        } else if (activeDrag == DRAG_FM_RATIO_BAR) {
            float norm = CLAMP((float)(mx - fmRatioBarRect.x) / (float)fmRatioBarRect.w, 0.0f, 1.0f);
            int seg = (int)std::round(norm * 30.0f);
            studio.track0.kick.fmRatio.value = 0.5f + seg * 0.25f;
        } else if (activeDrag == DRAG_DRIVE_XY) {
            int padBodyH = driveXyRect.h - 18;
            float drvNorm = CLAMP((float)(mx - (driveXyRect.x + 6)) / (float)(driveXyRect.w - 12), 0.0f, 1.0f);
            float boostNorm = CLAMP((float)(driveXyRect.y + padBodyH - 6 - my) / (float)(padBodyH - 20), 0.0f, 1.0f);
            studio.track0.kick.drive.value = drvNorm * 100.0f;
            studio.track0.kick.bassBoost.value = boostNorm * 100.0f;
        } else if (activeDrag == DRAG_FOLD_BAR) {
            float norm = CLAMP((float)(mx - foldBarRect.x) / (float)foldBarRect.w, 0.0f, 1.0f);
            int seg = (int)std::round(norm * 20.0f);
            studio.track0.kick.fold.value = seg * 5.0f;
        } else if (activeDrag == DRAG_DURATION_BAR) {
            float norm = CLAMP((float)(mx - durationBarRect.x) / (float)durationBarRect.w, 0.0f, 1.0f);
            Param& p = studio.track0.kick.duration;
            p.value = p.min + norm * (p.max - p.min);
        } else if (activeDrag == DRAG_SEMITONE_BAR_KICK) {
            int pBarX = semitoneBarKickRect.x + 2;
            int pBarW = semitoneBarKickRect.w - 4;
            int centerW = 4;
            int sideW = (pBarW - centerW) / 2;
            int centerX = pBarX + sideW;

            int semi = 0;
            if (mx < centerX) {
                float normLeft = CLAMP((float)(mx - pBarX) / (float)sideW, 0.0f, 1.0f);
                semi = -12 + (int)std::floor(normLeft * 12.0f);
            } else if (mx >= centerX + centerW) {
                float normRight = CLAMP((float)(mx - (centerX + centerW)) / (float)sideW, 0.0f, 1.0f);
                semi = 1 + (int)std::floor(normRight * 12.0f);
            } else {
                semi = 0;
            }
            semi = std::clamp(semi, -12, 12);
            studio.track0.kick.semitoneOffset.store(semi);
        } else if (activeDrag == DRAG_KICK_SUB_FREQ) {
            float norm = CLAMP((float)(mx - (kickSubFreqBarRect.x + 6)) / (float)(kickSubFreqBarRect.w - 12), 0.0f, 1.0f);
            Param& p = studio.track0.kick.baseFreq;
            p.value = p.min + norm * (p.max - p.min);
        } else if (activeDrag == DRAG_SYNTH_CUTOFF_RESO_XY) {
            float cutNorm = CLAMP((float)(mx - (synthCutoffResoXyRect.x + 6)) / (float)(synthCutoffResoXyRect.w - 12), 0.0f, 1.0f);
            float resoNorm = CLAMP((float)(synthCutoffResoXyRect.y + synthCutoffResoXyRect.h - 6 - my) / (float)(synthCutoffResoXyRect.h - 20), 0.0f, 1.0f);
            studio.track1.synth.cutoff.value = 0.02f + cutNorm * 0.96f;
            studio.track1.synth.resonance.value = resoNorm * 0.95f;
        } else if (activeDrag == DRAG_SYNTH_MORPH_CHAOS_XY) {
            float waveNorm = CLAMP((float)(mx - (synthMorphChaosXyRect.x + 6)) / (float)(synthMorphChaosXyRect.w - 12), 0.0f, 1.0f);
            float chaosNorm = CLAMP((float)(mx - (synthMorphChaosXyRect.x + 6)) / (float)(synthMorphChaosXyRect.w - 12), 0.0f, 1.0f);
            studio.track1.synth.waveform.value = waveNorm;
            studio.track1.synth.chaosMix.value = chaosNorm * 100.0f;
        } else if (activeDrag == DRAG_SYNTH_ENV_XY) {
            float relNorm = CLAMP((float)(mx - (synthEnvXyRect.x + 6)) / (float)(synthEnvXyRect.w - 12), 0.0f, 1.0f);
            float envNorm = CLAMP((float)(synthEnvXyRect.y + synthEnvXyRect.h - 6 - my) / (float)(synthEnvXyRect.h - 20), 0.0f, 1.0f);
            studio.track1.synth.release.value = 10.0f + relNorm * 1990.0f;
            studio.track1.synth.envAmt.value = envNorm;
        } else if (activeDrag == DRAG_SYNTH_LFO_CHAOS_XY) {
            float lfoNorm = CLAMP((float)(mx - (synthLfoChaosXyRect.x + 6)) / (float)(synthLfoChaosXyRect.w - 12), 0.0f, 1.0f);
            float chaosFmNorm = CLAMP((float)(synthLfoChaosXyRect.y + synthLfoChaosXyRect.h - 6 - my) / (float)(synthLfoChaosXyRect.h - 20), 0.0f, 1.0f);
            studio.track1.synth.lfoSpeed.value = lfoNorm * 100.0f;
            studio.track1.synth.fmDepth.value = chaosFmNorm * 100.0f;
        } else if (activeDrag == DRAG_SYNTH_FX_XY) {
            float drvNorm = CLAMP((float)(mx - (synthFxXyRect.x + 6)) / (float)(synthFxXyRect.w - 12), 0.0f, 1.0f);
            float rvbNorm = CLAMP((float)(synthFxXyRect.y + synthFxXyRect.h - 6 - my) / (float)(synthFxXyRect.h - 20), 0.0f, 1.0f);
            studio.track1.synth.drive.value = drvNorm * 100.0f;
            studio.track1.synth.reverbMix.value = rvbNorm * 100.0f;
        } else if (activeDrag == DRAG_SYNTH_DELAY_XY) {
            float dlyTimeNorm = CLAMP((float)(mx - (synthDelayXyRect.x + 6)) / (float)(synthDelayXyRect.w - 12), 0.0f, 1.0f);
            float dlyMixNorm = CLAMP((float)(synthDelayXyRect.y + synthDelayXyRect.h - 6 - my) / (float)(synthDelayXyRect.h - 20), 0.0f, 1.0f);
            studio.track1.synth.dlyTime.value = 10.0f + dlyTimeNorm * 990.0f;
            studio.track1.synth.dlyMix.value = dlyMixNorm * 100.0f;
        } else if (activeDrag == DRAG_SYNTH_CRUSH_COLOR_XY) {
            float colNorm = CLAMP((float)(mx - (synthColorCrushXyRect.x + 6)) / (float)(synthColorCrushXyRect.w - 12), 0.0f, 1.0f);
            float crushNorm = CLAMP((float)(synthColorCrushXyRect.y + synthColorCrushXyRect.h - 6 - my) / (float)(synthColorCrushXyRect.h - 20), 0.0f, 1.0f);
            studio.track1.synth.color.value = colNorm * 100.0f;
            studio.track1.synth.crushFm.value = -100.0f + crushNorm * 200.0f;
        } else if (activeDrag == DRAG_SYNTH_SEMITONE_BAR) {
            float norm = CLAMP((float)(mx - (semitoneBarSynthRect.x + 6)) / (float)(semitoneBarSynthRect.w - 12), 0.0f, 1.0f);
            int newPitch = 24 + (int)std::round(norm * 72.0f);
            studio.track1.synth.pitch.value = (float)newPitch;
        } else if (activeDrag == DRAG_STEP_NOTE_KICK && dragStepIdx >= 0 && dragStepIdx < SEQ_STEPS_TEK) {
            int dy = (dragStartY - my) / 6;
            int newNote = CLAMP(dragStartNote + dy, 36, 84);
            studio.track0.sequence[dragStepIdx].note = newNote;
        } else if (activeDrag == DRAG_STEP_NOTE_SYNTH && dragStepIdx >= 0 && dragStepIdx < SEQ_STEPS_TEK) {
            int dy = (dragStartY - my) / 6;
            int newNote = CLAMP(dragStartNote + dy, 24, 96);
            studio.track1.sequence[dragStepIdx].note = newNote;
        }
    }

    void onMouseUp()
    {
        activeDrag = DRAG_NONE;
        dragStepIdx = -1;
    }

    void onMouseWheel(int mx, int my, float delta)
    {
        std::lock_guard<std::mutex> lock(studio.audioMutex);

        if (bpmRect.contains(mx, my)) {
            float newBpm = CLAMP(studio.bpm.load() + (delta > 0 ? 1.0f : -1.0f), 40.0f, 280.0f);
            studio.bpm.store(newBpm);
            studio.updateClock();
            return;
        }

        if (volumeKickSliderRect.contains(mx, my)) {
            studio.track0.volume = CLAMP(studio.track0.volume + (delta > 0 ? 0.05f : -0.05f), 0.0f, 1.0f);
            return;
        }

        if (volumeSynthSliderRect.contains(mx, my)) {
            studio.track1.volume = CLAMP(studio.track1.volume + (delta > 0 ? 0.05f : -0.05f), 0.0f, 1.0f);
            return;
        }

        if (mmFilterXyRect.contains(mx, my)) {
            float newCut = CLAMP(studio.track0.kick.filterCutoff.value + (delta > 0 ? 5.0f : -5.0f), -100.0f, 100.0f);
            studio.track0.kick.filterCutoff.value = newCut;
            return;
        }

        if (durationBarRect.contains(mx, my)) {
            Param& p = studio.track0.kick.duration;
            p.value = CLAMP(p.value + (delta > 0 ? 10.0f : -10.0f), p.min, p.max);
            return;
        }

        if (semitoneBarKickRect.contains(mx, my)) {
            int curSemi = studio.track0.kick.semitoneOffset.load();
            studio.track0.kick.semitoneOffset.store(std::clamp(curSemi + (delta > 0 ? 1 : -1), -12, 12));
            return;
        }

        if (fmRatioBarRect.contains(mx, my)) {
            Param& p = studio.track0.kick.fmRatio;
            p.value = CLAMP(p.value + (delta > 0 ? 0.25f : -0.25f), p.min, p.max);
            return;
        }

        if (foldBarRect.contains(mx, my)) {
            Param& p = studio.track0.kick.fold;
            p.value = CLAMP(p.value + (delta > 0 ? 5.0f : -5.0f), p.min, p.max);
            return;
        }

        if (sweepCurveRect.contains(mx, my)) {
            studio.track0.kick.sweepShp.value = CLAMP(studio.track0.kick.sweepShp.value + (delta > 0 ? 2.0f : -2.0f), 0.0f, 100.0f);
            return;
        }

        if (vcoMorphRect.contains(mx, my)) {
            studio.track0.kick.vcoMorph.value = CLAMP(studio.track0.kick.vcoMorph.value + (delta > 0 ? 2.0f : -2.0f), 0.0f, 100.0f);
            return;
        }

        for (int i = 0; i < SEQ_STEPS_TEK; i++) {
            if (stepRects0[i].contains(mx, my)) {
                studio.track0.sequence[i].note = CLAMP(studio.track0.sequence[i].note + (delta > 0 ? 1 : -1), 36, 84);
                return;
            }
            if (stepRects1[i].contains(mx, my)) {
                studio.track1.sequence[i].note = CLAMP(studio.track1.sequence[i].note + (delta > 0 ? 1 : -1), 24, 96);
                return;
            }
        }
    }
};

extern UiZicTeK uiZicTeK;
