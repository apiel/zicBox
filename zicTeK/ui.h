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

enum DragMode {
    DRAG_NONE,
    DRAG_SWEEP_CURVE,
    DRAG_VCO_MORPH,
    DRAG_CLICK_XY,
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
    int dragStartY = 0;
    int dragStartValY = 0;
    int dragStartNote = 60;
    float animTime = 0.0f;

    // Interactive Widget Hit Boxes
    BoxRect playBtnRect;
    BoxRect bpmRect;

    BoxRect sweepCurveRect;
    BoxRect vcoMorphRect;
    BoxRect clickXyRect;
    BoxRect volumeSliderRect;

    // Engine Parameter Sliders
    struct ParamSlider {
        std::string label;
        Param* param;
        BoxRect rect;
    };
    std::vector<ParamSlider> paramSliders;

    // Sequencer Step Rects
    BoxRect stepRects[SEQ_STEPS_TEK];

    UiZicTeK()
    {
    }

    void drawHeader(Draw& d, int winW, int winH)
    {
        int headH = 38;
        // Header background with glowing bottom accent line
        d.filledRect({ 0, 0 }, { winW, headH }, { .color = { 14, 18, 26, 255 } });
        d.line({ 0, headH - 1 }, { winW, headH - 1 }, { .color = { 0, 180, 255, 255 } });
        d.line({ 0, headH - 2 }, { winW, headH - 2 }, { .color = { 0, 90, 140, 150 } });

        // Branding Title
        d.text({ 14, 8 }, "zicTeK", 16, { .color = { 0, 230, 255, 255 }, .font = &PoppinsLight_16 });
        d.text({ 94, 14 }, "CYBER-TEK SYNTHESIZER & WORKSTATION", 8, { .color = { 140, 165, 195, 255 }, .font = &PoppinsLight_8 });

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

        // Subtle diagonal grid pattern inside empty panel
        for (int i = 20; i < pw + ph; i += 40) {
            d.line({ px + std::max(0, i - ph), py + std::min(ph, i) }, { px + std::min(pw, i), py + std::max(0, i - pw) }, { .color = { 18, 24, 36, 255 } });
        }

        d.textCentered({ px + pw / 2, py + ph / 2 }, "UNASSIGNED TRACK PANEL", 12, { .color = { 45, 58, 80, 255 }, .font = &PoppinsLight_12 });
    }

    // Shaped Pitch Sweep Curve helper
    static float getShapedPitch(float p, float shape)
    {
        if (shape < 0.20f) {
            return CLAMP(std::sqrt(p) * (1.0f - shape * 5.0f) + p * (shape * 5.0f), 0.0f, 1.0f);
        } else if (shape < 0.40f) {
            float t = (shape - 0.20f) * 5.0f;
            return CLAMP(p * (1.0f - t) + (p * p) * t, 0.0f, 1.0f);
        } else if (shape < 0.60f) {
            float t = (shape - 0.40f) * 5.0f;
            float sCurve = p * p * (3.0f - 2.0f * p);
            return CLAMP((p * p) * (1.0f - t) + (sCurve * sCurve) * t, 0.0f, 1.0f);
        } else if (shape < 0.80f) {
            float t = (shape - 0.60f) * 5.0f;
            float sCurve = p * p * (3.0f - 2.0f * p);
            float subDive = std::pow(p, 4.0f);
            return CLAMP((sCurve * sCurve) * (1.0f - t) + subDive * t, 0.0f, 1.0f);
        } else {
            float t = (shape - 0.80f) * 5.0f;
            float sCurve = p * p * (3.0f - 2.0f * p);
            float bounce = sCurve * sCurve + (0.15f * std::sin(3.14159f * p) * p);
            float subDive = std::pow(p, 4.0f);
            return CLAMP(subDive * (1.0f - t) + bounce * t, 0.0f, 1.0f);
        }
    }

