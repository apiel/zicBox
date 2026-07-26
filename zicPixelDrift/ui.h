#pragma once

#include "draw/draw.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"
#include "kickBody.h"
#include "mixer.h"
#include "sequencer.h"
#include "spaceBackground.h"
#include "synth1.h"
#include "synth2.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

enum ViewState {
    VIEW_KICK_BODY1,    // [Q] Page 1
    VIEW_KICK_BODY2,    // [Q] Page 2
    VIEW_SYNTH1_PAGE1,  // [W] Page 1
    VIEW_SYNTH1_PAGE2,  // [W] Page 2
    VIEW_SYNTH1_PAGE3,  // [E] Page 1
    VIEW_SYNTH2_PAGE1,  // [E] Page 2
    VIEW_SYNTH2_PAGE2,  // [R] Page 1
    VIEW_SYNTH2_PAGE3,  // [R] Page 2
    VIEW_MASTER_PAGE1,  // [F] Page 1
    VIEW_MASTER_PAGE2,  // [F] Page 2
    VIEW_COUNT
};

struct EncoderKnob {
    std::string label;
    float* value;
    float minVal;
    float maxVal;
    std::string unit;
    float step;
    std::vector<std::string> displayOptions {};
    char** stringPtr = nullptr;
    Param* paramPtr = nullptr;
};

class UiPixelDrift {
public:
    ViewState currentView = VIEW_KICK_BODY1;

    KickBody& kick;
    Synth1& synth1;
    Synth2& synth2;
    Sequencer& seq;
    Mixer& mixer;

    SpaceBackground spaceBg;

    int activeEncoderHover = -1;
    bool isSynth1Muted = false;
    bool isSynth2Muted = false;

    // Visual feedback animation state
    float animTime = 0.0f;
    float kickPulseLevel = 0.0f;
    float synth1PulseLevel = 0.0f;
    int lastSeqStep = -1;

    UiPixelDrift(KickBody& k, Synth1& s1, Synth2& s2, Sequencer& sq, Mixer& m)
        : kick(k)
        , synth1(s1)
        , synth2(s2)
        , seq(sq)
        , mixer(m)
    {
    }

    EncoderKnob fromParam(Param& p)
    {
        return { p.label, &p.value, p.min, p.max, p.unit ? std::string(p.unit) : "", p.step, {}, &p.string, &p };
    }

    void handleViewKey(char key, bool& needFullRedraw)
    {
        needFullRedraw = true;
        if (key == 'q' || key == 'Q') {
            currentView = (currentView == VIEW_KICK_BODY1) ? VIEW_KICK_BODY2 : VIEW_KICK_BODY1;
        } else if (key == 'w' || key == 'W') {
            currentView = (currentView == VIEW_SYNTH1_PAGE1) ? VIEW_SYNTH1_PAGE2 : VIEW_SYNTH1_PAGE1;
        } else if (key == 'e' || key == 'E') {
            currentView = (currentView == VIEW_SYNTH1_PAGE3) ? VIEW_SYNTH2_PAGE1 : VIEW_SYNTH1_PAGE3;
        } else if (key == 'r' || key == 'R') {
            currentView = (currentView == VIEW_SYNTH2_PAGE2) ? VIEW_SYNTH2_PAGE3 : VIEW_SYNTH2_PAGE2;
        } else if (key == 'f' || key == 'F') {
            currentView = (currentView == VIEW_MASTER_PAGE1) ? VIEW_MASTER_PAGE2 : VIEW_MASTER_PAGE1;
        }
    }

    void handlePerformancePad(char key, bool pressed, bool& needFullRedraw)
    {
        if (key == 'z' || key == 'Z') {
            kick.isBodyMuted = pressed;
            return;
        }

        if (!pressed) return;
        needFullRedraw = true;

        if (key == 'a' || key == 'A') {
            kick.trigger(1.0f);
            spaceBg.triggerKickPulse();
            kickPulseLevel = 1.0f;
        } else if (key == 's' || key == 'S') {
            synth1.trigger();
            synth1PulseLevel = 1.0f;
        } else if (key == 'd' || key == 'D') {
            synth2.trigger();
        } else if (key == 'x' || key == 'X') {
            isSynth1Muted = !isSynth1Muted;
        } else if (key == 'c' || key == 'C') {
            isSynth2Muted = !isSynth2Muted;
        } else if (key == 'v' || key == 'V') {
            seq.isMutatedFill = !seq.isMutatedFill;
        }
    }

    std::vector<EncoderKnob> getActiveEncoders()
    {
        std::vector<EncoderKnob> encs;

        switch (currentView) {
        case VIEW_KICK_BODY1:
            encs = {
                fromParam(kick.baseFreq),
                fromParam(kick.clickAmt),
                fromParam(kick.duration),
                fromParam(kick.vcoMorph)
            };
            break;

        case VIEW_KICK_BODY2:
            encs = {
                fromParam(kick.fmDepth),
                fromParam(kick.drive),
                fromParam(kick.rumbleAmt),
                fromParam(kick.rumbleGap)
            };
            break;

        case VIEW_SYNTH1_PAGE1:
            encs = {
                fromParam(synth1.pitch),
                fromParam(synth1.waveform),
                fromParam(synth1.cutoff),
                fromParam(synth1.resonance)
            };
            break;

        case VIEW_SYNTH1_PAGE2:
            encs = {
                fromParam(synth1.release),
                fromParam(synth1.envAmt),
                fromParam(synth1.filterMorph),
                fromParam(synth1.delaySend)
            };
            break;

        case VIEW_SYNTH1_PAGE3:
            encs = {
                { "Mod Type", &synth1.modType.value, 0.0f, 11.0f, "", 1.0f, {
                    "ENV Cutoff", "ENV Pitch", "ENV Wave",
                    "LFO Tri Cut", "LFO Tri Pit", "LFO Tri Wave", "LFO Tri Lvl",
                    "LFO Saw Cut", "LFO Saw Pit", "LFO Saw Wave",
                    "LFO S&H Cut", "LFO S&H Pit"
                }},
                fromParam(synth1.modDepth),
                fromParam(synth1.modSpeed),
                fromParam(synth1.synthMix)
            };
            break;

        case VIEW_SYNTH2_PAGE1:
            encs = {
                fromParam(synth2.pitch),
                { "Chord", &synth2.chord.value, 0.0f, 5.0f, "", 1.0f, { "Unison", "Fifth", "Octave", "Maj 7th", "Min 7th", "Sus 4" } },
                fromParam(synth2.wtSelect),
                fromParam(synth2.wavetable)
            };
            break;

        case VIEW_SYNTH2_PAGE2:
            encs = {
                fromParam(synth2.cutoff),
                fromParam(synth2.resonance),
                fromParam(synth2.attack),
                fromParam(synth2.release)
            };
            break;

        case VIEW_SYNTH2_PAGE3:
            encs = {
                { "Mod Type", &synth2.modType.value, 0.0f, 11.0f, "", 1.0f, {
                    "ENV Cutoff", "ENV Pitch", "ENV Morph",
                    "LFO Tri Cut", "LFO Tri Pit", "LFO Tri Morph", "LFO Tri Lvl",
                    "LFO Saw Cut", "LFO Saw Pit", "LFO Saw Morph",
                    "LFO S&H Cut", "LFO S&H Pit"
                }},
                fromParam(synth2.modDepth),
                fromParam(synth2.modSpeed),
                fromParam(synth2.delaySend)
            };
            break;

        case VIEW_MASTER_PAGE1:
            encs = {
                { "VOLUME", &mixer.volume, 0.0f, 1.0f, "", 0.05f },
                { "MIX BALANCE", &mixer.mix, 0.0f, 1.0f, "", 0.05f },
                { "DELAY TIME", &mixer.delayTimeMs, 10.0f, 1000.0f, " ms", 10.0f },
                { "DELAY FDBK", &mixer.delayFeedback, 0.0f, 0.95f, "", 0.05f }
            };
            break;

        case VIEW_MASTER_PAGE2:
            encs = {
                { "BPM", &seq.bpm, 40.0f, 260.0f, " bpm", 1.0f },
                { "GEN KICK", &seq.genKick, 0.0f, 1.0f, " %", 0.05f },
                { "SYN1 TRIG", &seq.synth1TrigMode, 0.0f, 7.0f, "", 1.0f, seq.trigDisplayStrings },
                { "SYN2 TRIG", &seq.synth2TrigMode, 0.0f, 7.0f, "", 1.0f, seq.trigDisplayStrings }
            };
            break;

        default:
            break;
        }

        return encs;
    }

