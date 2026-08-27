#pragma once

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
    DRAG_SWEEP_XY,
    DRAG_VCO_MORPH_BODY,
    DRAG_VCO_MORPH_BAR,
    DRAG_CLICK_XY,
    DRAG_FM_XY,
    DRAG_FM_RATIO_BAR,
    DRAG_DRIVE_XY,
    DRAG_FOLD_BAR,
    DRAG_DURATION_BAR,
    DRAG_SEMITONE_BAR,
    DRAG_PARAM_SLIDER,
    DRAG_STEP_NOTE,
    DRAG_BPM,
    DRAG_VOLUME
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
    int dragParamIdx = -1;
    int dragStepIdx = -1;
    int dragStartX = 0;
    int dragStartY = 0;
    float dragStartValX = 0.0f;
    int dragStartValY = 0;
    int dragStartNote = 60;
    float animTime = 0.0f;
    float kickPulseLevel = 0.0f;

    // Interactive Widget Hit Boxes
    BoxRect playBtnRect;
    BoxRect bpmRect;

    BoxRect sweepCurveRect;
    BoxRect vcoMorphRect;
    BoxRect clickXyRect;
    BoxRect fmXyRect;
    BoxRect fmRatioBarRect;
    BoxRect driveXyRect;
    BoxRect foldBarRect;
    BoxRect durationBarRect;
    BoxRect semitoneBarRect;
    BoxRect volumeSliderRect;

    // Engine Parameter Sliders
    struct ParamSlider {
        std::string label;
        Param* param;
        BoxRect rect;
    };
    std::vector<ParamSlider> paramSliders;

    // Sequencer Hit Boxes & Controls
    BoxRect rowCheckRects[4];
    BoxRect rowGenRects[4];
    BoxRect colEnableRects[16];
    BoxRect stepRects[SEQ_STEPS_TEK];

    UiZicTeK()
    {
    }

    void generateRowPattern(int row)
    {
        if (row < 0 || row >= 4) return;
        int baseIdx = row * 16;
        static int patternPreset = 0;
        int style = (patternPreset++) % 5;

        for (int i = 0; i < 16; i++) {
            auto& stp = studio.track0.sequence[baseIdx + i];
            stp.note = 60;
            stp.velocity = 0.9f;

            if (style == 0) {
                stp.active = (i % 4 == 0);
            } else if (style == 1) {
                stp.active = (i == 0 || i == 3 || i == 6 || i == 8 || i == 10 || i == 14);
            } else if (style == 2) {
                stp.active = (i % 2 == 0);
            } else if (style == 3) {
                stp.active = (i == 0 || i == 3 || i == 4 || i == 7 || i == 8 || i == 11 || i == 12 || i == 15);
            } else {
                stp.active = (i == 0) || ((rand() % 100) < 35);
            }
        }
    }

    void triggerKickPulse()
    {
        kickPulseLevel = 1.0f;
    }

    void drawHeader(Draw& d, int winW, int winH)
    {
        int headH = 38;
        d.filledRect({ 0, 0 }, { winW, headH }, { .color = { 14, 18, 26, 255 } });
        d.line({ 0, headH - 1 }, { winW, headH - 1 }, { .color = { 0, 195, 255, 255 } });
        d.line({ 0, headH - 2 }, { winW, headH - 2 }, { .color = { 0, 90, 140, 150 } });

        // Branding Title
        d.text({ 14, 8 }, "zicTeK", 16, { .color = { 0, 230, 255, 255 }, .font = &PoppinsLight_16 });
        d.text({ 94, 14 }, "CYBER-TEK SYNTHESIZER WORKSTATION", 8, { .color = { 140, 165, 195, 255 }, .font = &PoppinsLight_8 });

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

    void drawEmptyPanel(Draw& d, int px, int py, int pw, int ph, const std::string& title)
    {
        d.filledRect({ px, py }, { pw, ph }, { .color = { 10, 13, 19, 255 } });
        d.rect({ px, py }, { pw, ph }, { .color = { 30, 38, 54, 255 } });

        // Header
        d.filledRect({ px + 1, py + 1 }, { pw - 2, 26 }, { .color = { 18, 23, 34, 255 } });
        d.line({ px, py + 27 }, { px + pw, py + 27 }, { .color = { 30, 38, 54, 255 } });
        d.text({ px + 10, py + 6 }, title, 12, { .color = { 75, 95, 125, 255 }, .font = &PoppinsLight_12 });

        // Diagonal grid pattern
        for (int i = 20; i < pw + ph; i += 40) {
            d.line({ px + std::max(0, i - ph), py + std::min(ph, i) }, { px + std::min(pw, i), py + std::max(0, i - pw) }, { .color = { 18, 24, 36, 255 } });
        }

        d.textCentered({ px + pw / 2, py + ph / 2 }, "UNASSIGNED TRACK PANEL", 12, { .color = { 45, 58, 80, 255 }, .font = &PoppinsLight_12 });
    }

    // Shaped Pitch Sweep Curve helper
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
        int volW = 100;
        volumeSliderRect = { px + pw - volW - 12, py + 5, volW, 16 };
        d.filledRect({ volumeSliderRect.x, volumeSliderRect.y }, { volumeSliderRect.w, volumeSliderRect.h }, { .color = { 28, 38, 56, 255 } });
        int fillW = (int)(volumeSliderRect.w * CLAMP(studio.track0.volume, 0.0f, 1.0f));
        d.filledRect({ volumeSliderRect.x, volumeSliderRect.y }, { fillW, volumeSliderRect.h }, { .color = { 0, 210, 160, 255 } });
        d.rect({ volumeSliderRect.x, volumeSliderRect.y }, { volumeSliderRect.w, volumeSliderRect.h }, { .color = { 70, 95, 130, 255 } });
        std::ostringstream volStr;
        volStr << "VOL " << (int)(studio.track0.volume * 100.0f) << "%";
        d.textCentered({ volumeSliderRect.x + volW / 2, volumeSliderRect.y + 2 }, volStr.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        int curY = py + 34;

        // --- RESPONSIVE EQUAL 3-COLUMN LAYOUT SCALING ---
        int contentW = pw - 20; // 10px margin on left & right
        int colGap = 10;
        int colW = (contentW - 2 * colGap) / 3; // Equal 1/3 panel width for each column!

        int col1X = px + 10;
        int col2X = col1X + colW + colGap;
        int col3X = col2X + colW + colGap;

        int seqY = py + ph - 142;
        int upperH = seqY - 11 - curY;    // Height for upper columns (~220-240px)
        int stackedH = (upperH - 4) / 2;  // Height for stacked 2D pads in Left & Right columns
        int vcoH = upperH - 40;           // Height for VCO MORPH box in Center column (leaves 40px for Duration & Pitch bars)

        // --- COLUMN 2 (CENTER): VCO MORPH ---
        vcoMorphRect = { col2X, curY, colW, vcoH };

        d.filledRect({ vcoMorphRect.x, vcoMorphRect.y }, { vcoMorphRect.w, vcoMorphRect.h }, { .color = { 10, 13, 20, 255 } });
        d.rect({ vcoMorphRect.x, vcoMorphRect.y }, { vcoMorphRect.w, vcoMorphRect.h }, { .color = { 0, 195, 255, 255 } });

        // --- TITLE TEXT HEADER: VCO MORPH ---
        d.textCentered({ vcoMorphRect.x + vcoMorphRect.w / 2, vcoMorphRect.y + 4 }, "VCO MORPH", 8, { .color = { 0, 230, 255, 255 }, .font = &PoppinsLight_8 });

        // --- MORPH PROGRESS BAR WITH FILLED SHAPE ICONS (CIRCLE -> TRIANGLE -> SAW -> RECTANGLE) ---
        int pBarX = vcoMorphRect.x + 10;
        int pBarY = vcoMorphRect.y + 16;
        int pBarW = vcoMorphRect.w - 20;
        int pBarH = 14;

        float morphVal = CLAMP(studio.track0.kick.vcoMorph.value / 100.0f, 0.0f, 1.0f);

        // Bar background & progress fill
        d.filledRect({ pBarX, pBarY }, { pBarW, pBarH }, { .color = { 18, 25, 38, 255 } });
        int pFillW = (int)(pBarW * morphVal);
        if (pFillW > 0) {
            d.filledRect({ pBarX, pBarY }, { pFillW, pBarH }, { .color = { 0, 175, 230, 255 } });
        }
        d.rect({ pBarX, pBarY }, { pBarW, pBarH }, { .color = { 0, 220, 255, 255 } });

        // Draw Filled Shape Icons along the Progress Bar
        int barCenterY = pBarY + pBarH / 2;

        // 1. Left Icon (0% Sine): Small Filled Circle
        int icon0X = pBarX + 7;
        d.filledCircle({ icon0X, barCenterY }, 3, { .color = { 255, 255, 255, 230 } });

        // 2. 33.3% Icon (Triangle): Small Filled Triangle
        int icon33X = pBarX + (int)(pBarW * 0.333f);
        d.filledPolygon({ { icon33X, barCenterY - 3 }, { icon33X + 3, barCenterY + 3 }, { icon33X - 3, barCenterY + 3 } }, { .color = { 255, 255, 255, 230 } });

        // 3. 66.6% Icon (Sawtooth): Small Filled Right Triangle
        int icon66X = pBarX + (int)(pBarW * 0.666f);
        d.filledPolygon({ { icon66X - 3, barCenterY + 3 }, { icon66X + 3, barCenterY - 3 }, { icon66X + 3, barCenterY + 3 } }, { .color = { 255, 255, 255, 230 } });

        // 4. Right Icon (100% Square): Small Filled Rectangle / Square
        int icon100X = pBarX + pBarW - 7;
        d.filledRect({ icon100X - 3, barCenterY - 3 }, { 6, 6 }, { .color = { 255, 255, 255, 230 } });

        // Current Position Handle Notch Indicator
        int handleX = std::clamp(pBarX + pFillW, pBarX + 1, pBarX + pBarW - 1);
        d.line({ handleX, pBarY - 1 }, { handleX, pBarY + pBarH + 1 }, { .color = { 255, 255, 100, 255 }, .thickness = 2 });

        // --- CENTER PIECE ANIMATION ---
        int cx = vcoMorphRect.x + vcoMorphRect.w / 2;
        int topPad = 34 + 16;
        int botPad = 18;
        int availW = vcoMorphRect.w - 32;
        int availH = vcoMorphRect.h - (topPad + botPad);
        int cy = vcoMorphRect.y + topPad + availH / 2;

        int maxOuterR = std::min(availW / 2, availH / 2);
        int halfSize = (int)((maxOuterR - 10) / 1.35f);
        halfSize = std::clamp(halfSize, 18, 65);
        float R = (float)halfSize;

        float clickAmt = studio.track0.kick.kickClickAmt.value;
        float durMs = studio.track0.kick.duration.value;
        float freqHz = studio.track0.kick.baseFreq.value;

        // Kick Trigger Pulse Decay & Expanding Shockwaves
        float decayRate = 12.0f / (CLAMP(durMs, 50.0f, 1500.0f) + 50.0f);
        kickPulseLevel = std::max(0.0f, kickPulseLevel - decayRate);

        if (kickPulseLevel > 0.01f) {
            int baseR = (int)(R * 0.8f);
            for (int r = 0; r < 3; r++) {
                float pFactor = kickPulseLevel - (r * 0.22f);
                if (pFactor > 0.0f) {
                    int radius = (int)(baseR + (1.0f - pFactor) * (R * 0.9f) + r * 8);
                    uint8_t alpha = (uint8_t)(pFactor * 140.0f);
                    d.circle({ cx, cy }, radius, { .color = { 0, 195, 255, alpha } });
                }
            }
        }

        // --- 3-ARC CURVATURE FLATTENING GEOMETRIC SINE -> TRIANGLE -> SAW -> 90° SQUARE MORPH ---
        Point triBL = { cx - halfSize, cy + halfSize };
        Point triBR = { cx + halfSize, cy + halfSize };
        Point triTop = { cx, cy - halfSize };

        Point sqBL = { cx - halfSize, cy + halfSize };
        Point sqBR = { cx + halfSize, cy + halfSize };
        Point sqTR = { cx + halfSize, cy - halfSize };
        Point sqTL = { cx - halfSize, cy - halfSize };

        std::vector<Point> morphShape;

        if (morphVal <= 0.333f) {
            // Stage 1: Circle split into 3 arcs; each 120° arc flattens into a triangle side
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
            // Stage 2: Equilateral Triangle -> Sawtooth / Right Triangle
            float t = (morphVal - 0.333f) / 0.333f;
            int curTopX = (int)((1.0f - t) * triTop.x + t * sqTR.x);
            int curTopY = (int)((1.0f - t) * triTop.y + t * sqTR.y);
            Point curTop = { curTopX, curTopY };

            morphShape = { triBL, curTop, triBR };
        } else {
            // Stage 3: Sawtooth / Right Triangle -> Perfect 90° Square / Rectangle
            float u = (morphVal - 0.666f) / 0.334f;
            int curTLX = (int)((1.0f - u) * sqTR.x + u * sqTL.x);
            Point curTL = { curTLX, sqTL.y };

            morphShape = { sqBL, curTL, sqTR, sqBR };
        }

        // FM Modulator Orbiting Shell
        float fmVal = CLAMP(studio.track0.kick.fmDepth.value / 100.0f, 0.0f, 1.0f);
        if (fmVal > 0.01f) {
            float rotAngle = animTime * (1.0f + fmVal * 8.0f);
            int numShellPts = 5;
            std::vector<Point> modShell;
            for (int i = 0; i < numShellPts; i++) {
                float a = rotAngle + i * (6.28318f / numShellPts);
                float radiusW = (R + 14.0f) + std::sin(a * 3.0f + animTime * 4.0f) * (fmVal * (R * 0.35f));
                float radiusH = (R + 14.0f) + std::cos(a * 2.0f + animTime * 3.0f) * (fmVal * (R * 0.3f));
                int mx = cx + (int)(std::cos(a) * radiusW);
                int my = cy + (int)(std::sin(a) * radiusH);
                modShell.push_back({ mx, my });
            }
            uint8_t shellAlpha = (uint8_t)(80 + fmVal * 165.0f);
            d.lines(modShell, { .color = { 0, 195, 255, shellAlpha }, .thickness = 1 });
            d.line(modShell.back(), modShell.front(), { .color = { 0, 195, 255, shellAlpha }, .thickness = 1 });
        }

        // Drive Overdrive Saturation Stroke & Fill
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

        // // Frequency Sine Ribbon Wave across bottom of VCO morph box
        // int freqY = vcoMorphRect.y + vcoMorphRect.h - 12;
        // std::vector<Point> freqWave;
        // int innerW = vcoMorphRect.w - 16;
        // for (int gx = 0; gx < innerW; gx += 2) {
        //     float t = (float)gx / (float)innerW;
        //     float wave = std::sin(t * (freqHz * 0.22f) + animTime * (freqHz * 0.07f)) * (4.0f + (freqHz * 0.015f));
        //     freqWave.push_back({ vcoMorphRect.x + 8 + gx, freqY + (int)wave });
        // }
        // if (freqWave.size() > 1) {
        //     d.lines(freqWave, { .color = { 0, 195, 255, 200 } });
        // }

        // --- COLUMN 1 (LEFT): SWEEP PITCH XY PAD ---
        sweepCurveRect = { col1X, curY, colW, stackedH };
        d.filledRect({ sweepCurveRect.x, sweepCurveRect.y }, { sweepCurveRect.w, sweepCurveRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ sweepCurveRect.x, sweepCurveRect.y }, { sweepCurveRect.w, sweepCurveRect.h }, { .color = { 255, 160, 40, 255 } });
        d.text({ sweepCurveRect.x + 6, sweepCurveRect.y + 4 }, "SWEEP", 8, { .color = { 255, 180, 50, 255 }, .font = &PoppinsLight_8 });

        // Grid lines inside XY Pad
        d.line({ sweepCurveRect.x + sweepCurveRect.w / 2, sweepCurveRect.y + 14 }, { sweepCurveRect.x + sweepCurveRect.w / 2, sweepCurveRect.y + sweepCurveRect.h - 4 }, { .color = { 45, 36, 28, 255 } });
        d.line({ sweepCurveRect.x + 4, sweepCurveRect.y + 14 + (sweepCurveRect.h - 18) / 2 }, { sweepCurveRect.x + sweepCurveRect.w - 4, sweepCurveRect.y + 14 + (sweepCurveRect.h - 18) / 2 }, { .color = { 45, 36, 28, 255 } });

        // Render Pitch Sweep Curve taking both Shape (X) and Depth (Y) into account
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

        // Calculate Draggable Handle Position: X = Shape (0..100%), Y = Depth (0..100%)
        int swpTargetX = sweepCurveRect.x + 6 + (int)(shpNorm * (sweepCurveRect.w - 12));
        int swpTargetY = sweepCurveRect.y + sweepCurveRect.h - 6 - (int)(depthNorm * (sweepCurveRect.h - 20));

        d.line({ swpTargetX - 6, swpTargetY }, { swpTargetX + 6, swpTargetY }, { .color = { 255, 180, 50, 255 } });
        d.line({ swpTargetX, swpTargetY - 6 }, { swpTargetX, swpTargetY + 6 }, { .color = { 255, 180, 50, 255 } });
        d.filledCircle({ swpTargetX, swpTargetY }, 4, { .color = { 255, 220, 90, 255 } });
        d.circle({ swpTargetX, swpTargetY }, 6, { .color = { 255, 255, 255, 255 } });

        // --- SWEEP PULSE & VISUAL FEEDBACK WHEN KICK PLAYS ---
        if (kickPulseLevel > 0.01f) {
            uint8_t pulseAlpha = (uint8_t)(kickPulseLevel * 220.0f);
            d.lines(curvePts, { .color = { 255, 230, 100, pulseAlpha }, .thickness = 3 });

            // Traveling Pitch Drop Ball moving along curve from t=0 to t=1
            float progress = CLAMP(1.0f - kickPulseLevel, 0.0f, 1.0f);
            int ballIdx = (int)(progress * steps);
            ballIdx = std::clamp(ballIdx, 0, (int)curvePts.size() - 1);
            Point ballPt = curvePts[ballIdx];

            d.filledCircle(ballPt, 5, { .color = { 255, 255, 200, 255 } });
            d.circle(ballPt, 7, { .color = { 255, 180, 50, pulseAlpha } });

            // Expanding Shockwave rings around handle
            for (int r = 0; r < 2; r++) {
                float pFactor = kickPulseLevel - (r * 0.3f);
                if (pFactor > 0.0f) {
                    int radius = (int)(6.0f + (1.0f - pFactor) * 18.0f);
                    uint8_t rAlpha = (uint8_t)(pFactor * 180.0f);
                    d.circle({ swpTargetX, swpTargetY }, radius, { .color = { 255, 180, 50, rAlpha } });
                }
            }
        }

        // --- COLUMN 1 (LEFT BOTTOM): CLICK RADAR XY TARGET PAD ---
        clickXyRect = { col1X, curY + stackedH + 4, colW, stackedH };
        d.filledRect({ clickXyRect.x, clickXyRect.y }, { clickXyRect.w, clickXyRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ clickXyRect.x, clickXyRect.y }, { clickXyRect.w, clickXyRect.h }, { .color = { 255, 80, 120, 255 } });
        d.text({ clickXyRect.x + 6, clickXyRect.y + 4 }, "CLICK", 8, { .color = { 255, 100, 140, 255 }, .font = &PoppinsLight_8 });

        // Grid lines inside XY Pad
        d.line({ clickXyRect.x + clickXyRect.w / 2, clickXyRect.y + 14 }, { clickXyRect.x + clickXyRect.w / 2, clickXyRect.y + clickXyRect.h - 4 }, { .color = { 40, 30, 48, 255 } });
        d.line({ clickXyRect.x + 4, clickXyRect.y + 14 + (clickXyRect.h - 18) / 2 }, { clickXyRect.x + clickXyRect.w - 4, clickXyRect.y + 14 + (clickXyRect.h - 18) / 2 }, { .color = { 40, 30, 48, 255 } });

        // Calculate handle position: X = Amt (0..100%), Y = Decay (1..100ms)
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

        // --- CLICK PULSE & FANCY NOISE PARTICLE EXPLOSION WHEN KICK PLAYS ---
        if (kickPulseLevel > 0.01f) {
            uint8_t pulseAlpha = (uint8_t)(kickPulseLevel * 220.0f);

            // 1. Flashing border glow
            d.rect({ clickXyRect.x, clickXyRect.y }, { clickXyRect.w, clickXyRect.h }, { .color = { 255, 150, 180, pulseAlpha } });

            // 2. Radar Pulse Rings expanding from target handle
            for (int r = 0; r < 3; r++) {
                float pFactor = kickPulseLevel - (r * 0.22f);
                if (pFactor > 0.0f) {
                    int radius = (int)(4.0f + (1.0f - pFactor) * 24.0f + r * 5);
                    uint8_t rAlpha = (uint8_t)(pFactor * 190.0f);
                    d.circle({ targetX, targetY }, radius, { .color = { 255, 100, 160, rAlpha } });
                }
            }

            // 3. FANCY NOISE POINT SWARM / PARTICLE EXPLOSION bursting outward from (targetX, targetY)
            int particleCount = (int)(24 + amtNorm * 24.0f);
            float burstExp = 1.0f - kickPulseLevel;

            for (int i = 0; i < particleCount; i++) {
                float angle = i * 0.2618f + (i * 1.37f);
                float speed = 12.0f + (float)((i * 17) % 35);
                float dist = burstExp * speed;

                int px = targetX + (int)(std::cos(angle) * dist);
                int py = targetY + (int)(std::sin(angle) * dist);

                px = std::clamp(px, clickXyRect.x + 4, clickXyRect.x + clickXyRect.w - 4);
                py = std::clamp(py, clickXyRect.y + 14, clickXyRect.y + clickXyRect.h - 4);

                uint8_t pAlpha = (uint8_t)(kickPulseLevel * (140 + (i * 19) % 115));
                Color pColor = (i % 2 == 0) ? Color { 255, 240, 245, pAlpha } : Color { 255, 140, 190, pAlpha };

                d.pixel({ px, py }, pColor);
                if (i % 3 == 0) {
                    d.pixel({ px + 1, py }, pColor);
                }
            }

            // 4. Bright center target flash
            d.filledCircle({ targetX, targetY }, 5, { .color = { 255, 235, 245, (uint8_t)(kickPulseLevel * 255.0f) } });
            d.circle({ targetX, targetY }, 7, { .color = { 255, 120, 170, pulseAlpha } });
        }

        // --- COLUMN 3 (RIGHT TOP): FM SYNTHESIS 2D XY PAD & SEGMENTED RATIO BAR ---
        fmXyRect = { col3X, curY, colW, stackedH };

        d.filledRect({ fmXyRect.x, fmXyRect.y }, { fmXyRect.w, fmXyRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ fmXyRect.x, fmXyRect.y }, { fmXyRect.w, fmXyRect.h }, { .color = { 180, 100, 255, 255 } });
        d.text({ fmXyRect.x + 6, fmXyRect.y + 4 }, "FM SYNTHESIS", 8, { .color = { 200, 130, 255, 255 }, .font = &PoppinsLight_8 });

        // Grid lines inside FM Pad
        int padBodyH = fmXyRect.h - 18;
        d.line({ fmXyRect.x + fmXyRect.w / 2, fmXyRect.y + 14 }, { fmXyRect.x + fmXyRect.w / 2, fmXyRect.y + padBodyH - 4 }, { .color = { 38, 28, 52, 255 } });
        d.line({ fmXyRect.x + 4, fmXyRect.y + 14 + (padBodyH - 18) / 2 }, { fmXyRect.x + fmXyRect.w - 4, fmXyRect.y + 14 + (padBodyH - 18) / 2 }, { .color = { 38, 28, 52, 255 } });

        // Calculate handle position: X = FM Depth (0..100%), Y = FM Decay/Snap (2..150ms)
        float fmDepthNorm = studio.track0.kick.fmDepth.value * 0.01f;
        float fmSnapNorm = (studio.track0.kick.fmSnap.value - 2.0f) / 148.0f;
        int fmTargetX = fmXyRect.x + 6 + (int)(fmDepthNorm * (fmXyRect.w - 12));
        int fmTargetY = fmXyRect.y + padBodyH - 6 - (int)(fmSnapNorm * (padBodyH - 20));

        d.line({ fmTargetX - 6, fmTargetY }, { fmTargetX + 6, fmTargetY }, { .color = { 200, 120, 255, 255 } });
        d.line({ fmTargetX, fmTargetY - 6 }, { fmTargetX, fmTargetY + 6 }, { .color = { 200, 120, 255, 255 } });
        d.filledCircle({ fmTargetX, fmTargetY }, 4, { .color = { 230, 170, 255, 255 } });
        d.circle({ fmTargetX, fmTargetY }, 6, { .color = { 255, 255, 255, 255 } });

        std::ostringstream fmTxt;
        fmTxt << (int)studio.track0.kick.fmDepth.value << "%/" << (int)studio.track0.kick.fmSnap.value << "ms";
        d.textRight({ fmXyRect.x + fmXyRect.w - 6, fmXyRect.y + 4 }, fmTxt.str(), 8, { .color = { 210, 150, 255, 255 }, .font = &PoppinsLight_8 });

        // FM Visual Pulse when kick plays
        if (kickPulseLevel > 0.01f) {
            for (int r = 0; r < 3; r++) {
                float pFactor = kickPulseLevel - (r * 0.22f);
                if (pFactor > 0.0f) {
                    int radius = (int)(4.0f + (1.0f - pFactor) * 22.0f + r * 5);
                    uint8_t rAlpha = (uint8_t)(pFactor * 190.0f);
                    d.circle({ fmTargetX, fmTargetY }, radius, { .color = { 190, 110, 255, rAlpha } });
                }
            }
            d.filledCircle({ fmTargetX, fmTargetY }, 5, { .color = { 245, 220, 255, (uint8_t)(kickPulseLevel * 255.0f) } });
        }

        // --- FM RATIO SEGMENTED BAR AT BOTTOM OF FM PAD ---
        int barX = fmXyRect.x;
        int barY = fmXyRect.y + padBodyH;
        int barW = fmXyRect.w;
        int barH = 18;
        fmRatioBarRect = { barX, barY, barW, barH };

        d.filledRect({ barX, barY }, { barW, barH }, { .color = { 12, 16, 26, 255 } });
        d.line({ barX, barY }, { barX + barW, barY }, { .color = { 180, 100, 255, 255 } });

        // Segmented Bar Calculation (31 segments from 0.5x to 8.0x in steps of 0.25x)
        const int NUM_SEGMENTS = 31;
        float curRatio = studio.track0.kick.fmRatio.value;
        int activeSegmentIdx = (int)std::round((curRatio - 0.5f) / 0.25f);
        activeSegmentIdx = std::clamp(activeSegmentIdx, 0, NUM_SEGMENTS - 1);

        float segWidth = (float)(barW - 4) / (float)NUM_SEGMENTS;

        for (int seg = 0; seg < NUM_SEGMENTS; seg++) {
            int sx = barX + 2 + (int)(seg * segWidth);
            int sw = std::max(1, (int)segWidth - 1);
            int sy = barY + 3;
            int sh = barH - 6;

            Color segCol;
            if (seg <= activeSegmentIdx) {
                float t = (float)seg / (float)NUM_SEGMENTS;
                segCol = Color {
                    (uint8_t)(180 * (1.0f - t) + 0 * t),
                    (uint8_t)(100 * (1.0f - t) + 230 * t),
                    (uint8_t)(255 * (1.0f - t) + 255 * t),
                    255
                };
            } else {
                segCol = Color { 28, 36, 52, 255 };
            }

            d.filledRect({ sx, sy }, { sw, sh }, { .color = segCol });
        }

        std::ostringstream ratioTxt;
        ratioTxt << "RATIO " << std::fixed << std::setprecision(2) << curRatio << "x";
        d.textCentered({ barX + barW / 2, barY + 4 }, ratioTxt.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        // --- COLUMN 3 (RIGHT BOTTOM): DRIVE & BASS BOOST 2D XY PAD & SEGMENTED WAVEFOLD BAR ---
        driveXyRect = { col3X, curY + stackedH + 4, colW, stackedH };

        d.filledRect({ driveXyRect.x, driveXyRect.y }, { driveXyRect.w, driveXyRect.h }, { .color = { 12, 14, 20, 255 } });
        d.rect({ driveXyRect.x, driveXyRect.y }, { driveXyRect.w, driveXyRect.h }, { .color = { 255, 100, 40, 255 } });
        d.text({ driveXyRect.x + 6, driveXyRect.y + 4 }, "DRIVE & BASS", 8, { .color = { 255, 130, 60, 255 }, .font = &PoppinsLight_8 });

        int drvBodyH = driveXyRect.h - 18;
        d.line({ driveXyRect.x + driveXyRect.w / 2, driveXyRect.y + 14 }, { driveXyRect.x + driveXyRect.w / 2, driveXyRect.y + drvBodyH - 4 }, { .color = { 50, 32, 28, 255 } });
        d.line({ driveXyRect.x + 4, driveXyRect.y + 14 + (drvBodyH - 18) / 2 }, { driveXyRect.x + driveXyRect.w - 4, driveXyRect.y + 14 + (drvBodyH - 18) / 2 }, { .color = { 50, 32, 28, 255 } });

        // Calculate handle position: X = Drive (0..100%), Y = Bass Boost (0..100%)
        float drvNorm = studio.track0.kick.drive.value * 0.01f;
        float boostNorm = studio.track0.kick.bassBoost.value * 0.01f;
        int driveTargetX = driveXyRect.x + 6 + (int)(drvNorm * (driveXyRect.w - 12));
        int driveTargetY = driveXyRect.y + drvBodyH - 6 - (int)(boostNorm * (drvBodyH - 20));

        d.line({ driveTargetX - 6, driveTargetY }, { driveTargetX + 6, driveTargetY }, { .color = { 255, 120, 50, 255 } });
        d.line({ driveTargetX, driveTargetY - 6 }, { driveTargetX, driveTargetY + 6 }, { .color = { 255, 120, 50, 255 } });
        d.filledCircle({ driveTargetX, driveTargetY }, 4, { .color = { 255, 200, 90, 255 } });
        d.circle({ driveTargetX, driveTargetY }, 6, { .color = { 255, 255, 255, 255 } });

        std::ostringstream drvTxt;
        drvTxt << (int)studio.track0.kick.drive.value << "%/" << (int)studio.track0.kick.bassBoost.value << "%";
        d.textRight({ driveXyRect.x + driveXyRect.w - 6, driveXyRect.y + 4 }, drvTxt.str(), 8, { .color = { 255, 170, 90, 255 }, .font = &PoppinsLight_8 });

        // Drive Visual Pulse when kick plays
        if (kickPulseLevel > 0.01f) {
            for (int r = 0; r < 3; r++) {
                float pFactor = kickPulseLevel - (r * 0.22f);
                if (pFactor > 0.0f) {
                    int radius = (int)(4.0f + (1.0f - pFactor) * 22.0f + r * 5);
                    uint8_t rAlpha = (uint8_t)(pFactor * 190.0f);
                    d.circle({ driveTargetX, driveTargetY }, radius, { .color = { 255, 100, 40, rAlpha } });
                }
            }
            d.filledCircle({ driveTargetX, driveTargetY }, 5, { .color = { 255, 240, 220, (uint8_t)(kickPulseLevel * 255.0f) } });
        }

        // --- SEGMENTED WAVEFOLD BAR AT BOTTOM OF DRIVE PAD ---
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
            int sy = fBarY + 3;
            int sh = fBarH - 6;

            Color segCol;
            if (seg <= activeFoldIdx) {
                float t = (float)seg / (float)FOLD_SEGMENTS;
                segCol = Color {
                    255,
                    (uint8_t)(60 * (1.0f - t) + 180 * t),
                    (uint8_t)(40 * (1.0f - t) + 20 * t),
                    255
                };
            } else {
                segCol = Color { 48, 28, 30, 255 };
            }

            d.filledRect({ sx, sy }, { sw, sh }, { .color = segCol });
        }

        std::ostringstream foldTxt;
        foldTxt << "FOLD " << (int)curFold << "%";
        d.textCentered({ fBarX + fBarW / 2, fBarY + 4 }, foldTxt.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        // --- PERFORMANCE BARS INSIDE COLUMN 2 (UNDERNEATH VCO MORPH): DURATION & BIPOLAR CENTERED PITCH BAR ---
        int durY = curY + vcoH + 4;
        int pitchY = durY + 16 + 4;

        // 1. DURATION BAR (50ms to 1500ms)
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

        // 2. BIPOLAR CENTERED PITCH BAR (12 Segments Left | Center Divider | 12 Segments Right)
        semitoneBarRect = { col2X, pitchY, colW, 16 };
        d.filledRect({ semitoneBarRect.x, semitoneBarRect.y }, { semitoneBarRect.w, semitoneBarRect.h }, { .color = { 14, 18, 28, 255 } });
        d.rect({ semitoneBarRect.x, semitoneBarRect.y }, { semitoneBarRect.w, semitoneBarRect.h }, { .color = { 255, 180, 40, 255 } });

        int curSemi = studio.track0.kick.semitoneOffset.load();
        curSemi = std::clamp(curSemi, -12, 12);

        int sBarX = semitoneBarRect.x + 2;
        int sBarW = semitoneBarRect.w - 4;
        int sBarH = semitoneBarRect.h - 4;
        int sBarY = semitoneBarRect.y + 2;

        int centerW = 4;
        int sideW = (sBarW - centerW) / 2;
        float segW = (float)sideW / 12.0f;

        // Left 12 Segments (-12 to -1)
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

        // Center Divider (0 st)
        int centerX = sBarX + sideW;
        Color centerCol = { 150, 155, 170, 255 };
        d.filledRect({ centerX, sBarY }, { centerW, sBarH }, { .color = centerCol });

        // Right 12 Segments (+1 to +12)
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

        // High-Contrast Text Display
        std::ostringstream semiTxt;
        semiTxt << "PITCH " << (curSemi > 0 ? "+" : "") << curSemi << " st";
        d.textCentered({ semitoneBarRect.x + semitoneBarRect.w / 2 + 1, semitoneBarRect.y + 4 }, semiTxt.str(), 8, { .color = { 0, 0, 0, 255 }, .font = &PoppinsLight_8 });
        d.textCentered({ semitoneBarRect.x + semitoneBarRect.w / 2, semitoneBarRect.y + 3 }, semiTxt.str(), 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        // --- 64-STEP SEQUENCER FOR TRACK 0 (4 ROWS x 16 STEPS) ---
        int seqW = pw - 20;

        d.text({ px + 10, seqY - 11 }, "64-STEP SEQUENCER (4 ROWS x 16 STEPS | G=GENERATE, [X]=ROW ENABLE)", 8, { .color = { 140, 165, 200, 255 }, .font = &PoppinsLight_8 });

        int leftCtrlW = 38; // 14px Checkbox + 3px gap + 16px G button + 5px margin
        int gridX = px + 10 + leftCtrlW;
        int gridW = seqW - leftCtrlW;
        int stepBoxW = (gridW - (16 - 1) * 2) / 16;
        int stepBoxH = 22;

        // Top Column Enable Buttons (Columns 1 to 16)
        int colBtnY = seqY + 2;
        for (int c = 0; c < 16; c++) {
            int cx = gridX + c * (stepBoxW + 2);
            colEnableRects[c] = { cx, colBtnY, stepBoxW, 11 };

            bool allColActive = true;
            for (int r = 0; r < 4; r++) {
                if (!studio.track0.sequence[r * 16 + c].active) {
                    allColActive = false;
                    break;
                }
            }

            Color colBg = allColActive ? Color { 0, 180, 220, 255 } : Color { 22, 28, 42, 255 };
            Color colBdr = allColActive ? Color { 0, 240, 255, 255 } : Color { 50, 65, 90, 255 };
            d.filledRect({ cx, colBtnY }, { stepBoxW, 11 }, { .color = colBg });
            d.rect({ cx, colBtnY }, { stepBoxW, 11 }, { .color = colBdr });

            std::ostringstream cStr;
            cStr << (c + 1);
            d.textCentered({ cx + stepBoxW / 2, colBtnY + 2 }, cStr.str(), 8, { .color = allColActive ? Color { 255, 255, 255, 255 } : Color { 100, 125, 160, 255 }, .font = &PoppinsLight_8 });
        }

        // 4 Rows of 16 Steps
        int rowStartY = colBtnY + 14;
        int playHead = studio.currentStep.load();

        for (int r = 0; r < 4; r++) {
            int ry = rowStartY + r * (stepBoxH + 3);
            bool isRowOn = studio.track0.rowEnabled[r];

            // 1. Left Checkbox [X]
            rowCheckRects[r] = { px + 10, ry + (stepBoxH - 14) / 2, 14, 14 };
            Color cbBg = isRowOn ? Color { 0, 160, 120, 255 } : Color { 24, 28, 36, 255 };
            Color cbBdr = isRowOn ? Color { 0, 230, 160, 255 } : Color { 60, 70, 90, 255 };
            d.filledRect({ rowCheckRects[r].x, rowCheckRects[r].y }, { 14, 14 }, { .color = cbBg });
            d.rect({ rowCheckRects[r].x, rowCheckRects[r].y }, { 14, 14 }, { .color = cbBdr });
            d.textCentered({ rowCheckRects[r].x + 7, rowCheckRects[r].y + 3 }, isRowOn ? "X" : "-", 8, { .color = isRowOn ? Color { 255, 255, 255, 255 } : Color { 100, 110, 130, 255 }, .font = &PoppinsLight_8 });

            // 2. Left "G" Generate Button
            rowGenRects[r] = { px + 10 + 17, ry + (stepBoxH - 14) / 2, 16, 14 };
            d.filledRect({ rowGenRects[r].x, rowGenRects[r].y }, { 16, 14 }, { .color = Color { 36, 46, 64, 255 } });
            d.rect({ rowGenRects[r].x, rowGenRects[r].y }, { 16, 14 }, { .color = Color { 255, 180, 40, 255 } });
            d.textCentered({ rowGenRects[r].x + 8, rowGenRects[r].y + 3 }, "G", 8, { .color = Color { 255, 210, 60, 255 }, .font = &PoppinsLight_8 });

            // 3. 16 Step Boxes for Row r
            for (int c = 0; c < 16; c++) {
                int i = r * 16 + c;
                int sx = gridX + c * (stepBoxW + 2);
                stepRects[i] = { sx, ry, stepBoxW, stepBoxH };

                auto& stp = studio.track0.sequence[i];
                bool isCurrent = (playHead == i && studio.isPlaying);

                Color stepBg;
                if (!isRowOn) {
                    stepBg = stp.active ? Color { 0, 70, 90, 180 } : Color { 14, 18, 26, 255 };
                } else if (stp.active) {
                    stepBg = (c % 4 == 0) ? Color { 0, 190, 230, 255 } : Color { 0, 150, 190, 255 };
                } else {
                    stepBg = (c % 4 == 0) ? Color { 26, 34, 50, 255 } : Color { 18, 24, 36, 255 };
                }

                d.filledRect({ sx, ry }, { stepBoxW, stepBoxH }, { .color = stepBg });

                Color borderCol = isCurrent ? Color { 255, 240, 0, 255 } : (isRowOn ? Color { 45, 60, 85, 255 } : Color { 28, 36, 48, 255 });
                d.rect({ sx, ry }, { stepBoxW, stepBoxH }, { .color = borderCol });
                if (isCurrent) {
                    d.rect({ sx + 1, ry + 1 }, { stepBoxW - 2, stepBoxH - 2 }, { .color = { 255, 255, 100, 255 } });
                }

                // Step text: note name or step number within row
                if (stp.active) {
                    std::string noteStr = (stp.note >= 0 && stp.note < 132) ? MIDI_NOTES_STR[stp.note] : "C4";
                    d.textCentered({ sx + stepBoxW / 2, ry + 6 }, noteStr, 8, { .color = isRowOn ? Color { 255, 255, 255, 255 } : Color { 140, 160, 180, 255 }, .font = &PoppinsLight_8 });
                } else {
                    std::ostringstream numStr;
                    numStr << (c + 1);
                    d.textCentered({ sx + stepBoxW / 2, ry + 6 }, numStr.str(), 8, { .color = isRowOn ? Color { 80, 100, 130, 255 } : Color { 45, 55, 70, 255 }, .font = &PoppinsLight_8 });
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
        int panelW = W / 2;
        int panelH = gridH / 2;

        // Top-Left (Track 0: ImpactKick)
        drawTrack0Panel(d, 0, headH, panelW, panelH);

        // Top-Right (Track 1: Empty)
        drawEmptyPanel(d, panelW, headH, panelW, panelH, "TRACK 2");

        // Bottom-Left (Track 2: Empty)
        drawEmptyPanel(d, 0, headH + panelH, panelW, panelH, "TRACK 3");

        // Bottom-Right (Track 3: Empty)
        drawEmptyPanel(d, panelW, headH + panelH, panelW, panelH, "TRACK 4");
    }

    // Interactive Input Handlers
    void onMouseDown(int mx, int my)
    {
        std::lock_guard<std::mutex> lock(studio.audioMutex);

        if (durationBarRect.contains(mx, my)) {
            activeDrag = DRAG_DURATION_BAR;
            float norm = CLAMP((float)(mx - durationBarRect.x) / (float)durationBarRect.w, 0.0f, 1.0f);
            Param& p = studio.track0.kick.duration;
            p.value = p.min + norm * (p.max - p.min);

            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        if (semitoneBarRect.contains(mx, my)) {
            activeDrag = DRAG_SEMITONE_BAR;
            int pBarX = semitoneBarRect.x + 2;
            int pBarW = semitoneBarRect.w - 4;
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

        if (playBtnRect.contains(mx, my)) {
            studio.isPlaying = !studio.isPlaying;
            return;
        }

        if (bpmRect.contains(mx, my)) {
            activeDrag = DRAG_BPM;
            dragStartY = my;
            return;
        }

        if (volumeSliderRect.contains(mx, my)) {
            activeDrag = DRAG_VOLUME;
            float norm = CLAMP((float)(mx - volumeSliderRect.x) / (float)volumeSliderRect.w, 0.0f, 1.0f);
            studio.track0.volume = norm;
            return;
        }

        if (sweepCurveRect.contains(mx, my)) {
            activeDrag = DRAG_SWEEP_XY;
            float shpNorm = CLAMP((float)(mx - (sweepCurveRect.x + 6)) / (float)(sweepCurveRect.w - 12), 0.0f, 1.0f);
            float depthNorm = CLAMP((float)(sweepCurveRect.y + sweepCurveRect.h - 6 - my) / (float)(sweepCurveRect.h - 20), 0.0f, 1.0f);

            studio.track0.kick.sweepShp.value = shpNorm * 100.0f;
            studio.track0.kick.sweepDepth.value = depthNorm * 100.0f;

            // Manual trigger & visual feedback pulse
            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        if (vcoMorphRect.contains(mx, my)) {
            BoxRect pBarRect = { vcoMorphRect.x + 10, vcoMorphRect.y + 16, vcoMorphRect.w - 20, 14 };
            if (pBarRect.contains(mx, my)) {
                activeDrag = DRAG_VCO_MORPH_BAR;
                float norm = CLAMP((float)(mx - pBarRect.x) / (float)pBarRect.w, 0.0f, 1.0f);
                studio.track0.kick.vcoMorph.value = norm * 100.0f;
            } else {
                activeDrag = DRAG_VCO_MORPH_BODY;
                dragStartX = mx;
                dragStartValX = studio.track0.kick.vcoMorph.value;
            }

            // Manual trigger & visual feedback pulse
            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        if (foldBarRect.contains(mx, my)) {
            activeDrag = DRAG_FOLD_BAR;
            float norm = CLAMP((float)(mx - (foldBarRect.x + 2)) / (float)(foldBarRect.w - 4), 0.0f, 1.0f);
            int seg = (int)std::round(norm * 20.0f);
            studio.track0.kick.fold.value = seg * 5.0f;

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

        if (fmRatioBarRect.contains(mx, my)) {
            activeDrag = DRAG_FM_RATIO_BAR;
            float norm = CLAMP((float)(mx - (fmRatioBarRect.x + 2)) / (float)(fmRatioBarRect.w - 4), 0.0f, 1.0f);
            int seg = (int)std::round(norm * 30.0f);
            studio.track0.kick.fmRatio.value = 0.5f + seg * 0.25f;

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

        if (clickXyRect.contains(mx, my)) {
            activeDrag = DRAG_CLICK_XY;
            float amtNorm = CLAMP((float)(mx - (clickXyRect.x + 6)) / (float)(clickXyRect.w - 12), 0.0f, 1.0f);
            float decNorm = CLAMP((float)(clickXyRect.y + clickXyRect.h - 6 - my) / (float)(clickXyRect.h - 20), 0.0f, 1.0f);

            studio.track0.kick.kickClickAmt.value = amtNorm * 100.0f;
            studio.track0.kick.kickClickDecay.value = 1.0f + decNorm * 99.0f;

            // Manual trigger & visual feedback pulse
            studio.track0.kick.noteOn(60, 0.9f);
            studio.kickPulseTrigger.store(true);
            return;
        }

        for (size_t i = 0; i < paramSliders.size(); i++) {
            if (paramSliders[i].rect.contains(mx, my)) {
                activeDrag = DRAG_PARAM_SLIDER;
                dragParamIdx = (int)i;
                Param* p = paramSliders[i].param;
                float norm = CLAMP((float)(mx - paramSliders[i].rect.x) / (float)paramSliders[i].rect.w, 0.0f, 1.0f);
                p->value = p->min + norm * (p->max - p->min);
                return;
            }
        }

        for (int r = 0; r < 4; r++) {
            if (rowCheckRects[r].contains(mx, my)) {
                studio.track0.rowEnabled[r] = !studio.track0.rowEnabled[r];
                return;
            }
            if (rowGenRects[r].contains(mx, my)) {
                generateRowPattern(r);
                studio.track0.kick.noteOn(60, 0.9f);
                studio.kickPulseTrigger.store(true);
                return;
            }
        }

        for (int c = 0; c < 16; c++) {
            if (colEnableRects[c].contains(mx, my)) {
                bool allActive = true;
                for (int r = 0; r < 4; r++) {
                    if (!studio.track0.sequence[r * 16 + c].active) {
                        allActive = false;
                        break;
                    }
                }
                bool newState = !allActive;
                for (int r = 0; r < 4; r++) {
                    studio.track0.sequence[r * 16 + c].active = newState;
                }
                if (newState) {
                    studio.track0.kick.noteOn(60, 0.9f);
                    studio.kickPulseTrigger.store(true);
                }
                return;
            }
        }

        for (int i = 0; i < SEQ_STEPS_TEK; i++) {
            if (stepRects[i].contains(mx, my)) {
                studio.track0.sequence[i].active = !studio.track0.sequence[i].active;
                activeDrag = DRAG_STEP_NOTE;
                dragStepIdx = i;
                dragStartY = my;
                dragStartNote = studio.track0.sequence[i].note;

                if (studio.track0.sequence[i].active) {
                    studio.track0.kick.noteOn(studio.track0.sequence[i].note, studio.track0.sequence[i].velocity);
                    studio.kickPulseTrigger.store(true);
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
        } else if (activeDrag == DRAG_VOLUME) {
            float norm = CLAMP((float)(mx - volumeSliderRect.x) / (float)volumeSliderRect.w, 0.0f, 1.0f);
            studio.track0.volume = norm;
        } else if (activeDrag == DRAG_SWEEP_XY) {
            float shpNorm = CLAMP((float)(mx - (sweepCurveRect.x + 6)) / (float)(sweepCurveRect.w - 12), 0.0f, 1.0f);
            float depthNorm = CLAMP((float)(sweepCurveRect.y + sweepCurveRect.h - 6 - my) / (float)(sweepCurveRect.h - 20), 0.0f, 1.0f);
            studio.track0.kick.sweepShp.value = shpNorm * 100.0f;
            studio.track0.kick.sweepDepth.value = depthNorm * 100.0f;
        } else if (activeDrag == DRAG_VCO_MORPH_BAR) {
            float norm = CLAMP((float)(mx - (vcoMorphRect.x + 10)) / (float)(vcoMorphRect.w - 20), 0.0f, 1.0f);
            studio.track0.kick.vcoMorph.value = norm * 100.0f;
        } else if (activeDrag == DRAG_VCO_MORPH_BODY) {
            int dx = mx - dragStartX;
            float deltaVal = (float)dx * 0.6f;
            float newMorph = CLAMP(dragStartValX + deltaVal, 0.0f, 100.0f);
            studio.track0.kick.vcoMorph.value = newMorph;
        } else if (activeDrag == DRAG_DURATION_BAR) {
            float norm = CLAMP((float)(mx - durationBarRect.x) / (float)durationBarRect.w, 0.0f, 1.0f);
            Param& p = studio.track0.kick.duration;
            p.value = p.min + norm * (p.max - p.min);
        } else if (activeDrag == DRAG_SEMITONE_BAR) {
            int pBarX = semitoneBarRect.x + 2;
            int pBarW = semitoneBarRect.w - 4;
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
        } else if (activeDrag == DRAG_DRIVE_XY) {
            int padBodyH = driveXyRect.h - 18;
            float drvNorm = CLAMP((float)(mx - (driveXyRect.x + 6)) / (float)(driveXyRect.w - 12), 0.0f, 1.0f);
            float boostNorm = CLAMP((float)(driveXyRect.y + padBodyH - 6 - my) / (float)(padBodyH - 20), 0.0f, 1.0f);
            studio.track0.kick.drive.value = drvNorm * 100.0f;
            studio.track0.kick.bassBoost.value = boostNorm * 100.0f;
        } else if (activeDrag == DRAG_FOLD_BAR) {
            float norm = CLAMP((float)(mx - (foldBarRect.x + 2)) / (float)(foldBarRect.w - 4), 0.0f, 1.0f);
            int seg = (int)std::round(norm * 20.0f);
            studio.track0.kick.fold.value = seg * 5.0f;
        } else if (activeDrag == DRAG_FM_XY) {
            int padBodyH = fmXyRect.h - 18;
            float depthNorm = CLAMP((float)(mx - (fmXyRect.x + 6)) / (float)(fmXyRect.w - 12), 0.0f, 1.0f);
            float snapNorm = CLAMP((float)(fmXyRect.y + padBodyH - 6 - my) / (float)(padBodyH - 20), 0.0f, 1.0f);
            studio.track0.kick.fmDepth.value = depthNorm * 100.0f;
            studio.track0.kick.fmSnap.value = 2.0f + snapNorm * 148.0f;
        } else if (activeDrag == DRAG_FM_RATIO_BAR) {
            float norm = CLAMP((float)(mx - (fmRatioBarRect.x + 2)) / (float)(fmRatioBarRect.w - 4), 0.0f, 1.0f);
            int seg = (int)std::round(norm * 30.0f);
            studio.track0.kick.fmRatio.value = 0.5f + seg * 0.25f;
        } else if (activeDrag == DRAG_CLICK_XY) {
            float amtNorm = CLAMP((float)(mx - (clickXyRect.x + 6)) / (float)(clickXyRect.w - 12), 0.0f, 1.0f);
            float decNorm = CLAMP((float)(clickXyRect.y + clickXyRect.h - 6 - my) / (float)(clickXyRect.h - 20), 0.0f, 1.0f);
            studio.track0.kick.kickClickAmt.value = amtNorm * 100.0f;
            studio.track0.kick.kickClickDecay.value = 1.0f + decNorm * 99.0f;
        } else if (activeDrag == DRAG_PARAM_SLIDER && dragParamIdx >= 0 && dragParamIdx < (int)paramSliders.size()) {
            Param* p = paramSliders[dragParamIdx].param;
            float norm = CLAMP((float)(mx - paramSliders[dragParamIdx].rect.x) / (float)paramSliders[dragParamIdx].rect.w, 0.0f, 1.0f);
            p->value = p->min + norm * (p->max - p->min);
        } else if (activeDrag == DRAG_STEP_NOTE && dragStepIdx >= 0 && dragStepIdx < SEQ_STEPS_TEK) {
            int dy = (dragStartY - my) / 6;
            int newNote = CLAMP(dragStartNote + dy, 36, 84);
            studio.track0.sequence[dragStepIdx].note = newNote;
        }
    }

    void onMouseUp()
    {
        activeDrag = DRAG_NONE;
        dragParamIdx = -1;
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

        if (volumeSliderRect.contains(mx, my)) {
            float newVol = CLAMP(studio.track0.volume + (delta > 0 ? 0.05f : -0.05f), 0.0f, 1.0f);
            studio.track0.volume = newVol;
            return;
        }

        if (durationBarRect.contains(mx, my)) {
            Param& p = studio.track0.kick.duration;
            float newDur = CLAMP(p.value + (delta > 0 ? 10.0f : -10.0f), p.min, p.max);
            p.value = newDur;
            return;
        }

        if (semitoneBarRect.contains(mx, my)) {
            int curSemi = studio.track0.kick.semitoneOffset.load();
            int newSemi = std::clamp(curSemi + (delta > 0 ? 1 : -1), -12, 12);
            studio.track0.kick.semitoneOffset.store(newSemi);
            return;
        }

        if (fmRatioBarRect.contains(mx, my)) {
            Param& p = studio.track0.kick.fmRatio;
            float newRatio = CLAMP(p.value + (delta > 0 ? 0.25f : -0.25f), p.min, p.max);
            p.value = newRatio;
            return;
        }

        if (foldBarRect.contains(mx, my)) {
            Param& p = studio.track0.kick.fold;
            float newFold = CLAMP(p.value + (delta > 0 ? 5.0f : -5.0f), p.min, p.max);
            p.value = newFold;
            return;
        }

        if (sweepCurveRect.contains(mx, my)) {
            float newShp = CLAMP(studio.track0.kick.sweepShp.value + (delta > 0 ? 2.0f : -2.0f), 0.0f, 100.0f);
            studio.track0.kick.sweepShp.value = newShp;
            return;
        }

        if (vcoMorphRect.contains(mx, my)) {
            float newMorph = CLAMP(studio.track0.kick.vcoMorph.value + (delta > 0 ? 2.0f : -2.0f), 0.0f, 100.0f);
            studio.track0.kick.vcoMorph.value = newMorph;
            return;
        }

        for (int i = 0; i < SEQ_STEPS_TEK; i++) {
            if (stepRects[i].contains(mx, my)) {
                int newNote = CLAMP(studio.track0.sequence[i].note + (delta > 0 ? 1 : -1), 36, 84);
                studio.track0.sequence[i].note = newNote;
                return;
            }
        }
    }
};

extern UiZicTeK uiZicTeK;