    void drawTrack0Panel(Draw& d, int px, int py, int pw, int ph)
    {
        animTime += 0.03f;

        // Panel Background & Glowing Cyan Border
        d.filledRect({ px, py }, { pw, ph }, { .color = { 12, 16, 24, 255 } });
        d.rect({ px, py }, { pw, ph }, { .color = { 0, 190, 230, 255 } });

        // Header Bar
        d.filledRect({ px + 1, py + 1 }, { pw - 2, 26 }, { .color = { 20, 28, 44, 255 } });
        d.line({ px, py + 27 }, { px + pw, py + 27 }, { .color = { 0, 190, 230, 255 } });
        d.text({ px + 10, py + 6 }, "TRACK 1: IMPACT KICK ENGINE", 12, { .color = { 0, 230, 255, 255 }, .font = &PoppinsLight_12 });

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

        // Widget dimensions
        int widgetW = (pw - 28) / 2;
        int widgetH = 105;

        // --- WIDGET 1: VCO MORPH RADAR VISUALIZER (Top-Left of Widget Area) ---
        // Matches exact zicPixelDrift geometric radar shape aesthetics!
        vcoMorphRect = { px + 10, curY, widgetW, widgetH };
        d.filledRect({ vcoMorphRect.x, vcoMorphRect.y }, { vcoMorphRect.w, vcoMorphRect.h }, { .color = { 8, 11, 18, 255 } });
        d.rect({ vcoMorphRect.x, vcoMorphRect.y }, { vcoMorphRect.w, vcoMorphRect.h }, { .color = { 0, 160, 210, 255 } });
        d.text({ vcoMorphRect.x + 6, vcoMorphRect.y + 4 }, "VCO MORPH (RADAR SHAPE)", 8, { .color = { 0, 230, 255, 255 }, .font = &PoppinsLight_8 });

        int cx = vcoMorphRect.x + vcoMorphRect.w / 2;
        int cy = vcoMorphRect.y + vcoMorphRect.h / 2 + 4;
        int halfW = 28;
        int halfH = 26;

        // Concentric Radar Rings (zicPixelDrift style)
        d.circle({ cx, cy }, 16, { .color = { 20, 45, 65, 255 } });
        d.circle({ cx, cy }, 28, { .color = { 25, 55, 80, 255 } });
        d.circle({ cx, cy }, 40, { .color = { 30, 65, 95, 255 } });

        // Calculate Morphing Geometric Polygon Vertices (Sine -> Tri -> Saw -> Square)
        float wf = studio.track0.kick.vcoMorph.value * 0.01f;
        Point pBL = { cx - halfW, cy + halfH };
        Point pBR = { cx + halfW, cy + halfH };
        Point pTL, pTR;

        if (wf <= 0.333f) {
            float t = wf / 0.333f;
            int topX = cx + (int)(t * halfW * 0.2f);
            pTL = { cx, cy - halfH };
            pTR = { cx, cy - halfH };
        } else if (wf <= 0.666f) {
            float t = (wf - 0.333f) / 0.333f;
            pTL = { cx - (int)(t * halfW * 0.7f), cy - halfH };
            pTR = { cx + (int)(t * halfW), cy - halfH };
        } else {
            float t = (wf - 0.666f) / 0.334f;
            pTL = { cx - halfW, cy - halfH };
            pTR = { cx + halfW, cy - halfH };
        }

        std::vector<Point> baseShape;
        if (std::abs(pTL.x - pTR.x) <= 2) {
            baseShape = { pBL, pTR, pBR };
        } else {
            baseShape = { pBL, pTL, pTR, pBR };
        }

        // Subtly animated jitter & pulse echo
        std::vector<Point> morphedShape = baseShape;

        // Draw Ghost Echo Shape
        std::vector<Point> ghostShape;
        for (const auto& pt : morphedShape) {
            int gx = cx + 8 + (int)((pt.x - cx) * 0.85f);
            int gy = cy + 2 + (int)((pt.y - cy) * 0.85f);
            ghostShape.push_back({ gx, gy });
        }
        d.filledPolygon(ghostShape, { .color = { 0, 160, 210, 30 } });
        d.lines(ghostShape, { .color = { 0, 180, 230, 80 }, .thickness = 1 });
        d.line(ghostShape.back(), ghostShape.front(), { .color = { 0, 180, 230, 80 }, .thickness = 1 });

        // Main Solid Polygon
        d.filledPolygon(morphedShape, { .color = { 0, 200, 240, 75 } });
        d.lines(morphedShape, { .color = { 0, 240, 255, 255 }, .thickness = 2 });
        d.line(morphedShape.back(), morphedShape.front(), { .color = { 0, 240, 255, 255 }, .thickness = 2 });

        // Percentage Text Label
        std::ostringstream morphTxt;
        morphTxt << (int)studio.track0.kick.vcoMorph.value << "%";
        d.textRight({ vcoMorphRect.x + vcoMorphRect.w - 6, vcoMorphRect.y + 4 }, morphTxt.str(), 8, { .color = { 0, 255, 220, 255 }, .font = &PoppinsLight_8 });

        // --- WIDGET 2: INTUITIVE SWEEP SHAPE PITCH CURVE (Top-Right of Widget Area) ---
        // INTUITIVE DRAG: Dragging UP increases sweep shape (bends/lifts curve up), dragging DOWN lowers it!
        sweepCurveRect = { px + 18 + widgetW, curY, widgetW, widgetH };
        d.filledRect({ sweepCurveRect.x, sweepCurveRect.y }, { sweepCurveRect.w, sweepCurveRect.h }, { .color = { 8, 11, 18, 255 } });
        d.rect({ sweepCurveRect.x, sweepCurveRect.y }, { sweepCurveRect.w, sweepCurveRect.h }, { .color = { 255, 160, 40, 255 } });
        d.text({ sweepCurveRect.x + 6, sweepCurveRect.y + 4 }, "SWEEP SHAPE (PITCH DROP)", 8, { .color = { 255, 180, 50, 255 }, .font = &PoppinsLight_8 });

        // Draw Grid Lines
        d.line({ sweepCurveRect.x + 6, sweepCurveRect.y + sweepCurveRect.h / 2 }, { sweepCurveRect.x + sweepCurveRect.w - 6, sweepCurveRect.y + sweepCurveRect.h / 2 }, { .color = { 35, 45, 60, 255 } });

        // Render Pitch Sweep Curve
        float shpNorm = studio.track0.kick.sweepShp.value * 0.01f;
        std::vector<Point> curvePts;
        int steps = 45;
        for (int i = 0; i <= steps; i++) {
            float t = (float)i / (float)steps;
            float pitchVal = getShapedPitch(1.0f - t, shpNorm);
            int cxPt = sweepCurveRect.x + 6 + (int)(t * (sweepCurveRect.w - 12));
            int cyPt = sweepCurveRect.y + sweepCurveRect.h - 10 - (int)(pitchVal * (sweepCurveRect.h - 26));
            curvePts.push_back({ cxPt, cyPt });
        }
        if (curvePts.size() > 1) {
            d.lines(curvePts, { .color = { 255, 170, 40, 255 }, .thickness = 2 });
        }

        // Draggable Control Point on Curve Peak/Bend
        int ctrlX = sweepCurveRect.x + 6 + (int)(0.45f * (sweepCurveRect.w - 12));
        float midPitch = getShapedPitch(0.55f, shpNorm);
        int ctrlY = sweepCurveRect.y + sweepCurveRect.h - 10 - (int)(midPitch * (sweepCurveRect.h - 26));

        // Pulsing target handle on curve
        d.filledCircle({ ctrlX, ctrlY }, 5, { .color = { 255, 220, 90, 255 } });
        d.circle({ ctrlX, ctrlY }, 8, { .color = { 255, 255, 255, 255 } });

        // Helper label: "DRAG UP/DOWN TO BEND"
        d.textCentered({ sweepCurveRect.x + sweepCurveRect.w / 2, sweepCurveRect.y + sweepCurveRect.h - 12 }, "DRAG UP/DOWN TO BEND CURVE", 8, { .color = { 150, 130, 100, 255 }, .font = &PoppinsLight_8 });

        std::ostringstream shpTxt;
        shpTxt << (int)studio.track0.kick.sweepShp.value << "%";
        d.textRight({ sweepCurveRect.x + sweepCurveRect.w - 6, sweepCurveRect.y + 4 }, shpTxt.str(), 8, { .color = { 255, 220, 100, 255 }, .font = &PoppinsLight_8 });

        curY += widgetH + 12;

        // --- WIDGET 3: CLICK RADAR XY TARGET PAD (Bottom-Left of Widget Area) ---
        int xyW = 125;
        int xyH = 80;
        clickXyRect = { px + 10, curY, xyW, xyH };
        d.filledRect({ clickXyRect.x, clickXyRect.y }, { clickXyRect.w, clickXyRect.h }, { .color = { 8, 11, 18, 255 } });
        d.rect({ clickXyRect.x, clickXyRect.y }, { clickXyRect.w, clickXyRect.h }, { .color = { 255, 80, 120, 255 } });
        d.text({ clickXyRect.x + 6, clickXyRect.y + 4 }, "CLICK (AMT / DEC XY)", 8, { .color = { 255, 100, 140, 255 }, .font = &PoppinsLight_8 });

        // Grid lines & Radar concentric circles inside XY Pad
        d.line({ clickXyRect.x + xyW / 2, clickXyRect.y + 16 }, { clickXyRect.x + xyW / 2, clickXyRect.y + xyH - 4 }, { .color = { 40, 30, 48, 255 } });
        d.line({ clickXyRect.x + 4, clickXyRect.y + 16 + (xyH - 20) / 2 }, { clickXyRect.x + xyW - 4, clickXyRect.y + 16 + (xyH - 20) / 2 }, { .color = { 40, 30, 48, 255 } });

        // Calculate handle position: X = Amt (0..100%), Y = Decay (1..100ms)
        float amtNorm = studio.track0.kick.kickClickAmt.value * 0.01f;
        float decNorm = (studio.track0.kick.kickClickDecay.value - 1.0f) / 99.0f;
        int targetX = clickXyRect.x + 6 + (int)(amtNorm * (xyW - 12));
        int targetY = clickXyRect.y + xyH - 6 - (int)(decNorm * (xyH - 22));

        // Glowing crosshair laser target
        d.line({ targetX - 7, targetY }, { targetX + 7, targetY }, { .color = { 255, 100, 140, 255 } });
        d.line({ targetX, targetY - 7 }, { targetX, targetY + 7 }, { .color = { 255, 100, 140, 255 } });
        d.filledCircle({ targetX, targetY }, 4, { .color = { 255, 180, 200, 255 } });

        std::ostringstream xyTxt;
        xyTxt << (int)studio.track0.kick.kickClickAmt.value << "% / " << (int)studio.track0.kick.kickClickDecay.value << "ms";
        d.textRight({ clickXyRect.x + clickXyRect.w - 6, clickXyRect.y + 4 }, xyTxt.str(), 8, { .color = { 255, 150, 180, 255 }, .font = &PoppinsLight_8 });

        // --- ENGINE PARAMETERS SLIDERS GRID (Bottom-Right of Widget Area) ---
        paramSliders.clear();
        ImpactKick& k = studio.track0.kick;

        std::vector<Param*> params = {
            &k.baseFreq, &k.duration, &k.drive, &k.bassBoost,
            &k.fold, &k.fmDepth, &k.fmRatio, &k.fmSnap
        };

        int paramStartX = px + 10 + xyW + 12;
        int paramW = pw - (10 + xyW + 12 + 10);
        int sliderH = 15;
        int paramGap = 4;

        for (size_t i = 0; i < params.size(); i++) {
            int sy = curY + (int)i * (sliderH + paramGap);
            if (sy + sliderH > py + ph - 48) break;

            Param* p = params[i];
            BoxRect sRect = { paramStartX, sy, paramW, sliderH };
            paramSliders.push_back({ p->label, p, sRect });

            d.filledRect({ sRect.x, sRect.y }, { sRect.w, sRect.h }, { .color = { 16, 22, 34, 255 } });

            // Slider Progress Fill
            float norm = (p->value - p->min) / (p->max - p->min);
            int sFillW = (int)(sRect.w * CLAMP(norm, 0.0f, 1.0f));
            d.filledRect({ sRect.x, sRect.y }, { sFillW, sRect.h }, { .color = { 35, 95, 140, 255 } });
            d.rect({ sRect.x, sRect.y }, { sRect.w, sRect.h }, { .color = { 50, 75, 110, 255 } });

            // Label & Value display
            d.text({ sRect.x + 4, sRect.y + 2 }, p->label, 8, { .color = { 220, 235, 255, 255 }, .font = &PoppinsLight_8 });
            std::ostringstream valStr;
            valStr << std::fixed << std::setprecision(1) << p->value << " " << p->unit;
            d.textRight({ sRect.x + sRect.w - 4, sRect.y + 2 }, valStr.str(), 8, { .color = { 0, 230, 255, 255 }, .font = &PoppinsLight_8 });
        }

        // --- 16-STEP SEQUENCER FOR TRACK 0 ---
        int seqY = py + ph - 42;
        int seqW = pw - 20;
        int stepBoxW = (seqW - (SEQ_STEPS_TEK - 1) * 2) / SEQ_STEPS_TEK;
        int stepBoxH = 34;

        d.text({ px + 10, seqY - 11 }, "16-STEP SEQUENCER (CLICK TOGGLE, SCROLL/DRAG NOTE PITCH)", 8, { .color = { 140, 165, 200, 255 }, .font = &PoppinsLight_8 });

        int playHead = studio.currentStep.load();

        for (int i = 0; i < SEQ_STEPS_TEK; i++) {
            int sx = px + 10 + i * (stepBoxW + 2);
            stepRects[i] = { sx, seqY, stepBoxW, stepBoxH };

            auto& stp = studio.track0.sequence[i];
            bool isCurrent = (playHead == i && studio.isPlaying);

            Color stepBg;
            if (stp.active) {
                stepBg = (i % 4 == 0) ? Color { 0, 190, 230, 255 } : Color { 0, 150, 190, 255 };
            } else {
                stepBg = (i % 4 == 0) ? Color { 26, 34, 50, 255 } : Color { 18, 24, 36, 255 };
            }

            d.filledRect({ sx, seqY }, { stepBoxW, stepBoxH }, { .color = stepBg });

            Color borderCol = isCurrent ? Color { 255, 240, 0, 255 } : Color { 45, 60, 85, 255 };
            d.rect({ sx, seqY }, { stepBoxW, stepBoxH }, { .color = borderCol });
            if (isCurrent) {
                d.rect({ sx + 1, seqY + 1 }, { stepBoxW - 2, stepBoxH - 2 }, { .color = { 255, 255, 100, 255 } });
            }

            // Step number or Note text
            if (stp.active) {
                std::string noteStr = (stp.note >= 0 && stp.note < 132) ? MIDI_NOTES_STR[stp.note] : "C4";
                d.textCentered({ sx + stepBoxW / 2, seqY + 11 }, noteStr, 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
            } else {
                std::ostringstream numStr;
                numStr << (i + 1);
                d.textCentered({ sx + stepBoxW / 2, seqY + 11 }, numStr.str(), 8, { .color = { 80, 100, 130, 255 }, .font = &PoppinsLight_8 });
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
            activeDrag = DRAG_SWEEP_CURVE;
            dragStartY = my;
            dragStartValY = (int)studio.track0.kick.sweepShp.value;

            // Direct Y-axis mapping: dragging UP (lower mouse Y) increases sweep shape, dragging DOWN decreases it!
            float normY = CLAMP((float)(sweepCurveRect.y + sweepCurveRect.h - 10 - my) / (float)(sweepCurveRect.h - 26), 0.0f, 1.0f);
            studio.track0.kick.sweepShp.value = normY * 100.0f;
            return;
        }

        if (vcoMorphRect.contains(mx, my)) {
            activeDrag = DRAG_VCO_MORPH;
            float norm = CLAMP((float)(mx - (vcoMorphRect.x + 6)) / (float)(vcoMorphRect.w - 12), 0.0f, 1.0f);
            studio.track0.kick.vcoMorph.value = norm * 100.0f;
            return;
        }

        if (clickXyRect.contains(mx, my)) {
            activeDrag = DRAG_CLICK_XY;
            float amtNorm = CLAMP((float)(mx - (clickXyRect.x + 6)) / (float)(clickXyRect.w - 12), 0.0f, 1.0f);
            float decNorm = CLAMP((float)(clickXyRect.y + clickXyRect.h - 6 - my) / (float)(clickXyRect.h - 22), 0.0f, 1.0f);

            studio.track0.kick.kickClickAmt.value = amtNorm * 100.0f;
            studio.track0.kick.kickClickDecay.value = 1.0f + decNorm * 99.0f;
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

        for (int i = 0; i < SEQ_STEPS_TEK; i++) {
            if (stepRects[i].contains(mx, my)) {
                studio.track0.sequence[i].active = !studio.track0.sequence[i].active;
                activeDrag = DRAG_STEP_NOTE;
                dragStepIdx = i;
                dragStartY = my;
                dragStartNote = studio.track0.sequence[i].note;
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
        } else if (activeDrag == DRAG_SWEEP_CURVE) {
            // Intuitive 2D/Vertical drag: moving UP (decreasing my) increases sweep shape, moving DOWN decreases it!
            float normY = CLAMP((float)(sweepCurveRect.y + sweepCurveRect.h - 10 - my) / (float)(sweepCurveRect.h - 26), 0.0f, 1.0f);
            studio.track0.kick.sweepShp.value = normY * 100.0f;
        } else if (activeDrag == DRAG_VCO_MORPH) {
            float norm = CLAMP((float)(mx - (vcoMorphRect.x + 6)) / (float)(vcoMorphRect.w - 12), 0.0f, 1.0f);
            studio.track0.kick.vcoMorph.value = norm * 100.0f;
        } else if (activeDrag == DRAG_CLICK_XY) {
            float amtNorm = CLAMP((float)(mx - (clickXyRect.x + 6)) / (float)(clickXyRect.w - 12), 0.0f, 1.0f);
            float decNorm = CLAMP((float)(clickXyRect.y + clickXyRect.h - 6 - my) / (float)(clickXyRect.h - 22), 0.0f, 1.0f);
            studio.track0.kick.kickClickAmt.value = amtNorm * 100.0f;
            studio.track0.kick.kickClickDecay.value = 1.0f + decNorm * 99.0f;
        } else if (activeDrag == DRAG_PARAM_SLIDER && dragParamIdx >= 0 && dragParamIdx < (int)paramSliders.size()) {
            Param* p = paramSliders[dragParamIdx].param;
            float norm = CLAMP((float)(mx - paramSliders[dragParamIdx].rect.x) / (float)paramSliders[dragParamIdx].rect.w, 0.0f, 1.0f);
            p->value = p->min + norm * (p->max - p->min);
        } else if (activeDrag == DRAG_STEP_NOTE && dragStepIdx >= 0 && dragStepIdx < SEQ_STEPS_TEK) {
            int dy = (dragStartY - my) / 6; // 1 semitone per 6 pixels drag
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