    void handleEncoderTurn(int encoderIdx, int direction, bool& needFullRedraw)
    {
        auto encs = getActiveEncoders();
        if (encoderIdx >= 0 && encoderIdx < (int)encs.size()) {
            needFullRedraw = true;
            auto& e = encs[encoderIdx];
            float newVal = *(e.value) + (direction * e.step);
            newVal = std::clamp(newVal, e.minVal, e.maxVal);
            if (e.paramPtr) {
                e.paramPtr->set(newVal);
            } else {
                *(e.value) = newVal;
            }

            if (e.label == "BPM") {
                seq.setBpm(*(e.value));
            } else if (e.label == "GEN KICK") {
                seq.updateKickEuclidean();
            }
        }
    }

    Color getViewThemeColor(ViewState view)
    {
        switch (view) {
        case VIEW_KICK_BODY1:
        case VIEW_KICK_BODY2:
            return Color { 0, 180, 255, 255 }; // Electric Kick Blue

        case VIEW_SYNTH1_PAGE1:
        case VIEW_SYNTH1_PAGE2:
        case VIEW_SYNTH1_PAGE3:
            return Color { 0, 230, 180, 255 }; // Neon Synth1 Cyan/Teal

        case VIEW_SYNTH2_PAGE1:
        case VIEW_SYNTH2_PAGE2:
        case VIEW_SYNTH2_PAGE3:
            return Color { 190, 90, 255, 255 }; // Electric Synth2 Purple

        case VIEW_MASTER_PAGE1:
        case VIEW_MASTER_PAGE2:
            return Color { 255, 195, 0, 255 }; // Bright Master Gold

        default:
            return Color { 0, 210, 255, 255 };
        }
    }

    std::string getViewTitle()
    {
        switch (currentView) {
        case VIEW_KICK_BODY1: return "KICK: CLICK & VCO MORPH";
        case VIEW_KICK_BODY2: return "KICK: FM, DRIVE & RUMBLE";
        case VIEW_SYNTH1_PAGE1: return "SYNTH 1: TONE & FILTER";
        case VIEW_SYNTH1_PAGE2: return "SYNTH 1: ENV & DELAY SEND";
        case VIEW_SYNTH1_PAGE3: return "SYNTH 1: MOD & SYNTH MIX";
        case VIEW_SYNTH2_PAGE1: return "SYNTH 2: CHORD & WAVETABLE";
        case VIEW_SYNTH2_PAGE2: return "SYNTH 2: FILTER & ENV";
        case VIEW_SYNTH2_PAGE3: return "SYNTH 2: MOD & DELAY SEND";
        case VIEW_MASTER_PAGE1: return "MASTER: VOL, MIX & DELAY";
        case VIEW_MASTER_PAGE2: return "SEQUENCER: BPM & KICK GEN";
        default: return "zicPixelDrift";
        }
    }

    static constexpr int PARAM_ROW_H = 34;

