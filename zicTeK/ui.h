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
    DRAG_SYNTH_PARAM_BASE,
    DRAG_SYNTH_VOLUME = DRAG_SYNTH_PARAM_BASE + 30,
    // Drum Drag Modes
    DRAG_DRUM_PARAM_BASE,
    DRAG_VOLUME_DRUM = DRAG_DRUM_PARAM_BASE + 10,
    // Sequencer / Global
    DRAG_STEP_NOTE_KICK,
    DRAG_STEP_NOTE_SYNTH,
    DRAG_STEP_NOTE_DRUM,
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
    int dragEqBand = 0;
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
    BoxRect synthBarRects[24];
    BoxRect volumeSynthSliderRect;

    // Track 2 (Tribe Drums) Widgets
    BoxRect drumBarRects[4];
    BoxRect volumeDrumSliderRect;
    BoxRect genTribeBtnRect;
    BoxRect undoTribeBtnRect;

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

    // Sequencer Hit Boxes (Track 2: Tribe Drums)
    BoxRect rowCheckRects2[4];
    BoxRect rowGenRects2[4];
    BoxRect colEnableRects2[16];
    BoxRect stepRects2[SEQ_STEPS_TEK];

    UiZicTeK()
    {
    }

    static void generateMentalTribeDrums(Step* sequence)
    {
        for (int i = 0; i < 64; i++) {
            sequence[i].active = false;
            sequence[i].note = 60;
            sequence[i].velocity = 0.9f;
        }

        // Row 0: Snare / Rim (steps 4, 12 on beats 2 & 4, plus mental tribe ghost accents)
        sequence[4].active = true;
        sequence[12].active = true;
        if (Generator::rand01() > 0.35f) sequence[14].active = true;
        if (Generator::rand01() > 0.55f) sequence[10].active = true;
        if (Generator::rand01() > 0.70f) sequence[6].active = true;

        // Row 1: Closed HH (driving 16ths / offbeats)
        int hhType = rand() % 3;
        for (int c = 0; c < 16; c++) {
            if (hhType == 0) {
                if (c % 4 != 0 || Generator::rand01() > 0.5f) sequence[16 + c].active = true;
            } else if (hhType == 1) {
                if (c % 2 == 1 || c % 4 == 2) sequence[16 + c].active = true;
            } else {
                if (c % 4 == 1 || c % 4 == 2 || c % 4 == 3) sequence[16 + c].active = true;
            }
        }

        // Row 2: Open HH (offbeat accents on steps 2, 6, 10, 14 or variations)
        if (Generator::rand01() > 0.25f) sequence[32 + 2].active = true;
        if (Generator::rand01() > 0.25f) sequence[32 + 6].active = true;
        if (Generator::rand01() > 0.25f) sequence[32 + 10].active = true;
        if (Generator::rand01() > 0.25f) sequence[32 + 14].active = true;

        // Row 3: Clap / Mental Zap (beats 2 & 4 or syncopated tribe zap)
        sequence[48 + 4].active = true;
        sequence[48 + 12].active = true;
        if (Generator::rand01() > 0.5f) sequence[48 + 7].active = true;
        if (Generator::rand01() > 0.6f) sequence[48 + 15].active = true;
    }

    void generateRowPattern(int trackIdx, int row)
    {
        if (row < 0 || row >= 4) return;
        int baseIdx = row * 16;

        if (trackIdx == 2) {
            studio.track2.saveHistory();
            if (row == 0) { // Snare
                for (int i = 0; i < 16; i++) studio.track2.sequence[baseIdx + i].active = (i == 4 || i == 12 || (i == 14 && Generator::rand01() > 0.4f));
            } else if (row == 1) { // Closed HH
                for (int i = 0; i < 16; i++) studio.track2.sequence[baseIdx + i].active = (i % 2 == 1 || Generator::rand01() > 0.4f);
            } else if (row == 2) { // Open HH
                for (int i = 0; i < 16; i++) studio.track2.sequence[baseIdx + i].active = (i % 4 == 2 && Generator::rand01() > 0.3f);
            } else if (row == 3) { // Clap
                for (int i = 0; i < 16; i++) studio.track2.sequence[baseIdx + i].active = (i == 4 || i == 12 || (i == 7 && Generator::rand01() > 0.6f));
            }
            return;
        }

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
        d.text({ 94, 14 }, "3-TRACK TEKNO WORKSTATION (KICK + TEKSYNTH + TRIBE DRUMS)", 8, { .color = { 140, 165, 195, 255 }, .font = &PoppinsLight_8 });

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

        // 2. FREQ BAR (16px HEIGHT, ALIGNED WITH BOTTOM OF CLICK AND DRIVE AT 304px!)
        int freqY = durY + 16 + 4;
        kickSubFreqBarRect = { col2X, freqY, colW, 16 };
        d.filledRect({ kickSubFreqBarRect.x, kickSubFreqBarRect.y }, { kickSubFreqBarRect.w, kickSubFreqBarRect.h }, { .color = { 14, 18, 28, 255 } });

        Param& subP = studio.track0.kick.baseFreq;
        float subNorm = (subP.value - subP.min) / (subP.max - subP.min);
        int subFillW = (int)(kickSubFreqBarRect.w * subNorm);
        if (subFillW > 0) {
            d.filledRect({ kickSubFreqBarRect.x, kickSubFreqBarRect.y }, { subFillW, kickSubFreqBarRect.h }, { .color = { 0, 190, 160, 255 } });
        }
        d.rect({ kickSubFreqBarRect.x, kickSubFreqBarRect.y }, { kickSubFreqBarRect.w, kickSubFreqBarRect.h }, { .color = { 0, 230, 170, 255 } });

        std::ostringstream subTxt;
        subTxt << "FREQ " << (int)subP.value << " Hz";
        d.textCentered({ kickSubFreqBarRect.x + kickSubFreqBarRect.w / 2 + 1, kickSubFreqBarRect.y + 4 }, subTxt.str(), 8, { .color = { 0, 0, 0, 255 }, .font = &PoppinsLight_8 });
        d.textCentered({ kickSubFreqBarRect.x + kickSubFreqBarRect.w / 2, kickSubFreqBarRect.y + 3 }, subTxt.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

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

        // --- COLUMN 1 (LEFT ROW 3 - 20% HEIGHT = 115px UNDER CLICK): 3-BAND EQ (LOW | MID | HIGH) ---
        int row3Y = row2Y + stackedH + 4;
        mmFilterXyRect = { col1X, row3Y, colW, stackedH };
        d.filledRect({ mmFilterXyRect.x, mmFilterXyRect.y }, { mmFilterXyRect.w, mmFilterXyRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ mmFilterXyRect.x, mmFilterXyRect.y }, { mmFilterXyRect.w, mmFilterXyRect.h }, { .color = { 255, 90, 160, 255 } });
        d.text({ mmFilterXyRect.x + 6, mmFilterXyRect.y + 4 }, "3-BAND EQ (LOW | MID | HIGH)", 8, { .color = { 255, 120, 180, 255 }, .font = &PoppinsLight_8 });

        // Center 0dB reference line
        int eqZeroY = mmFilterXyRect.y + 16 + (mmFilterXyRect.h - 24) / 2;
        d.line({ mmFilterXyRect.x + 4, eqZeroY }, { mmFilterXyRect.x + mmFilterXyRect.w - 4, eqZeroY }, { .color = { 50, 50, 65, 255 } });

        // Draw EQ response curve using audio/Eq.h curvePoints
        auto pts = studio.track0.kick.eq.curvePoints(
            (float)(mmFilterXyRect.x + 4), (float)(mmFilterXyRect.y + 16),
            (float)(mmFilterXyRect.w - 8), (float)(mmFilterXyRect.h - 24),
            12.0f, 44100.0, 80);

        for (size_t i = 1; i < pts.size(); i++) {
            d.line({ (int)pts[i - 1].first, (int)pts[i - 1].second },
                { (int)pts[i].first, (int)pts[i].second }, { .color = { 255, 140, 180, 255 } });
        }

        // Draw 3 band control dots (Low, Mid, High)
        for (int b = 0; b < 3; b++) {
            auto [px, py] = studio.track0.kick.eq.dotPos(
                b, (float)(mmFilterXyRect.x + 4), (float)(mmFilterXyRect.y + 16),
                (float)(mmFilterXyRect.w - 8), (float)(mmFilterXyRect.h - 24), 12.0f);

            d.filledCircle({ (int)px, (int)py }, 4, { .color = { 255, 220, 240, 255 } });
            d.circle({ (int)px, (int)py }, 4, { .color = { 255, 100, 160, 255 } });
        }

        // Display current dB gain values
        std::ostringstream eqTxt;
        eqTxt << "LO:" << (studio.track0.kick.eqLow.value >= 0 ? "+" : "") << (int)studio.track0.kick.eqLow.value
              << " MID:" << (studio.track0.kick.eqMid.value >= 0 ? "+" : "") << (int)studio.track0.kick.eqMid.value
              << " HI:" << (studio.track0.kick.eqHigh.value >= 0 ? "+" : "") << (int)studio.track0.kick.eqHigh.value;

        d.textRight({ mmFilterXyRect.x + mmFilterXyRect.w - 6, mmFilterXyRect.y + 4 }, eqTxt.str(), 8, { .color = { 255, 170, 200, 255 }, .font = &PoppinsLight_8 });

        if (kickPulseLevel > 0.01f) {
            uint8_t pulseAlpha = (uint8_t)(kickPulseLevel * 220.0f);
            d.circle({ mmFilterXyRect.x + mmFilterXyRect.w / 2, eqZeroY }, 8, { .color = { 255, 120, 180, pulseAlpha } });
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

        // --- 64-STEP SEQUENCER STICKING TO THE BOTTOM ---
        drawSequencer(d, px, seqY, pw, 0);
    }

    // --- TRACK 1 (TEKSYNTH) PANEL RENDERING (3 COLUMNS x 8 ROWS HORIZONTAL SLIDER BARS) ---
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
        int seqH = 114;
        int seqY = py + ph - seqH - 4;

        int curY = py + 30;
        int contentW = pw - 16;
        int colGap = 6;
        int colW = (contentW - 2 * colGap) / 3;

        int col1X = px + 8;
        int col2X = col1X + colW + colGap;
        int col3X = col2X + colW + colGap;

        Param* synthParams[24] = {
            &studio.track1.synth.pitch,
            &studio.track1.synth.waveform,
            &studio.track1.synth.cutoff,
            &studio.track1.synth.resonance,
            &studio.track1.synth.release,
            &studio.track1.synth.envAmt,
            &studio.track1.synth.filterMorph,
            &studio.track1.synth.lfoSpeed,
            &studio.track1.synth.lfoShape,
            &studio.track1.synth.lfoToCutoff,
            &studio.track1.synth.lfoToPitch,
            &studio.track1.synth.lfoToMorph,

            &studio.track1.synth.lfoToLevel,
            &studio.track1.synth.lfoToCrushFm,
            &studio.track1.synth.fmDepth,
            &studio.track1.synth.ringMod,
            &studio.track1.synth.pitchGlitch,
            &studio.track1.synth.crushFm,
            &studio.track1.synth.drive,
            &studio.track1.synth.reverbMix,
            &studio.track1.synth.reverbDamp,
            &studio.track1.synth.dlyMix,
            &studio.track1.synth.dlyTime,
            &studio.track1.synth.dlyFdbk
        };

        for (int i = 0; i < 24; i++) {
            int col = i / 8;
            int row = i % 8;
            int bx = (col == 0) ? col1X : ((col == 1) ? col2X : col3X);
            int by = curY + row * 23;
            int bw = colW;
            int bh = 19;

            synthBarRects[i] = { bx, by, bw, bh };

            Param& p = *synthParams[i];

            // Background
            d.filledRect({ bx, by }, { bw, bh }, { .color = { 16, 20, 30, 255 } });

            bool isBipolar = (p.min < 0.0f);
            if (isBipolar) {
                int centerX = bx + bw / 2;
                float norm = CLAMP((p.value - p.min) / (p.max - p.min), 0.0f, 1.0f);
                int fillX = bx + (int)(bw * std::min(norm, 0.5f));
                int fillW = (int)(bw * std::abs(norm - 0.5f));
                if (fillW > 0) {
                    d.filledRect({ fillX, by }, { fillW, bh }, { .color = { 220, 120, 0, 255 } });
                }
                d.line({ centerX, by }, { centerX, by + bh }, { .color = { 160, 160, 160, 255 } });
            } else {
                float norm = CLAMP((p.value - p.min) / (p.max - p.min), 0.0f, 1.0f);
                int barFillW = (int)(bw * norm);
                if (barFillW > 0) {
                    Color barCol = (col == 0) ? Color { 220, 130, 0, 255 } : ((col == 1) ? Color { 0, 160, 210, 255 } : Color { 180, 80, 220, 255 });
                    d.filledRect({ bx, by }, { barFillW, bh }, { .color = barCol });
                }
            }

            Color borderCol = (col == 0) ? Color { 255, 170, 0, 255 } : ((col == 1) ? Color { 0, 195, 255, 255 } : Color { 220, 100, 255, 255 });
            d.rect({ bx, by }, { bw, bh }, { .color = borderCol });

            std::ostringstream txt;
            if (p.key == "pitch") {
                int noteNum = (int)p.value;
                txt << "PITCH: " << (noteNum >= 0 && noteNum < 132 ? MIDI_NOTES_STR[noteNum] : "") << " (" << noteNum << ")";
            } else if (p.unit == "%") {
                txt << p.label << ": " << (int)p.value << "%";
            } else if (p.unit == "ms") {
                txt << p.label << ": " << (int)p.value << " ms";
            } else {
                txt << p.label << ": " << std::fixed << std::setprecision(2) << p.value;
            }

            d.textCentered({ bx + bw / 2 + 1, by + 6 }, txt.str(), 8, { .color = { 0, 0, 0, 255 }, .font = &PoppinsLight_8 });
            d.textCentered({ bx + bw / 2, by + 5 }, txt.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
        }

        // 64-Step Sequencer
        drawSequencer(d, px, seqY, pw, 1);
    }

    // --- TRACK 3 (TEKNO TRIBE DRUMS) PANEL RENDERING ---
    void drawTrack3Panel(Draw& d, int px, int py, int pw, int ph)
    {
        // Panel Background & Deep Purple/Teal Border
        d.filledRect({ px, py }, { pw, ph }, { .color = { 12, 14, 20, 255 } });
        d.rect({ px, py }, { pw, ph }, { .color = { 180, 70, 220, 255 } });

        // Header Bar
        d.filledRect({ px + 1, py + 1 }, { pw - 2, 26 }, { .color = { 32, 20, 38, 255 } });
        d.line({ px, py + 27 }, { px + pw, py + 27 }, { .color = { 180, 70, 220, 255 } });
        d.text({ px + 10, py + 6 }, "TRACK 3: TEKNO TRIBE DRUMS", 12, { .color = { 220, 140, 255, 255 }, .font = &PoppinsLight_12 });

        // Volume Level Slider in Header
        int volW = 85;
        volumeDrumSliderRect = { px + pw - volW - 10, py + 5, volW, 16 };
        d.filledRect({ volumeDrumSliderRect.x, volumeDrumSliderRect.y }, { volumeDrumSliderRect.w, volumeDrumSliderRect.h }, { .color = { 38, 24, 46, 255 } });
        int fillW = (int)(volumeDrumSliderRect.w * CLAMP(studio.track2.volume, 0.0f, 1.0f));
        d.filledRect({ volumeDrumSliderRect.x, volumeDrumSliderRect.y }, { fillW, volumeDrumSliderRect.h }, { .color = { 200, 90, 240, 255 } });
        d.rect({ volumeDrumSliderRect.x, volumeDrumSliderRect.y }, { volumeDrumSliderRect.w, volumeDrumSliderRect.h }, { .color = { 130, 60, 160, 255 } });
        std::ostringstream volStr;
        volStr << "VOL " << (int)(studio.track2.volume * 100.0f) << "%";
        d.textCentered({ volumeDrumSliderRect.x + volW / 2, volumeDrumSliderRect.y + 2 }, volStr.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        // Top Action Buttons: [GENERATE TRIBE] and [UNDO / PREV]
        int curY = py + 32;
        int btnW = 145;
        int btnH = 20;

        genTribeBtnRect = { px + 10, curY, btnW, btnH };
        d.filledRect({ genTribeBtnRect.x, genTribeBtnRect.y }, { genTribeBtnRect.w, genTribeBtnRect.h }, { .color = { 140, 40, 180, 255 } });
        d.rect({ genTribeBtnRect.x, genTribeBtnRect.y }, { genTribeBtnRect.w, genTribeBtnRect.h }, { .color = { 220, 120, 255, 255 } });
        d.textCentered({ genTribeBtnRect.x + btnW / 2, genTribeBtnRect.y + 4 }, "GEN TRIBE PATTERN", 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        undoTribeBtnRect = { px + 10 + btnW + 10, curY, 100, btnH };
        Color undoBg = studio.track2.hasPrevSequence ? Color { 45, 55, 75, 255 } : Color { 25, 30, 40, 255 };
        Color undoBdr = studio.track2.hasPrevSequence ? Color { 100, 140, 190, 255 } : Color { 50, 60, 75, 255 };
        d.filledRect({ undoTribeBtnRect.x, undoTribeBtnRect.y }, { undoTribeBtnRect.w, undoTribeBtnRect.h }, { .color = undoBg });
        Color undoTxtCol = studio.track2.hasPrevSequence ? Color { 220, 235, 255, 255 } : Color { 100, 110, 130, 255 };
        d.textCentered({ undoTribeBtnRect.x + undoTribeBtnRect.w / 2, undoTribeBtnRect.y + 4 }, "UNDO / PREV", 8, { .color = undoTxtCol, .font = &PoppinsLight_8 });

        // Minimal Parameter Bars (4 controls: SNARE VOL, HIHAT VOL, CLAP VOL, CUTOFF)
        int paramY = curY + 24;
        int contentW = pw - 16;
        int colGap = 6;
        int colW = (contentW - 3 * colGap) / 4;

        struct DrumParamDef {
            const char* label;
            float* valPtr;
            bool isCutoff;
        } dParams[4] = {
            { "SNARE", &studio.track2.drums.snareVol, false },
            { "HI-HAT", &studio.track2.drums.hhVol, false },
            { "CLAP", &studio.track2.drums.clapVol, false },
            { "CUTOFF", &studio.track2.drums.drumCutoff, true }
        };

        for (int i = 0; i < 4; i++) {
            int bx = px + 8 + i * (colW + colGap);
            int by = paramY;
            int bw = colW;
            int bh = 19;

            drumBarRects[i] = { bx, by, bw, bh };

            d.filledRect({ bx, by }, { bw, bh }, { .color = { 16, 20, 30, 255 } });

            if (dParams[i].isCutoff) {
                float norm = CLAMP((*dParams[i].valPtr + 100.0f) / 200.0f, 0.0f, 1.0f);
                int fillW = (int)(bw * norm);
                if (fillW > 0) d.filledRect({ bx, by }, { fillW, bh }, { .color = { 0, 160, 210, 255 } });
                d.rect({ bx, by }, { bw, bh }, { .color = { 0, 195, 255, 255 } });
                std::ostringstream pTxt;
                pTxt << "CUTOFF: " << (int)*dParams[i].valPtr;
                d.textCentered({ bx + bw / 2, by + 5 }, pTxt.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
            } else {
                float norm = CLAMP(*dParams[i].valPtr, 0.0f, 1.0f);
                int fillW = (int)(bw * norm);
                if (fillW > 0) d.filledRect({ bx, by }, { fillW, bh }, { .color = { 180, 80, 220, 255 } });
                d.rect({ bx, by }, { bw, bh }, { .color = { 220, 100, 255, 255 } });
                std::ostringstream pTxt;
                pTxt << dParams[i].label << ": " << (int)(*dParams[i].valPtr * 100.0f) << "%";
                d.textCentered({ bx + bw / 2, by + 5 }, pTxt.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
            }
        }

        // 64-Step Sequencer (4 drum lanes) Sticking to Bottom
        int seqH = 114;
        int seqY = py + ph - seqH - 4;
        drawSequencer(d, px, seqY, pw, 2);
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
        int track2H = gridH / 2;

        // Track 0 (Left Side: Massive Kick)
        drawTrack0Panel(d, 0, headH, trackW, gridH);

        // Track 1 (Top Right: TeKSynth)
        drawTrack1Panel(d, trackW, headH, trackW, track2H);

        // Track 2 (Bottom Right: Tekno Tribe Drums)
        drawTrack3Panel(d, trackW, headH + track2H, trackW, gridH - track2H);
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

        auto& sequence = (trackIdx == 0) ? studio.track0.sequence : ((trackIdx == 1) ? studio.track1.sequence : studio.track2.sequence);
        auto& rowEnabled = (trackIdx == 0) ? studio.track0.rowEnabled : ((trackIdx == 1) ? studio.track1.rowEnabled : studio.track2.rowEnabled);
        BoxRect* colRects = (trackIdx == 0) ? colEnableRects0 : ((trackIdx == 1) ? colEnableRects1 : colEnableRects2);
        BoxRect* rowChkRects = (trackIdx == 0) ? rowCheckRects0 : ((trackIdx == 1) ? rowCheckRects1 : rowCheckRects2);
        BoxRect* rowGRects = (trackIdx == 0) ? rowGenRects0 : ((trackIdx == 1) ? rowGenRects1 : rowGenRects2);
        BoxRect* stpRects = (trackIdx == 0) ? stepRects0 : ((trackIdx == 1) ? stepRects1 : stepRects2);

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
        static const char* DRUM_LANE_NAMES[4] = { "SNR", "CHH", "OHH", "CLP" };

        for (int r = 0; r < 4; r++) {
            int ry = rowStartY + r * (stepBoxH + 2);
            bool isRowOn = rowEnabled[r];

            // 1. Left Checkbox [X]
            rowChkRects[r] = { px + 8, ry + (stepBoxH - 12) / 2, 13, 12 };
            Color cbBg = isRowOn ? (trackIdx == 0 ? Color { 0, 160, 120, 255 } : ((trackIdx == 1) ? Color { 220, 120, 0, 255 } : Color { 180, 80, 220, 255 })) : Color { 24, 28, 36, 255 };
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
                bool isCurrent = (playHead % 16 == c && studio.isPlaying);

                Color stepBg;
                if (!isRowOn) {
                    stepBg = stp.active ? Color { 50, 60, 70, 180 } : Color { 14, 18, 26, 255 };
                } else if (stp.active) {
                    if (trackIdx == 0) {
                        stepBg = (c % 4 == 0) ? Color { 0, 190, 230, 255 } : Color { 0, 150, 190, 255 };
                    } else if (trackIdx == 1) {
                        stepBg = (c % 4 == 0) ? Color { 255, 160, 0, 255 } : Color { 220, 120, 0, 255 };
                    } else {
                        stepBg = (c % 4 == 0) ? Color { 220, 90, 255, 255 } : Color { 170, 60, 210, 255 };
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
                    if (trackIdx == 2) {
                        d.textCentered({ sx + stepBoxW / 2, ry + 5 }, DRUM_LANE_NAMES[r], 8, { .color = isRowOn ? Color { 255, 255, 255, 255 } : Color { 140, 160, 180, 255 }, .font = &PoppinsLight_8 });
                    } else {
                        std::string noteStr = (stp.note >= 0 && stp.note < 132) ? MIDI_NOTES_STR[stp.note] : "C4";
                        d.textCentered({ sx + stepBoxW / 2, ry + 5 }, noteStr, 8, { .color = isRowOn ? Color { 255, 255, 255, 255 } : Color { 140, 160, 180, 255 }, .font = &PoppinsLight_8 });
                    }
                } else {
                    std::ostringstream numStr;
                    numStr << (c + 1);
                    d.textCentered({ sx + stepBoxW / 2, ry + 5 }, numStr.str(), 8, { .color = isRowOn ? Color { 80, 100, 130, 255 } : Color { 45, 55, 70, 255 }, .font = &PoppinsLight_8 });
                }
            }
        }
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

        if (volumeSynthSliderRect.contains(mx, my)) {
            activeDrag = DRAG_SYNTH_VOLUME;
            float norm = CLAMP((float)(mx - volumeSynthSliderRect.x) / (float)volumeSynthSliderRect.w, 0.0f, 1.0f);
            studio.track1.volume = norm;
            return;
        }

        // --- TRACK 2 (TRIBE DRUMS) MOUSE DOWN ---
        if (volumeDrumSliderRect.contains(mx, my)) {
            activeDrag = DRAG_VOLUME_DRUM;
            float norm = CLAMP((float)(mx - volumeDrumSliderRect.x) / (float)volumeDrumSliderRect.w, 0.0f, 1.0f);
            studio.track2.volume = norm;
            return;
        }

        if (genTribeBtnRect.contains(mx, my)) {
            studio.track2.saveHistory();
            generateMentalTribeDrums(studio.track2.sequence);
            return;
        }

        if (undoTribeBtnRect.contains(mx, my)) {
            studio.track2.undoPattern();
            return;
        }

        for (int i = 0; i < 4; i++) {
            if (drumBarRects[i].contains(mx, my)) {
                activeDrag = (DragMode)(DRAG_DRUM_PARAM_BASE + i);
                float norm = CLAMP((float)(mx - drumBarRects[i].x) / (float)drumBarRects[i].w, 0.0f, 1.0f);
                if (i == 0) studio.track2.drums.snareVol = norm;
                else if (i == 1) studio.track2.drums.hhVol = norm;
                else if (i == 2) studio.track2.drums.clapVol = norm;
                else if (i == 3) studio.track2.drums.drumCutoff = -100.0f + norm * 200.0f;
                return;
            }
        }

        if (mmFilterXyRect.contains(mx, my)) {
            activeDrag = DRAG_MM_FILTER_XY;
            float relX = (float)(mx - mmFilterXyRect.x) / (float)mmFilterXyRect.w;
            float gainNorm = CLAMP((float)(mmFilterXyRect.y + mmFilterXyRect.h - 6 - my) / (float)(mmFilterXyRect.h - 20), 0.0f, 1.0f);
            float dbVal = -12.0f + gainNorm * 24.0f;

            if (relX < 0.333f) {
                studio.track0.kick.eqLow.value = dbVal;
                dragEqBand = 0;
            } else if (relX < 0.666f) {
                studio.track0.kick.eqMid.value = dbVal;
                dragEqBand = 1;
            } else {
                studio.track0.kick.eqHigh.value = dbVal;
                dragEqBand = 2;
            }
            return;
        }

        if (sweepCurveRect.contains(mx, my)) {
            activeDrag = DRAG_SWEEP_XY;
            float shpNorm = CLAMP((float)(mx - (sweepCurveRect.x + 6)) / (float)(sweepCurveRect.w - 12), 0.0f, 1.0f);
            float depthNorm = CLAMP((float)(sweepCurveRect.y + sweepCurveRect.h - 6 - my) / (float)(sweepCurveRect.h - 20), 0.0f, 1.0f);
            studio.track0.kick.sweepShp.value = shpNorm * 100.0f;
            studio.track0.kick.sweepDepth.value = depthNorm * 100.0f;
            return;
        }

        if (vcoMorphRect.contains(mx, my)) {
            int vcoBarY = vcoMorphRect.y + vcoMorphRect.h - 22;
            if (my >= vcoBarY) {
                activeDrag = DRAG_VCO_MORPH_BAR;
                float norm = CLAMP((float)(mx - (vcoMorphRect.x + 8)) / (float)(vcoMorphRect.w - 16), 0.0f, 1.0f);
                studio.track0.kick.vcoMorph.value = norm * 100.0f;
            } else {
                activeDrag = DRAG_VCO_MORPH_BODY;
                dragStartX = mx;
                dragStartValX = studio.track0.kick.vcoMorph.value;
            }
            return;
        }

        if (clickXyRect.contains(mx, my)) {
            activeDrag = DRAG_CLICK_XY;
            float amtNorm = CLAMP((float)(mx - (clickXyRect.x + 6)) / (float)(clickXyRect.w - 12), 0.0f, 1.0f);
            float decNorm = CLAMP((float)(clickXyRect.y + clickXyRect.h - 6 - my) / (float)(clickXyRect.h - 20), 0.0f, 1.0f);
            studio.track0.kick.kickClickAmt.value = amtNorm * 100.0f;
            studio.track0.kick.kickClickDecay.value = 1.0f + decNorm * 99.0f;
            return;
        }

        if (fmXyRect.contains(mx, my)) {
            int padBodyH = fmXyRect.h - 18;
            if (my > fmXyRect.y + padBodyH) {
                activeDrag = DRAG_FM_RATIO_BAR;
                float norm = CLAMP((float)(mx - fmRatioBarRect.x) / (float)fmRatioBarRect.w, 0.0f, 1.0f);
                int seg = (int)std::round(norm * 30.0f);
                studio.track0.kick.fmRatio.value = 0.5f + seg * 0.25f;
            } else {
                activeDrag = DRAG_FM_XY;
                float depthNorm = CLAMP((float)(mx - (fmXyRect.x + 6)) / (float)(fmXyRect.w - 12), 0.0f, 1.0f);
                float snapNorm = CLAMP((float)(fmXyRect.y + padBodyH - 6 - my) / (float)(padBodyH - 20), 0.0f, 1.0f);
                studio.track0.kick.fmDepth.value = depthNorm * 100.0f;
                studio.track0.kick.fmSnap.value = 2.0f + snapNorm * 148.0f;
            }
            return;
        }

        if (driveXyRect.contains(mx, my)) {
            activeDrag = DRAG_DRIVE_XY;
            int padBodyH = driveXyRect.h - 18;
            float drvNorm = CLAMP((float)(mx - (driveXyRect.x + 6)) / (float)(driveXyRect.w - 12), 0.0f, 1.0f);
            float boostNorm = CLAMP((float)(driveXyRect.y + padBodyH - 6 - my) / (float)(padBodyH - 20), 0.0f, 1.0f);
            studio.track0.kick.drive.value = drvNorm * 100.0f;
            studio.track0.kick.bassBoost.value = boostNorm * 100.0f;
            return;
        }

        if (foldBarRect.contains(mx, my)) {
            activeDrag = DRAG_FOLD_BAR;
            float norm = CLAMP((float)(mx - foldBarRect.x) / (float)foldBarRect.w, 0.0f, 1.0f);
            int seg = (int)std::round(norm * 20.0f);
            studio.track0.kick.fold.value = seg * 5.0f;
            return;
        }

        if (durationBarRect.contains(mx, my)) {
            activeDrag = DRAG_DURATION_BAR;
            float norm = CLAMP((float)(mx - durationBarRect.x) / (float)durationBarRect.w, 0.0f, 1.0f);
            Param& p = studio.track0.kick.duration;
            p.value = p.min + norm * (p.max - p.min);
            return;
        }

        if (kickSubFreqBarRect.contains(mx, my)) {
            activeDrag = DRAG_KICK_SUB_FREQ;
            float norm = CLAMP((float)(mx - kickSubFreqBarRect.x) / (float)kickSubFreqBarRect.w, 0.0f, 1.0f);
            Param& p = studio.track0.kick.baseFreq;
            p.value = p.min + norm * (p.max - p.min);
            return;
        }

        // --- TRACK 1 (SYNTH) MOUSE DOWN ---
        Param* synthParams[24] = {
            &studio.track1.synth.pitch,
            &studio.track1.synth.waveform,
            &studio.track1.synth.cutoff,
            &studio.track1.synth.resonance,
            &studio.track1.synth.release,
            &studio.track1.synth.envAmt,
            &studio.track1.synth.filterMorph,
            &studio.track1.synth.lfoSpeed,
            &studio.track1.synth.lfoShape,
            &studio.track1.synth.lfoToCutoff,
            &studio.track1.synth.lfoToPitch,
            &studio.track1.synth.lfoToMorph,

            &studio.track1.synth.lfoToLevel,
            &studio.track1.synth.lfoToCrushFm,
            &studio.track1.synth.fmDepth,
            &studio.track1.synth.ringMod,
            &studio.track1.synth.pitchGlitch,
            &studio.track1.synth.crushFm,
            &studio.track1.synth.drive,
            &studio.track1.synth.reverbMix,
            &studio.track1.synth.reverbDamp,
            &studio.track1.synth.dlyMix,
            &studio.track1.synth.dlyTime,
            &studio.track1.synth.dlyFdbk
        };

        for (int i = 0; i < 24; i++) {
            if (synthBarRects[i].contains(mx, my)) {
                activeDrag = (DragMode)(DRAG_SYNTH_PARAM_BASE + i);
                Param& p = *synthParams[i];
                float norm = CLAMP((float)(mx - synthBarRects[i].x) / (float)synthBarRects[i].w, 0.0f, 1.0f);
                p.value = p.min + norm * (p.max - p.min);
                studio.track1.synth.noteOn((uint8_t)studio.track1.synth.pitch.value, 0.9f);
                studio.synthPulseTrigger.store(true);
                return;
            }
        }

        // --- SEQUENCER CLICK HANDLERS (TRACK 0, TRACK 1, TRACK 2) ---
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
            if (rowCheckRects2[r].contains(mx, my)) {
                studio.track2.rowEnabled[r] = !studio.track2.rowEnabled[r];
                return;
            }
            if (rowGenRects2[r].contains(mx, my)) {
                generateRowPattern(2, r);
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
            if (colEnableRects2[c].contains(mx, my)) {
                bool allActive = true;
                for (int r = 0; r < 4; r++) {
                    if (!studio.track2.sequence[r * 16 + c].active) allActive = false;
                }
                bool newState = !allActive;
                for (int r = 0; r < 4; r++) studio.track2.sequence[r * 16 + c].active = newState;
                return;
            }
        }

        for (int i = 0; i < SEQ_STEPS_TEK; i++) {
            if (stepRects0[i].contains(mx, my)) {
                studio.track0.sequence[i].active = !studio.track0.sequence[i].active;
                activeDrag = DRAG_STEP_NOTE_KICK;
                dragStepIdx = i;
                return;
            }

            if (stepRects1[i].contains(mx, my)) {
                studio.track1.sequence[i].active = !studio.track1.sequence[i].active;
                activeDrag = DRAG_STEP_NOTE_SYNTH;
                dragStepIdx = i;

                if (studio.track1.sequence[i].active && !studio.isPlaying) {
                    studio.track1.synth.noteOn(studio.track1.sequence[i].note, studio.track1.sequence[i].velocity);
                    studio.synthPulseTrigger.store(true);
                }
                return;
            }

            if (stepRects2[i].contains(mx, my)) {
                studio.track2.sequence[i].active = !studio.track2.sequence[i].active;
                activeDrag = DRAG_STEP_NOTE_DRUM;
                dragStepIdx = i;

                if (studio.track2.sequence[i].active && !studio.isPlaying) {
                    int voice = i / 16;
                    studio.track2.drums.noteOn(voice, 60, 0.9f);
                    studio.drumPulseTrigger.store(true);
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
        } else if (activeDrag == DRAG_VOLUME_DRUM) {
            float norm = CLAMP((float)(mx - volumeDrumSliderRect.x) / (float)volumeDrumSliderRect.w, 0.0f, 1.0f);
            studio.track2.volume = norm;
        } else if (activeDrag >= DRAG_DRUM_PARAM_BASE && activeDrag < DRAG_DRUM_PARAM_BASE + 4) {
            int i = activeDrag - DRAG_DRUM_PARAM_BASE;
            float norm = CLAMP((float)(mx - drumBarRects[i].x) / (float)drumBarRects[i].w, 0.0f, 1.0f);
            if (i == 0) studio.track2.drums.snareVol = norm;
            else if (i == 1) studio.track2.drums.hhVol = norm;
            else if (i == 2) studio.track2.drums.clapVol = norm;
            else if (i == 3) studio.track2.drums.drumCutoff = -100.0f + norm * 200.0f;
        } else if (activeDrag == DRAG_MM_FILTER_XY) {
            float gainNorm = CLAMP((float)(mmFilterXyRect.y + mmFilterXyRect.h - 6 - my) / (float)(mmFilterXyRect.h - 20), 0.0f, 1.0f);
            float dbVal = -12.0f + gainNorm * 24.0f;

            if (dragEqBand == 0) {
                studio.track0.kick.eqLow.value = dbVal;
            } else if (dragEqBand == 1) {
                studio.track0.kick.eqMid.value = dbVal;
            } else {
                studio.track0.kick.eqHigh.value = dbVal;
            }
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
        } else if (activeDrag == DRAG_KICK_SUB_FREQ) {
            float norm = CLAMP((float)(mx - kickSubFreqBarRect.x) / (float)kickSubFreqBarRect.w, 0.0f, 1.0f);
            Param& p = studio.track0.kick.baseFreq;
            p.value = p.min + norm * (p.max - p.min);
        } else if (activeDrag >= DRAG_SYNTH_PARAM_BASE && activeDrag < DRAG_SYNTH_PARAM_BASE + 24) {
            int idx = activeDrag - DRAG_SYNTH_PARAM_BASE;
            Param* synthParams[24] = {
                &studio.track1.synth.pitch,
                &studio.track1.synth.waveform,
                &studio.track1.synth.cutoff,
                &studio.track1.synth.resonance,
                &studio.track1.synth.release,
                &studio.track1.synth.envAmt,
                &studio.track1.synth.filterMorph,
                &studio.track1.synth.lfoSpeed,
                &studio.track1.synth.lfoShape,
                &studio.track1.synth.lfoToCutoff,
                &studio.track1.synth.lfoToPitch,
                &studio.track1.synth.lfoToMorph,

                &studio.track1.synth.lfoToLevel,
                &studio.track1.synth.lfoToCrushFm,
                &studio.track1.synth.fmDepth,
                &studio.track1.synth.ringMod,
                &studio.track1.synth.pitchGlitch,
                &studio.track1.synth.crushFm,
                &studio.track1.synth.drive,
                &studio.track1.synth.reverbMix,
                &studio.track1.synth.reverbDamp,
                &studio.track1.synth.dlyMix,
                &studio.track1.synth.dlyTime,
                &studio.track1.synth.dlyFdbk
            };
            Param& p = *synthParams[idx];
            float norm = CLAMP((float)(mx - synthBarRects[idx].x) / (float)synthBarRects[idx].w, 0.0f, 1.0f);
            p.value = p.min + norm * (p.max - p.min);
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

        if (volumeDrumSliderRect.contains(mx, my)) {
            studio.track2.volume = CLAMP(studio.track2.volume + (delta > 0 ? 0.05f : -0.05f), 0.0f, 1.0f);
            return;
        }

        for (int i = 0; i < 4; i++) {
            if (drumBarRects[i].contains(mx, my)) {
                float step = (delta > 0 ? 0.05f : -0.05f);
                if (i == 0) studio.track2.drums.snareVol = CLAMP(studio.track2.drums.snareVol + step, 0.0f, 1.0f);
                else if (i == 1) studio.track2.drums.hhVol = CLAMP(studio.track2.drums.hhVol + step, 0.0f, 1.0f);
                else if (i == 2) studio.track2.drums.clapVol = CLAMP(studio.track2.drums.clapVol + step, 0.0f, 1.0f);
                else if (i == 3) studio.track2.drums.drumCutoff = CLAMP(studio.track2.drums.drumCutoff + (delta > 0 ? 5.0f : -5.0f), -100.0f, 100.0f);
                return;
            }
        }

        if (mmFilterXyRect.contains(mx, my)) {
            float relX = (float)(mx - mmFilterXyRect.x) / (float)mmFilterXyRect.w;
            float step = (delta > 0 ? 0.5f : -0.5f);
            if (relX < 0.333f) {
                studio.track0.kick.eqLow.value = CLAMP(studio.track0.kick.eqLow.value + step, -12.0f, 12.0f);
            } else if (relX < 0.666f) {
                studio.track0.kick.eqMid.value = CLAMP(studio.track0.kick.eqMid.value + step, -12.0f, 12.0f);
            } else {
                studio.track0.kick.eqHigh.value = CLAMP(studio.track0.kick.eqHigh.value + step, -12.0f, 12.0f);
            }
            return;
        }

        if (durationBarRect.contains(mx, my)) {
            Param& p = studio.track0.kick.duration;
            p.value = CLAMP(p.value + (delta > 0 ? 10.0f : -10.0f), p.min, p.max);
            return;
        }

        if (kickSubFreqBarRect.contains(mx, my)) {
            Param& p = studio.track0.kick.baseFreq;
            p.value = CLAMP(p.value + (delta > 0 ? 1.0f : -1.0f), p.min, p.max);
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

        Param* synthParams[24] = {
            &studio.track1.synth.pitch,
            &studio.track1.synth.waveform,
            &studio.track1.synth.cutoff,
            &studio.track1.synth.resonance,
            &studio.track1.synth.release,
            &studio.track1.synth.envAmt,
            &studio.track1.synth.filterMorph,
            &studio.track1.synth.lfoSpeed,
            &studio.track1.synth.lfoShape,
            &studio.track1.synth.lfoToCutoff,
            &studio.track1.synth.lfoToPitch,
            &studio.track1.synth.lfoToMorph,

            &studio.track1.synth.lfoToLevel,
            &studio.track1.synth.lfoToCrushFm,
            &studio.track1.synth.fmDepth,
            &studio.track1.synth.ringMod,
            &studio.track1.synth.pitchGlitch,
            &studio.track1.synth.crushFm,
            &studio.track1.synth.drive,
            &studio.track1.synth.reverbMix,
            &studio.track1.synth.reverbDamp,
            &studio.track1.synth.dlyMix,
            &studio.track1.synth.dlyTime,
            &studio.track1.synth.dlyFdbk
        };

        for (int i = 0; i < 24; i++) {
            if (synthBarRects[i].contains(mx, my)) {
                Param& p = *synthParams[i];
                float step = (p.step > 0.0f) ? p.step : ((p.max - p.min) * 0.02f);
                p.value = CLAMP(p.value + (delta > 0 ? step : -step), p.min, p.max);
                return;
            }
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