    void drawParamCard(Draw& d, const EncoderKnob& e, int x, int y, int colW, bool isActiveHover, const Color& themeColor)
    {
        int cardH = PARAM_ROW_H - 2; // 32px height
        int cardW = colW - 2;

        Color cardBg = isActiveHover ? Color { 28, 35, 48, 255 } : Color { 20, 22, 30, 255 };
        Color cardBorder = isActiveHover ? themeColor : Color { 45, 52, 68, 255 };
        Color pColor = themeColor;

        d.filledRect({ x, y }, { cardW, cardH }, { .color = cardBg });
        d.rect({ x, y }, { cardW, cardH }, { .color = cardBorder });

        // Label
        d.text({ x + 4, y + 2 }, e.label, 8, { .color = d.styles.colors.text, .font = &PoppinsLight_8 });

        // Value text
        std::string valStr = "";
        if (e.stringPtr && *e.stringPtr && strlen(*e.stringPtr) > 0) {
            valStr = *e.stringPtr;
        } else if (!e.displayOptions.empty()) {
            int optIdx = std::clamp((int)std::round(*(e.value)), 0, (int)e.displayOptions.size() - 1);
            valStr = e.displayOptions[optIdx];
        } else {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(1) << *(e.value) << e.unit;
            valStr = ss.str();
        }
        d.text({ x + 4, y + 14 }, valStr, 8, { .color = { 170, 170, 180, 255 }, .font = &PoppinsLight_8, .maxWidth = cardW - 6 });

        // Gauge slider bar at bottom (height 3px)
        int bX = x + 4;
        int bY = y + cardH - 5;
        int bW = cardW - 8;

        float range = e.maxVal - e.minVal;
        float pct = (*(e.value) - e.minVal) / (range <= 0.0f ? 1.0f : range);
        pct = std::clamp(pct, 0.0f, 1.0f);

        if (!e.displayOptions.empty() || (e.step > 0.0f && (e.maxVal - e.minVal) / e.step <= 25.0f)) {
            int segmentCount = !e.displayOptions.empty() ? (int)e.displayOptions.size() : (int)((e.maxVal - e.minVal) / e.step) + 1;
            if (segmentCount > 1 && segmentCount <= 25) {
                int currentIndex = !e.displayOptions.empty() ? (int)std::round(*(e.value)) : (int)std::round((*(e.value) - e.minVal) / e.step);
                currentIndex = std::max(0, std::min(currentIndex, segmentCount - 1));

                int gap = 2;
                int segW = std::max(1, (bW - (gap * (segmentCount - 1))) / segmentCount);

                d.filledRect({ bX, bY }, { bW, 3 }, { .color = { 40, 42, 50, 255 } });
                for (int segIdx = 0; segIdx < segmentCount; segIdx++) {
                    int segX = bX + segIdx * (segW + gap);
                    d.filledRect({ segX, bY }, { segW, 3 }, { .color = (segIdx == currentIndex) ? pColor : Color { 70, 75, 88, 255 } });
                }
                return;
            }
        }

        if (e.minVal < 0.0f && e.maxVal > 0.0f) {
            int mid = bX + bW / 2;
            float maxAbs = std::max(std::abs(e.minVal), std::abs(e.maxVal));
            float normVal = *(e.value) / (maxAbs <= 0.0f ? 1.0f : maxAbs);
            int fw = (int)((bW / 2.0f) * normVal);

            d.filledRect({ bX, bY }, { bW, 3 }, { .color = { 40, 42, 50, 255 } });
            if (fw < 0) d.filledRect({ mid + fw, bY }, { std::abs(fw), 3 }, { .color = pColor });
            else d.filledRect({ mid, bY }, { fw, 3 }, { .color = pColor });
            d.filledRect({ mid, bY - 1 }, { 1, 5 }, { .color = { 110, 120, 140, 255 } });
        } else {
            d.filledRect({ bX, bY }, { bW, 3 }, { .color = { 40, 42, 50, 255 } });
            d.filledRect({ bX, bY }, { (int)(bW * pct), 3 }, { .color = pColor });
        }
    }

    void drawVisualFeedback(Draw& d, int winW, int winH)
    {
        int feedbackY = 46;
        int feedbackH = winH - feedbackY - 14; // Y = 46..162 (116px height)
        int feedbackW = winW;

        // Advance animation frame timer
        animTime += 0.05f;

        // Auto-trigger kick & synth pulse shockwave on step hit
        if (seq.isPlaying && seq.currentStep != lastSeqStep) {
            lastSeqStep = seq.currentStep;
            if (seq.kickPattern[lastSeqStep] || (seq.isMutatedFill && (lastSeqStep % 2 == 0))) {
                kickPulseLevel = 1.0f;
            }
            if (seq.shouldTrigSynth((int)std::round(seq.synth1TrigMode), lastSeqStep, seq.kickPattern[lastSeqStep])) {
                synth1PulseLevel = 1.0f;
            }
        }

        // 1. Render dynamic 3D Space Background in feedback region
        // Lets keep this commented out for now! Please do not uncomment.
        // spaceBg.updateAndDraw(d, seq.bpm, kick.drive.value, winW, winH, 0, feedbackY, feedbackW, feedbackH);

        // 2. Render Page-Specific Graph / Feedback Overlay
        int graphX = 8;
        int graphY = feedbackY + 4;
        int graphW = winW - 16;
        int graphH = feedbackH - 8;

        Color themeColor = getViewThemeColor(currentView);

        // Overlay semi-transparent visualizer box frame in active part color
        d.rect({ graphX - 1, graphY - 1 }, { graphW + 2, graphH + 2 }, { .color = { themeColor.r, themeColor.g, themeColor.b, 140 } });

        switch (currentView) {
        case VIEW_KICK_BODY1:
        case VIEW_KICK_BODY2:
        {
            Color themeCol = getViewThemeColor(currentView);

            int cx = graphX + graphW / 2;
            int cy = graphY + (graphH / 2) - 3;
            int halfW = 28;
            int halfH = 22;

            // Parameters
            float morphVal = kick.vcoMorph.value / 100.0f;
            morphVal = std::clamp(morphVal, 0.0f, 1.0f);

            float clickAmt = kick.clickAmt.value;
            float durMs = kick.duration.value;
            float freqHz = kick.baseFreq.value;

            // 1. Kick Trigger Pulse Decay & Expanding Shockwaves (Clean & Soft Opacity)
            float decayRate = 12.0f / (std::clamp(durMs, 50.0f, 1500.0f) + 50.0f);
            kickPulseLevel = std::max(0.0f, kickPulseLevel - decayRate);

            if (kickPulseLevel > 0.01f) {
                for (int r = 0; r < 3; r++) {
                    float pFactor = kickPulseLevel - (r * 0.22f);
                    if (pFactor > 0.0f) {
                        int radius = (int)(28.0f + (1.0f - pFactor) * 36.0f + r * 6);
                        uint8_t alpha = (uint8_t)(pFactor * 130.0f);
                        d.circle({ cx, cy }, radius, { .color = { themeCol.r, themeCol.g, themeCol.b, alpha } });
                    }
                }
            }

            // 2. VCO Morph Geometry (Triangle -> Right Triangle ("triangle rectangle") -> Square / Rectangle)
            Point pBL = { cx - halfW, cy + halfH };
            Point pBR = { cx + halfW, cy + halfH };
            Point pTL, pTR;

            if (morphVal <= 0.5f) {
                // 0.0 -> 0.5: Morph from Symmetrical Triangle (top peak at cx) to Right-Angled Triangle (top peak at cx + halfW)
                float t = morphVal / 0.5f;
                int topX = cx + (int)(t * halfW);
                pTL = { topX, cy - halfH };
                pTR = { topX, cy - halfH };
            } else {
                // 0.5 -> 1.0: Morph from Right-Angled Triangle to Full Square / Rectangle
                // pTR stays at top-right (cx + halfW), pTL moves across to top-left (cx - halfW)
                float t = (morphVal - 0.5f) / 0.5f;
                int tlX = (cx + halfW) - (int)(t * 2.0f * halfW);
                pTR = { cx + halfW, cy - halfH };
                pTL = { tlX, cy - halfH };
            }

            std::vector<Point> morphShape;
            if (std::abs(pTL.x - pTR.x) <= 1) {
                morphShape = { pBL, pTR, pBR };
            } else {
                morphShape = { pBL, pTL, pTR, pBR };
            }

            // Rumble Sub-Bass Background Ghost Waveform Echo (Rumble Amount & Gap Offset)
            float rAmt = std::clamp(kick.rumbleAmt.value / 100.0f, 0.0f, 1.0f);
            float rGapMs = kick.rumbleGap.value; // 10 to 400ms

            if (rAmt > 0.01f) {
                int rOffsetX = (int)((rGapMs / 400.0f) * 32.0f);
                int rOffsetY = (int)((rGapMs / 400.0f) * 6.0f);
                uint8_t rAlpha = (uint8_t)(std::max(0.2f, rAmt) * 140.0f);

                std::vector<Point> rumbleGhostShape;
                for (const auto& pt : morphShape) {
                    rumbleGhostShape.push_back({ pt.x + rOffsetX, pt.y + rOffsetY });
                }

                // Render sub-bass background ghost echo
                d.filledPolygon(rumbleGhostShape, { .color = { 0, 180, 255, (uint8_t)(rAlpha * 0.35f) } });
                d.lines(rumbleGhostShape, { .color = { 0, 220, 255, rAlpha }, .thickness = 1 });
                d.line(rumbleGhostShape.back(), rumbleGhostShape.front(), { .color = { 0, 220, 255, rAlpha }, .thickness = 1 });
            }

            // FM Modulator Shell (Orbiting 5-point polygon shell around central shape for VIEW_KICK_BODY2 / FM Depth)
            float fmVal = std::clamp(kick.fmDepth.value / 100.0f, 0.0f, 1.0f);
            if (fmVal > 0.01f) {
                float rotAngle = animTime * (1.0f + fmVal * 8.0f);
                int numShellPts = 5;
                std::vector<Point> modShell;
                for (int i = 0; i < numShellPts; i++) {
                    float a = rotAngle + i * (6.28318f / numShellPts);
                    float radiusW = (halfW + 8.0f) + std::sin(a * 3.0f + animTime * 4.0f) * (fmVal * 10.0f);
                    float radiusH = (halfH + 8.0f) + std::cos(a * 2.0f + animTime * 3.0f) * (fmVal * 8.0f);
                    int mx = cx + (int)(std::cos(a) * radiusW);
                    int my = cy + (int)(std::sin(a) * radiusH);
                    modShell.push_back({ mx, my });
                }
                uint8_t shellAlpha = (uint8_t)(80 + fmVal * 165.0f);
                d.lines(modShell, { .color = { themeCol.r, themeCol.g, themeCol.b, shellAlpha }, .thickness = 1 });
                d.line(modShell.back(), modShell.front(), { .color = { themeCol.r, themeCol.g, themeCol.b, shellAlpha }, .thickness = 1 });
            }

            // Drive Overdrive & Saturation Gradient (Drive Idea B)
            float drv = std::clamp(kick.drive.value / 100.0f, 0.0f, 1.0f);
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

            // Fill & Outline for morphing geometry with Drive saturation
            d.filledPolygon(morphShape, { .color = { shapeStroke.r, shapeStroke.g, shapeStroke.b, fillAlpha } });
            d.lines(morphShape, { .color = shapeStroke, .thickness = strokeThickness });
            d.line(pBR, pBL, { .color = shapeStroke, .thickness = strokeThickness }); // ensure bottom edge closed

            // 3. Click Amount Noise Particles (flickering dot swarm)
            int dotCount = (int)(clickAmt * 0.45f);
            for (int i = 0; i < dotCount; i++) {
                float angle = i * 0.488f + animTime * (0.6f + (i % 4) * 0.3f);
                float dist = 16.0f + std::fmod((float)(i * 9 + animTime * 20.0f), 34.0f);
                int dotX = cx + (int)(std::cos(angle) * dist);
                int dotY = cy + (int)(std::sin(angle) * dist);
                dotX = std::clamp(dotX, graphX + 4, graphX + graphW - 4);
                dotY = std::clamp(dotY, graphY + 14, graphY + graphH - 14);
                uint8_t dotAlpha = (uint8_t)(110 + (i * 13 + (int)(animTime * 100)) % 145);
                d.pixel({ dotX, dotY }, Color { 255, 245, 170, dotAlpha });
                if (i % 2 == 0) {
                    d.pixel({ dotX + 1, dotY }, Color { 255, 255, 220, (uint8_t)(dotAlpha * 0.6f) });
                }
            }

            // 4. Frequency Sine Wave Ribbon
            int freqY = graphY + graphH - 10;
            std::vector<Point> freqWave;
            int innerW = graphW - 12;
            for (int gx = 0; gx < innerW; gx++) {
                float t = (float)gx / (float)innerW;
                float wave = std::sin(t * (freqHz * 0.22f) + animTime * (freqHz * 0.07f)) * (3.5f + (freqHz * 0.025f));
                freqWave.push_back({ graphX + 6 + gx, freqY + (int)wave });
            }
            d.lines(freqWave, { .color = { themeCol.r, themeCol.g, themeCol.b, 210 } });

            // Frequency readout overlay
            std::stringstream ssF;
            ssF << "FREQ: " << std::fixed << std::setprecision(1) << freqHz << " Hz";
            d.text({ graphX + 6, graphY + graphH - 11 }, ssF.str(), 8, { .color = { themeCol.r, themeCol.g, themeCol.b, 180 }, .font = &PoppinsLight_8 });

            break;
        }

        case VIEW_SYNTH1_PAGE1:
        case VIEW_SYNTH1_PAGE2:
        case VIEW_SYNTH1_PAGE3: {
            Color themeCol = getViewThemeColor(currentView);

            int cx = graphX + graphW / 2;
            int cy = graphY + (graphH / 2) - 2;
            int halfW = 28;
            int halfH = 22;

            // Parameters
            float pitchMidi = synth1.pitch.value; // 24 .. 72
            float wf = synth1.waveform.value;     // 0.0 .. 1.0
            float cutVal = synth1.cutoff.value;   // 0.02 .. 0.98
            float resVal = synth1.resonance.value; // 0.0 .. 0.95

            // Modulation Parameters
            float modD = synth1.modDepth.value * 0.01f; // -1.0 .. +1.0
            float modS = synth1.modSpeed.value * 0.01f; // 0.0 .. 1.0
            float lfoHz = 0.05f + (modS * modS * 39.95f);
            float lfoPhase = std::fmod(animTime * lfoHz * 0.5f, 1.0f);

            int routeIdx = std::clamp((int)std::round(synth1.modType.value), 0, Synth1::TOTAL_MOD_TYPES - 1);
            auto currentRoute = Synth1::modMatrix[routeIdx];

            float lfoVal = 0.0f;
            switch (currentRoute.source) {
            case Synth1::SRC_ENV:
                lfoVal = synth1PulseLevel;
                break;
            case Synth1::SRC_LFO_TRI:
                lfoVal = (lfoPhase < 0.5f) ? (4.0f * lfoPhase - 1.0f) : (3.0f - 4.0f * lfoPhase);
                break;
            case Synth1::SRC_LFO_SAW:
                lfoVal = 2.0f * lfoPhase - 1.0f;
                break;
            case Synth1::SRC_LFO_SH: {
                float stepIdx = std::floor(lfoPhase * 10.0f);
                lfoVal = std::sin(stepIdx * 17.13f + 1.5f);
                break;
            }
            }

            float modAmount = lfoVal * modD;

            // Apply Modulation to Waveform Morph if DST_MORPH
            if (currentRoute.dest == Synth1::DST_MORPH) {
                wf = std::clamp(wf + modAmount * 0.4f, 0.0f, 1.0f);
            }

            // 1. Sleek Filter Cutoff Position & Resonance Peak Laser Beam
            int innerW = graphW - 12;
            int cutX = graphX + 6 + (int)(std::clamp(cutVal, 0.02f, 0.98f) * innerW);

            // 2. Central Waveform Core (Morphing Geometry -> Noise Matrix Swarm)
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

            // Calculate Noise Morph Factor (0.0 when wf <= 0.666, 0.0..1.0 when wf > 0.666)
            float noiseFactor = (wf > 0.666f) ? std::clamp((wf - 0.666f) / 0.334f, 0.0f, 1.0f) : 0.0f;

            // Apply high-frequency mini earthquake tremor across all states (Triangle, Saw, Square, Noise)
            float baseJitterX = 1.6f + noiseFactor * 5.4f;
            float baseJitterY = 1.4f + noiseFactor * 4.6f;

            std::vector<Point> morphedShape;
            for (size_t i = 0; i < baseShape.size(); ++i) {
                float noiseSeed = animTime * 15.0f + i * 2.3f;
                int jitterX = (int)(std::sin(noiseSeed * 3.7f) * baseJitterX);
                int jitterY = (int)(std::cos(noiseSeed * 4.1f) * baseJitterY);
                morphedShape.push_back({ baseShape[i].x + jitterX, baseShape[i].y + jitterY });
            }

            // Synth 1 Trigger Pulse Decay & Expanding Waveform-Shaped Shockwave Echoes
            float sDecayRate = 12.0f / (std::clamp(synth1.release.value, 10.0f, 2000.0f) + 40.0f);
            synth1PulseLevel = std::max(0.0f, synth1PulseLevel - sDecayRate);

            if (synth1PulseLevel > 0.01f) {
                for (int r = 0; r < 3; r++) {
                    float pFactor = synth1PulseLevel - (r * 0.22f);
                    if (pFactor > 0.0f) {
                        float scale = 1.05f + (1.0f - pFactor) * 0.65f + r * 0.18f;
                        uint8_t pulseAlpha = (uint8_t)(pFactor * 160.0f);

                        std::vector<Point> pulseShape;
                        for (const auto& pt : morphedShape) {
                            int px = cx + (int)((pt.x - cx) * scale);
                            int py = cy + (int)((pt.y - cy) * scale);
                            pulseShape.push_back({ px, py });
                        }

                        d.lines(pulseShape, { .color = { themeCol.r, themeCol.g, themeCol.b, pulseAlpha }, .thickness = 1 });
                        d.line(pulseShape.back(), pulseShape.front(), { .color = { themeCol.r, themeCol.g, themeCol.b, pulseAlpha }, .thickness = 1 });
                    }
                }
            }

            // Spatial Delay Echo Ghosts (Delay Send Visual Feedback)
            float dlyAmt = std::clamp(synth1.delaySend.value * 0.01f, 0.0f, 1.0f);
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

                        d.filledPolygon(ghostShape, { .color = { themeCol.r, themeCol.g, themeCol.b, (uint8_t)(gAlpha * 0.25f) } });
                        d.lines(ghostShape, { .color = { themeCol.r, themeCol.g, themeCol.b, gAlpha }, .thickness = 1 });
                        d.line(ghostShape.back(), ghostShape.front(), { .color = { themeCol.r, themeCol.g, themeCol.b, gAlpha }, .thickness = 1 });
                    }
                }
            }

            // Opacity & Level Modulation (DST_LEVEL)
            float levelMod = (currentRoute.dest == Synth1::DST_LEVEL) ? std::clamp(1.0f + modAmount * 0.5f, 0.1f, 1.8f) : 1.0f;
            uint8_t lineAlpha = (uint8_t)(std::clamp(255.0f * (1.0f - noiseFactor * 0.85f) * levelMod, 10.0f, 255.0f));
            uint8_t fillAlpha = (uint8_t)(std::clamp(60.0f * (1.0f - noiseFactor) * levelMod, 5.0f, 180.0f));

            if (lineAlpha > 15) {
                d.filledPolygon(morphedShape, { .color = { themeCol.r, themeCol.g, themeCol.b, fillAlpha } });
                d.lines(morphedShape, { .color = { themeCol.r, themeCol.g, themeCol.b, lineAlpha }, .thickness = 1 });
                d.line(morphedShape.back(), morphedShape.front(), { .color = { themeCol.r, themeCol.g, themeCol.b, lineAlpha }, .thickness = 1 });
            }

            // Dynamic Noise Particle Swarm (flickering dot cloud as waveform morphs to noise)
            if (noiseFactor > 0.01f) {
                int particleCount = (int)(noiseFactor * 90.0f);
                for (int p = 0; p < particleCount; p++) {
                    float pAngle = p * 0.418f + animTime * (1.2f + (p % 5) * 0.4f);
                    float pDist = std::fmod((float)(p * 7 + animTime * 35.0f), 32.0f);
                    int px = cx + (int)(std::cos(pAngle) * pDist);
                    int py = cy + (int)(std::sin(pAngle) * (pDist * 0.7f));

                    px = std::clamp(px, graphX + 6, graphX + graphW - 6);
                    py = std::clamp(py, graphY + 12, graphY + graphH - 14);

                    uint8_t pAlpha = (uint8_t)((100 + (p * 17 + (int)(animTime * 120)) % 155) * noiseFactor);
                    Color pCol = (p % 3 == 0) ? Color { 255, 255, 255, pAlpha } : Color { 0, 255, 210, pAlpha };
                    d.pixel({ px, py }, pCol);
                    if (p % 4 == 0) {
                        d.pixel({ px + 1, py }, Color { pCol.r, pCol.g, pCol.b, (uint8_t)(pAlpha * 0.5f) });
                    }
                }
            }

            // Top-Left Rotating LFO Shape & Dotted Target Pointer (Subtle Gray Visual Feedback)
            if (std::abs(synth1.modDepth.value) > 1.0f) {
                int lfoCx = graphX + 16;
                int lfoCy = graphY + 16;

                // Base size scales slightly with modDepth, but stays small (< 10px radius)
                float lfoRadius = 4.5f + std::abs(modD) * 4.5f;

                // Moderate rotation speed proportional to modSpeed (not too fast)
                float spinHz = 0.15f + modS * 1.8f;
                float rotAngle = animTime * spinHz * 6.28318f;

                // Color palette: Subtle metallic gray / slate (non-prominent)
                uint8_t lfoAlpha = (uint8_t)(std::clamp(110.0f + std::abs(modD) * 120.0f, 60.0f, 230.0f));
                Color grayCol = { 155, 170, 185, lfoAlpha };
                Color dimGrayCol = { 120, 135, 150, (uint8_t)(lfoAlpha * 0.5f) };

                // Build spinning vertices based on LFO source shape
                std::vector<Point> iconPts;
                if (currentRoute.source == Synth1::SRC_LFO_TRI) {
                    // Triangle Shape
                    for (int i = 0; i < 3; i++) {
                        float a = rotAngle + i * (6.28318f / 3.0f) - 1.5708f;
                        iconPts.push_back({ lfoCx + (int)(std::cos(a) * lfoRadius), lfoCy + (int)(std::sin(a) * lfoRadius) });
                    }
                    d.lines(iconPts, { .color = grayCol, .thickness = 1 });
                    d.line(iconPts.back(), iconPts.front(), { .color = grayCol, .thickness = 1 });
                } else if (currentRoute.source == Synth1::SRC_LFO_SAW) {
                    // Sawtooth / Right Triangle Shape
                    float a0 = rotAngle;
                    float a1 = rotAngle + 2.1f;
                    float a2 = rotAngle + 4.2f;
                    iconPts = {
                        { lfoCx + (int)(std::cos(a0) * lfoRadius * 1.1f), lfoCy + (int)(std::sin(a0) * lfoRadius * 1.1f) },
                        { lfoCx + (int)(std::cos(a1) * lfoRadius * 0.7f), lfoCy + (int)(std::sin(a1) * lfoRadius * 0.7f) },
                        { lfoCx + (int)(std::cos(a2) * lfoRadius * 0.9f), lfoCy + (int)(std::sin(a2) * lfoRadius * 0.9f) }
                    };
                    d.lines(iconPts, { .color = grayCol, .thickness = 1 });
                    d.line(iconPts.back(), iconPts.front(), { .color = grayCol, .thickness = 1 });
                } else if (currentRoute.source == Synth1::SRC_LFO_SH) {
                    // Square / Diamond Shape (Sample & Hold)
                    for (int i = 0; i < 4; i++) {
                        float a = rotAngle + i * (6.28318f / 4.0f);
                        iconPts.push_back({ lfoCx + (int)(std::cos(a) * lfoRadius), lfoCy + (int)(std::sin(a) * lfoRadius) });
                    }
                    d.lines(iconPts, { .color = grayCol, .thickness = 1 });
                    d.line(iconPts.back(), iconPts.front(), { .color = grayCol, .thickness = 1 });
                } else {
                    // Envelope: Circle Shape with rotating pointer dot
                    d.circle({ lfoCx, lfoCy }, (int)lfoRadius, { .color = dimGrayCol });
                    int dotX = lfoCx + (int)(std::cos(rotAngle) * lfoRadius);
                    int dotY = lfoCy + (int)(std::sin(rotAngle) * lfoRadius);
                    d.pixel({ dotX, dotY }, grayCol);
                }

                // Determine Target Coordinates (dstX, dstY)
                int dstX = cx;
                int dstY = cy;

                if (currentRoute.dest == Synth1::DST_FILTER) {
                    dstX = cutX;
                    dstY = graphY + graphH - 22;
                } else if (currentRoute.dest == Synth1::DST_PITCH) {
                    dstX = cx;
                    dstY = graphY + graphH - 10;
                }

                // Draw Dotted Line from LFO Icon (lfoCx, lfoCy) to Target (dstX, dstY)
                float lineLen = std::hypot(dstX - lfoCx, dstY - lfoCy);
                int dashStep = 5;
                for (float dPos = 0.0f; dPos < lineLen; dPos += dashStep * 2) {
                    float t0 = dPos / lineLen;
                    float t1 = std::min(lineLen, dPos + dashStep) / lineLen;
                    int x0 = lfoCx + (int)((dstX - lfoCx) * t0);
                    int y0 = lfoCy + (int)((dstY - lfoCy) * t0);
                    int x1 = lfoCx + (int)((dstX - lfoCx) * t1);
                    int y1 = lfoCy + (int)((dstY - lfoCy) * t1);
                    d.line({ x0, y0 }, { x1, y1 }, { .color = dimGrayCol });
                }

                // Traveling Packet Dot along Dotted Line
                float pktProgress = std::fmod(animTime * spinHz * 0.8f, 1.0f);
                int px = lfoCx + (int)((dstX - lfoCx) * pktProgress);
                int py = lfoCy + (int)((dstY - lfoCy) * pktProgress);
                Color pktCol = { 210, 225, 240, 230 };
                d.pixel({ px, py }, pktCol);
                d.pixel({ px + 1, py }, pktCol);
            }

            // 3. Holographic SVF Spectral Wave Modulated by Filter Envelope (envAmt) & LFO (DST_FILTER)
            float envModAmt = synth1.envAmt.value;
            float filterModOffset = (currentRoute.dest == Synth1::DST_FILTER) ? modAmount * 0.35f : 0.0f;
            float modulatedCut = std::clamp(cutVal + (synth1PulseLevel * envModAmt * 0.45f) + filterModOffset, 0.02f, 0.98f);

            cutX = graphX + 6 + (int)(modulatedCut * innerW);
            int baseY = graphY + graphH - 10;
            int passbandH = 14 + (int)(synth1PulseLevel * envModAmt * 6.0f);

            float fMorph = synth1.filterMorph.value; // 0.0 (LP) -> 0.5 (BP) -> 1.0 (HP)

            std::vector<Point> svfPoints;
            int stepPx = 4;
            int peakY = baseY - passbandH;

            for (int gx = graphX + 6; gx <= graphX + graphW - 6; gx += stepPx) {
                float freqNorm = (float)(gx - (graphX + 6)) / (float)innerW;
                float dist = freqNorm - modulatedCut;

                // SVF Response Components (Low-Pass, Band-Pass, High-Pass)
                float lpResp = 1.0f / (1.0f + std::pow(freqNorm / std::max(0.04f, modulatedCut), 4.0f));
                float hpResp = 1.0f - lpResp;
                float bpResp = std::exp(-dist * dist * (25.0f + resVal * 50.0f));

                float baseCurve = 0.0f;
                if (fMorph < 0.5f) {
                    baseCurve = lpResp * (1.0f - fMorph * 2.0f) + bpResp * (fMorph * 2.0f);
                } else {
                    baseCurve = bpResp * (1.0f - (fMorph - 0.5f) * 2.0f) + hpResp * ((fMorph - 0.5f) * 2.0f);
                }

                float resonancePeak = bpResp * (resVal * 2.2f);
                float totalResp = baseCurve + resonancePeak;

                int drawH = (int)(totalResp * passbandH);
                drawH = std::clamp(drawH, 0, graphH - 20);
                int sy = baseY - drawH;
                svfPoints.push_back({ gx, sy });

                if (std::abs(gx - cutX) <= stepPx) {
                    peakY = sy;
                }
            }

            if (svfPoints.size() >= 2) {
                // Soft semi-transparent passband energy fill under the curve (dynamically sweeps with envAmt & LFO)
                uint8_t fillAlpha = (uint8_t)(25 + synth1PulseLevel * envModAmt * 35.0f);
                std::vector<Point> svfPoly = svfPoints;
                svfPoly.push_back({ graphX + graphW - 6, baseY });
                svfPoly.push_back({ graphX + 6, baseY });
                d.filledPolygon(svfPoly, { .color = { 0, 255, 220, fillAlpha } });
            }

            if (resVal > 0.01f) {
                for (int h = 0; h < 2; h++) {
                    float haloPulse = std::sin(animTime * 8.0f + h * 1.5f) * 1.5f;
                    int r = (int)(4 + h * 5 + resVal * 6.0f + haloPulse);
                    uint8_t hAlpha = (uint8_t)(std::clamp(180.0f * resVal - h * 50.0f, 0.0f, 255.0f));
                    d.circle({ cutX, peakY }, r, { .color = { 0, 255, 220, hAlpha } });
                }
            }

            // 4. Pitch & Frequency Ribbon + Readout Overlay + LFO Pitch Modulation (DST_PITCH)
            float pitchModOffset = (currentRoute.dest == Synth1::DST_PITCH) ? modAmount * 12.0f : 0.0f;
            float pitchHz = 440.0f * std::pow(2.0f, (pitchMidi + pitchModOffset - 69.0f) / 12.0f);

            int freqY = graphY + graphH - 10;
            std::vector<Point> pitchWave;
            float cycScale = (pitchHz / 110.0f) * 0.15f;
            for (int gx = 0; gx < innerW; gx++) {
                float t = (float)gx / (float)innerW;
                float wave = std::sin(t * (cycScale * 25.0f) + animTime * 4.0f) * 3.0f;
                pitchWave.push_back({ graphX + 6 + gx, freqY + (int)wave });
            }

            // Trailing Horizon Delay Echo Ripples
            if (dlyAmt > 0.01f) {
                int numRipples = (dlyAmt > 0.6f) ? 3 : ((dlyAmt > 0.3f) ? 2 : 1);
                for (int r = 1; r <= numRipples; r++) {
                    float phaseOffset = r * 0.55f;
                    uint8_t rippleAlpha = (uint8_t)(dlyAmt * (140.0f / (r * 1.25f)));
                    int rippleY = freqY - r * 2;

                    std::vector<Point> rippleWave;
                    for (int gx = 0; gx < innerW; gx++) {
                        float t = (float)gx / (float)innerW;
                        float wave = std::sin(t * (cycScale * 25.0f) + animTime * 4.0f - phaseOffset) * (3.0f - r * 0.6f);
                        rippleWave.push_back({ graphX + 6 + gx, rippleY + (int)wave });
                    }
                    d.lines(rippleWave, { .color = { themeCol.r, themeCol.g, themeCol.b, rippleAlpha }, .thickness = 1 });
                }
            }

            d.lines(pitchWave, { .color = { themeCol.r, themeCol.g, themeCol.b, 210 } });

            break;
        }

        case VIEW_SYNTH2_PAGE1:
        case VIEW_SYNTH2_PAGE2:
        case VIEW_SYNTH2_PAGE3: {
            Color syn2Col = Color { 190, 90, 255, 255 }; // Synth2 Purple
            // Synth 2 Chord Harmonics & Ambient Drift Spectrum
            d.text({ graphX + 4, graphY + 3 }, "SYNTH 2 CHORD HARMONICS & WAVETABLE", 8, { .color = syn2Col, .font = &PoppinsLight_8 });

            int innerW = graphW - 8;
            int innerH = graphH - 18;
            int baselineY = graphY + graphH - 6;

            int chordType = (int)std::round(synth2.chord.value);
            float chordFreqs[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            if (chordType == 1) { chordFreqs[1] = 1.5f; } // Fifth
            else if (chordType == 2) { chordFreqs[1] = 2.0f; } // Octave
            else if (chordType == 3) { chordFreqs[1] = 1.25f; chordFreqs[2] = 1.5f; chordFreqs[3] = 1.875f; } // Maj 7
            else if (chordType == 4) { chordFreqs[1] = 1.2f; chordFreqs[2] = 1.5f; chordFreqs[3] = 1.78f; } // Min 7
            else if (chordType == 5) { chordFreqs[1] = 1.33f; chordFreqs[2] = 1.5f; } // Sus 4

            int barW = 12;
            int numBars = 8;
            int spacing = (innerW - (numBars * barW)) / (numBars + 1);

            for (int b = 0; b < numBars; b++) {
                int bx = graphX + 4 + spacing + b * (barW + spacing);
                float hFactor = (b < 4) ? chordFreqs[b] : (1.0f + b * 0.25f);
                float hAmp = std::sin(b * 0.8f + synth2.modSpeed.value * 0.05f) * 0.2f + 0.6f;
                int bh = (int)(innerH * (0.3f + 0.4f / hFactor) * hAmp);
                bh = std::clamp(bh, 6, innerH);

                Color bColor = (b < 4) ? syn2Col : Color { 220, 140, 255, 200 };
                d.filledRect({ bx, baselineY - bh }, { barW, bh }, { .color = bColor });
            }

            // Wavetable Morph / Modulation flowing wave curve
            std::vector<Point> driftPoints;
            float wtMorphVal = (synth2.wavetable.value - 1.0f) / 63.0f;
            float mSpeed = synth2.modSpeed.value * 0.05f;
            float mDepth = std::abs(synth2.modDepth.value) * 0.12f;
            for (int gx = 0; gx < innerW; gx++) {
                float t = (float)gx / (float)innerW;
                float dWave = std::sin(t * 8.0f + mSpeed) * mDepth;
                float sWave = std::sin(t * 24.0f) * (wtMorphVal * 6.0f);
                driftPoints.push_back({ graphX + 4 + gx, graphY + 24 + (int)(dWave + sWave) });
            }
            d.lines(driftPoints, { .color = { 230, 130, 255, 220 } });
            break;
        }

        case VIEW_MASTER_PAGE1:
        case VIEW_MASTER_PAGE2: {
            Color mstCol = Color { 255, 195, 0, 255 }; // Master Gold

            int gridX = graphX + 4;
            int gridY = graphY + 14;
            int stepStride = 4;
            int cellW = 2; // 2px step box + 2px gap = distinct step separation
            int rowH = 10;

            const char* trackNames[3] = { "KICK", "SYN1", "SYN2" };
            Color trackColors[3] = { { 0, 180, 255, 255 }, { 0, 230, 180, 255 }, { 190, 90, 255, 255 } };

            // Draw Bar Headers (B1, B2, B3, B4) above steps
            for (int b = 0; b < 4; b++) {
                int bx = gridX + 28 + b * 16 * stepStride;
                std::string barLabel = "Bar " + std::to_string(b + 1);
                d.text({ bx, gridY - 1 }, barLabel, 8, { .color = mstCol, .font = &PoppinsLight_8 });
            }

            int tracksStartY = gridY + 10;

            for (int r = 0; r < 3; r++) {
                int ry = tracksStartY + r * (rowH + 3);

                // Dark row container background
                d.filledRect({ gridX + 26, ry - 1 }, { 64 * stepStride + 2, rowH + 2 }, { .color = { 14, 16, 22, 255 } });

                d.text({ gridX, ry + 1 }, trackNames[r], 8, { .color = trackColors[r], .font = &PoppinsLight_8 });

                for (int s = 0; s < 64; s++) {
                    int sx = gridX + 28 + s * stepStride;
                    bool isHit = false;
                    if (r == 0) isHit = seq.kickPattern[s];
                    else if (r == 1) isHit = seq.shouldTrigSynth((int)std::round(seq.synth1TrigMode), s, seq.kickPattern[s]);
                    else if (r == 2) isHit = seq.shouldTrigSynth((int)std::round(seq.synth2TrigMode), s, seq.kickPattern[s]);

                    Color cellBg;
                    if (s == seq.currentStep) {
                        cellBg = { 255, 255, 255, 255 }; // Playhead highlight
                    } else if (isHit) {
                        cellBg = trackColors[r]; // Active hit step in track color
                    } else if (s % 4 == 0) {
                        cellBg = { 55, 65, 82, 255 }; // On-beat step marker
                    } else {
                        cellBg = { 30, 34, 46, 255 }; // Off-beat step marker
                    }

                    // 2px wide step box with 2px gap
                    d.filledRect({ sx, ry }, { cellW, rowH }, { .color = cellBg });
                }
            }

            break;
        }

        default:
            break;
        }

        // 3. Bottom Performance Pad Status Bar (Y = 162..176)
        int barY = winH - 14;
        d.filledRect({ 0, barY }, { winW, 14 }, { .color = { 12, 14, 18, 255 } });
        d.line({ 0, barY }, { winW, barY }, { .color = { 40, 48, 64, 255 } });

        // Performance status indicators with part colors
        Color kickBadgeCol = kick.isBodyMuted ? Color { 120, 50, 50, 255 } : Color { 0, 180, 255, 255 };
        Color syn1BadgeCol = isSynth1Muted ? Color { 120, 50, 50, 255 } : Color { 0, 230, 180, 255 };
        Color syn2BadgeCol = isSynth2Muted ? Color { 120, 50, 50, 255 } : Color { 190, 90, 255, 255 };
        Color fillBadgeCol = seq.isMutatedFill ? Color { 255, 195, 0, 255 } : Color { 60, 65, 80, 255 };

        d.text({ 4, barY + 2 }, "[Z] KICK", 8, { .color = kickBadgeCol, .font = &PoppinsLight_8 });
        d.text({ 64, barY + 2 }, "[X] SYN1", 8, { .color = syn1BadgeCol, .font = &PoppinsLight_8 });
        d.text({ 124, barY + 2 }, "[C] SYN2", 8, { .color = syn2BadgeCol, .font = &PoppinsLight_8 });
        d.text({ 184, barY + 2 }, "[V] FILL", 8, { .color = fillBadgeCol, .font = &PoppinsLight_8 });

        std::stringstream bpmSs;
        bpmSs << (int)seq.bpm << " BPM";
        d.text({ winW - 50, barY + 2 }, bpmSs.str(), 8, { .color = { 255, 195, 0, 255 }, .font = &PoppinsLight_8 });
    }

    bool drawUI(Draw& d, int winW, int winH, bool& needFullRedraw)
    {
        // Clear screen with clean dark background
        d.filledRect({ 0, 0 }, { winW, winH }, { .color = { 18, 18, 24, 255 } });

        Color themeColor = getViewThemeColor(currentView);

        // Active Encoders Area (4 Columns across top)
        auto encs = getActiveEncoders();
        int colW = winW / 4;
        int rowY = 0;

        for (size_t i = 0; i < encs.size(); ++i) {
            int x = 1 + (int)i * colW;
            bool isActive = (activeEncoderHover == (int)i);
            drawParamCard(d, encs[i], x, rowY, colW, isActive, themeColor);
        }

        // View Title Badge (Top Center) in Part Theme Color
        d.textCentered({ winW / 2, 36 }, getViewTitle(), 8, { .color = themeColor, .font = &PoppinsLight_8 });

        // Visual Feedback in rest of screen (Y = 46..176)
        drawVisualFeedback(d, winW, winH);

        return true;
    }
};
