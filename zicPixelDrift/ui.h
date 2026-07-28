#pragma once

#include "audioWorker.h"
#include "draw/draw.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"
#include "kickBody.h"
#include "mixer.h"
#include "sequencer.h"
#include "spaceBackground.h"
#include "synth1.h"
#include "synth2.h"
#include "audio/Scatter.h"

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
    VIEW_SYNTH1_PAGE3,  // [W] Page 3
    VIEW_SYNTH2_PAGE1,  // [E] Page 1
    VIEW_SYNTH2_PAGE2,  // [E] Page 2
    VIEW_SYNTH2_PAGE3,  // [E] Page 3
    VIEW_MASTER_PAGE1,  // [R] Master Page
    VIEW_MASTER_PAGE2,  // [F] Sequencer Page
    VIEW_COUNT
};

enum EncoderControlFocus {
    FOCUS_VIEW,
    FOCUS_KICK_REPEAT,
    FOCUS_SCATTER_0,
    FOCUS_SCATTER_1,
    FOCUS_SCATTER_2,
    FOCUS_SCATTER_3
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
    EncoderControlFocus encoderFocus = FOCUS_VIEW;

    KickBody& kick;
    Synth1& synth1;
    Synth2& synth2;
    Sequencer& seq;
    Mixer& mixer;
    Scatter& scatter;
    AudioWorker& worker;

    SpaceBackground spaceBg;

    int activeEncoderHover = -1;
    bool isSynth1Muted = false;
    bool isSynth2Muted = false;

    bool isDKeyHeld = false;

    bool isPressedA = false;
    bool isPressedS = false;
    bool isPressedZ = false;
    bool isPressedX = false;
    bool isPressedC = false;
    bool isPressedV = false;

    bool isLatchedA = false;
    bool isLatchedS = false;
    bool isLatchedZ = false;
    bool isLatchedX = false;
    bool isLatchedC = false;
    bool isLatchedV = false;

    int getScatterModeForSlot(int slot) const
    {
        if (slot == 0) return 4; // Pad Z -> Mode 4 ([Z] CRUNCH)
        if (slot == 1) return 5; // Pad X -> Mode 5 ([X] DRIVE)
        if (slot == 2) return 2; // Pad C -> Mode 2 ([C] DIST - Kept Unchanged!)
        if (slot == 3) return 6; // Pad V -> Mode 6 ([V] ACID)
        return 2;
    }

    void syncPadStates()
    {
        kick.isBodyMuted = isLatchedA || isPressedA;

        seq.isKickRepeatActive = isLatchedS || isPressedS;

        scatter.setModeActive(4, isLatchedZ || isPressedZ);
        scatter.setModeActive(5, isLatchedX || isPressedX);
        scatter.setModeActive(2, isLatchedC || isPressedC);
        scatter.setModeActive(6, isLatchedV || isPressedV);

        updateFocusFallback();
    }

    int kickRepeatIdx = 1; // Default: 2 steps (8th notes)
    const int repeatRates[5] = { 1, 2, 4, 8, 16 };
    std::vector<std::string> kickRepeatDisplayStrings = { "1 step", "2 steps", "4 steps", "8 steps", "16 steps" };
    float dummyKickRepeatVal = 1.0f;

    void updateFocusFallback()
    {
        if (seq.isKickRepeatActive) {
            encoderFocus = FOCUS_KICK_REPEAT;
        } else if (scatter.anyActive()) {
            if (isLatchedZ || isPressedZ) encoderFocus = FOCUS_SCATTER_0;
            else if (isLatchedX || isPressedX) encoderFocus = FOCUS_SCATTER_1;
            else if (isLatchedC || isPressedC) encoderFocus = FOCUS_SCATTER_2;
            else if (isLatchedV || isPressedV) encoderFocus = FOCUS_SCATTER_3;
            else encoderFocus = FOCUS_VIEW;
        } else {
            encoderFocus = FOCUS_VIEW;
        }
    }

    // Visual feedback animation state
    float animTime = 0.0f;
    float kickPulseLevel = 0.0f;
    float synth1PulseLevel = 0.0f;
    float synth2PulseLevel = 0.0f;
    int lastSeqStep = -1;

    UiPixelDrift(AudioWorker& w)
        : kick(w.kickEngine)
        , synth1(w.synth1Engine)
        , synth2(w.synth2Engine)
        , seq(w.seqEngine)
        , mixer(w.mixerEngine)
        , scatter(w.scatter)
        , worker(w)
    {
    }

    UiPixelDrift(KickBody& k, Synth1& s1, Synth2& s2, Sequencer& sq, Mixer& m, Scatter& sc, AudioWorker& w)
        : kick(k)
        , synth1(s1)
        , synth2(s2)
        , seq(sq)
        , mixer(m)
        , scatter(sc)
        , worker(w)
    {
    }

    EncoderKnob fromParam(Param& p)
    {
        return { p.label, &p.value, p.min, p.max, p.unit ? std::string(p.unit) : "", p.step, {}, &p.string, &p };
    }

    void handleViewKey(char key, bool& needFullRedraw)
    {
        needFullRedraw = true;
        encoderFocus = FOCUS_VIEW;
        if (key == 'q' || key == 'Q') {
            currentView = (currentView == VIEW_KICK_BODY1) ? VIEW_KICK_BODY2 : VIEW_KICK_BODY1;
        } else if (key == 'w' || key == 'W') {
            if (currentView == VIEW_SYNTH1_PAGE1) {
                currentView = VIEW_SYNTH1_PAGE2;
            } else if (currentView == VIEW_SYNTH1_PAGE2) {
                currentView = VIEW_SYNTH1_PAGE3;
            } else {
                currentView = VIEW_SYNTH1_PAGE1;
            }
        } else if (key == 'e' || key == 'E') {
            if (currentView == VIEW_SYNTH2_PAGE1) {
                currentView = VIEW_SYNTH2_PAGE2;
            } else if (currentView == VIEW_SYNTH2_PAGE2) {
                currentView = VIEW_SYNTH2_PAGE3;
            } else {
                currentView = VIEW_SYNTH2_PAGE1;
            }
        } else if (key == 'r' || key == 'R') {
            currentView = VIEW_MASTER_PAGE1;
        } else if (key == 'f' || key == 'F') {
            currentView = VIEW_MASTER_PAGE2;
        }
    }

    void handlePerformancePad(char key, bool pressed, bool& needFullRedraw)
    {
        needFullRedraw = true;

        if (key == 'd' || key == 'D') {
            isDKeyHeld = pressed;
            if (pressed) {
                // When D is pressed, latch any performance pad currently being physically held down!
                if (isPressedA) isLatchedA = true;
                if (isPressedS) isLatchedS = true;
                if (isPressedZ) isLatchedZ = true;
                if (isPressedX) isLatchedX = true;
                if (isPressedC) isLatchedC = true;
                if (isPressedV) isLatchedV = true;
            }
            syncPadStates();
            return;
        }

        auto processPad = [&](bool& isPressed, bool& isLatched, EncoderControlFocus focus) {
            isPressed = pressed;
            if (pressed) {
                if (isDKeyHeld) {
                    isLatched = !isLatched;
                } else if (isLatched) {
                    isLatched = false;
                }
            }
            syncPadStates();
        };

        if (key == 'a' || key == 'A') {
            processPad(isPressedA, isLatchedA, FOCUS_VIEW);
            return;
        }

        if (key == 's' || key == 'S') {
            processPad(isPressedS, isLatchedS, FOCUS_KICK_REPEAT);
            return;
        }

        if (key == 'z' || key == 'Z') {
            processPad(isPressedZ, isLatchedZ, FOCUS_SCATTER_0);
            return;
        }

        if (key == 'x' || key == 'X') {
            processPad(isPressedX, isLatchedX, FOCUS_SCATTER_1);
            return;
        }

        if (key == 'c' || key == 'C') {
            processPad(isPressedC, isLatchedC, FOCUS_SCATTER_2);
            return;
        }

        if (key == 'v' || key == 'V') {
            processPad(isPressedV, isLatchedV, FOCUS_SCATTER_3);
            return;
        }
    }

    std::vector<EncoderKnob> getActiveEncoders()
    {
        if (encoderFocus == FOCUS_KICK_REPEAT) {
            dummyKickRepeatVal = (float)kickRepeatIdx;
            return {
                { "KICK REPEAT", &dummyKickRepeatVal, 0.0f, 4.0f, "", 1.0f, kickRepeatDisplayStrings },
                { "BPM", &seq.bpm, 40.0f, 260.0f, " bpm", 1.0f },
                { "GEN KICK", &seq.genKick, 0.0f, 1.0f, " %", 0.05f },
                fromParam(kick.baseFreq)
            };
        }

        if (encoderFocus >= FOCUS_SCATTER_0 && encoderFocus <= FOCUS_SCATTER_3) {
            int slot = encoderFocus - FOCUS_SCATTER_0;
            int mode = getScatterModeForSlot(slot);
            return {
                { scatter.getParamName(mode, 0), &scatter.params[mode][0], 0.0f, 1.0f, "", 0.01f },
                { scatter.getParamName(mode, 1), &scatter.params[mode][1], 0.0f, 1.0f, "", 0.01f },
                { scatter.getParamName(mode, 2), &scatter.params[mode][2], 0.0f, 1.0f, "", 0.01f },
                { scatter.getParamName(mode, 3), &scatter.params[mode][3], 0.0f, 1.0f, "", 0.01f }
            };
        }

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
                { "Mod Type", &synth2.modType.value, 0.0f, 16.0f, "", 1.0f, {
                    "ENV Cutoff", "ENV Pitch", "ENV Morph",
                    "LFO Tri Cut", "LFO Tri Pit", "LFO Tri Morph", "LFO Tri Lvl",
                    "LFO Saw Cut", "LFO Saw Pit", "LFO Saw Morph",
                    "LFO S&H Cut", "LFO S&H Pit", "LFO S&H Morph",
                    "Drift Cut", "Drift Pit", "Drift Morph", "Drift Level"
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
        needFullRedraw = true;

        if (encoderFocus == FOCUS_KICK_REPEAT) {
            if (encoderIdx == 0) {
                kickRepeatIdx = std::clamp(kickRepeatIdx + direction, 0, 4);
                seq.kickRepeatRate = repeatRates[kickRepeatIdx];
                dummyKickRepeatVal = (float)kickRepeatIdx;
                return;
            }
        } else if (encoderFocus >= FOCUS_SCATTER_0 && encoderFocus <= FOCUS_SCATTER_3) {
            int slot = encoderFocus - FOCUS_SCATTER_0;
            int mode = getScatterModeForSlot(slot);
            scatter.tweakParam(mode, encoderIdx, direction, false);
            return;
        }

        auto encs = getActiveEncoders();
        if (encoderIdx >= 0 && encoderIdx < (int)encs.size()) {
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
            return Color { 0, 195, 255, 255 }; // Electric Kick Blue

        case VIEW_SYNTH1_PAGE1:
        case VIEW_SYNTH1_PAGE2:
        case VIEW_SYNTH1_PAGE3:
            return Color { 0, 240, 190, 255 }; // Neon Synth1 Cyan/Teal

        case VIEW_SYNTH2_PAGE1:
        case VIEW_SYNTH2_PAGE2:
        case VIEW_SYNTH2_PAGE3:
            return Color { 215, 125, 255, 255 }; // Electric Synth2 Purple

        case VIEW_MASTER_PAGE1:
        case VIEW_MASTER_PAGE2:
            return Color { 255, 210, 0, 255 }; // Bright Master Gold

        default:
            return Color { 0, 210, 255, 255 };
        }
    }

    std::pair<int, int> getViewPageInfo() const
    {
        switch (currentView) {
        case VIEW_KICK_BODY1: return { 1, 2 };
        case VIEW_KICK_BODY2: return { 2, 2 };
        case VIEW_SYNTH1_PAGE1: return { 1, 3 };
        case VIEW_SYNTH1_PAGE2: return { 2, 3 };
        case VIEW_SYNTH1_PAGE3: return { 3, 3 };
        case VIEW_SYNTH2_PAGE1: return { 1, 3 };
        case VIEW_SYNTH2_PAGE2: return { 2, 3 };
        case VIEW_SYNTH2_PAGE3: return { 3, 3 };
        case VIEW_MASTER_PAGE1: return { 1, 1 };
        case VIEW_MASTER_PAGE2: return { 1, 1 };
        default: return { 1, 1 };
        }
    }

    std::string getViewTitle()
    {
        switch (currentView) {
        case VIEW_KICK_BODY1: return "KICK";
        case VIEW_KICK_BODY2: return "KICK";
        case VIEW_SYNTH1_PAGE1: return "SYNTH 1";
        case VIEW_SYNTH1_PAGE2: return "SYNTH 1";
        case VIEW_SYNTH1_PAGE3: return "SYNTH 1";
        case VIEW_SYNTH2_PAGE1: return "SYNTH 2";
        case VIEW_SYNTH2_PAGE2: return "SYNTH 2";
        case VIEW_SYNTH2_PAGE3: return "SYNTH 2";
        case VIEW_MASTER_PAGE1: return "MASTER";
        case VIEW_MASTER_PAGE2: return "SEQUENCER";
        default: return "zicPixelDrift";
        }
    }

    static constexpr int PARAM_ROW_H = 34;

    void drawParamCard(Draw& d, const EncoderKnob& e, int x, int y, int colW, bool isActiveHover, const Color& themeColor)
    {
        int cardH = PARAM_ROW_H - 2; // 32px height
        int cardW = colW - 2;

        Color cardBg = isActiveHover ? Color { 40, 52, 75, 255 } : Color { 28, 33, 46, 255 };
        Color cardBorder = isActiveHover ? themeColor : Color { 75, 88, 115, 255 };
        Color pColor = themeColor;

        d.filledRect({ x, y }, { cardW, cardH }, { .color = cardBg });
        d.rect({ x, y }, { cardW, cardH }, { .color = cardBorder });

        // Label (Bright Crisp Text)
        d.text({ x + 4, y + 2 }, e.label, 8, { .color = Color { 240, 245, 255, 255 }, .font = &PoppinsLight_8 });

        // Value text (High Contrast Blue-White)
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
        d.text({ x + 4, y + 14 }, valStr, 8, { .color = Color { 220, 235, 255, 255 }, .font = &PoppinsLight_8, .maxWidth = cardW - 6 });

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

                d.filledRect({ bX, bY }, { bW, 3 }, { .color = Color { 60, 68, 85, 255 } });
                for (int segIdx = 0; segIdx < segmentCount; segIdx++) {
                    int segX = bX + segIdx * (segW + gap);
                    d.filledRect({ segX, bY }, { segW, 3 }, { .color = (segIdx == currentIndex) ? pColor : Color { 90, 105, 130, 255 } });
                }
                return;
            }
        }

        if (e.minVal < 0.0f && e.maxVal > 0.0f) {
            int mid = bX + bW / 2;
            float maxAbs = std::max(std::abs(e.minVal), std::abs(e.maxVal));
            float normVal = *(e.value) / (maxAbs <= 0.0f ? 1.0f : maxAbs);
            int fw = (int)((bW / 2.0f) * normVal);

            d.filledRect({ bX, bY }, { bW, 3 }, { .color = Color { 60, 68, 85, 255 } });
            if (fw < 0) d.filledRect({ mid + fw, bY }, { std::abs(fw), 3 }, { .color = pColor });
            else d.filledRect({ mid, bY }, { fw, 3 }, { .color = pColor });
            d.filledRect({ mid, bY - 1 }, { 1, 5 }, { .color = Color { 180, 195, 220, 255 } });
        } else {
            d.filledRect({ bX, bY }, { bW, 3 }, { .color = Color { 60, 68, 85, 255 } });
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
            if (seq.shouldTrigSynth((int)std::round(seq.synth2TrigMode), lastSeqStep, seq.kickPattern[lastSeqStep])) {
                synth2PulseLevel = 1.0f;
            }
        }

        // 1. Render dynamic 3D Space Background in feedback region
        // Lets keep this commented out for now! Please do not uncomment.
        // spaceBg.updateAndDraw(d, seq.bpm, kick.drive.value, winW, winH, 0, feedbackY, feedbackW, feedbackH);

        // 2. Render Page-Specific Graph / Feedback Overlay
        int graphX = 8;
        int graphY = feedbackY;
        int graphW = winW - 16;
        int graphH = feedbackH - 8;

        Color themeColor = getViewThemeColor(currentView);

        // Solid graph box background + vibrant frame outline for high contrast
        d.filledRect({ graphX - 1, graphY - 1 }, { graphW + 2, graphH + 2 }, { .color = { 12, 14, 20, 255 } });
        d.rect({ graphX - 1, graphY - 1 }, { graphW + 2, graphH + 2 }, { .color = { themeColor.r, themeColor.g, themeColor.b, 220 } });

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
            d.lines(freqWave, { .color = { themeCol.r, themeCol.g, themeCol.b, 255 } });

            // Frequency readout overlay (Bright High-Contrast Text)
            std::stringstream ssF;
            ssF << "FREQ: " << std::fixed << std::setprecision(1) << freqHz << " Hz";
            d.text({ graphX + 6, graphY + graphH - 11 }, ssF.str(), 8, { .color = Color { 230, 240, 255, 255 }, .font = &PoppinsLight_8 });

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

            // Top-Left Rotating LFO Shape & Dotted Target Pointer (High-Contrast Slate Blue)
            if (std::abs(synth1.modDepth.value) > 1.0f) {
                int lfoCx = graphX + 16;
                int lfoCy = graphY + 16;

                // Base size scales slightly with modDepth, but stays small (< 10px radius)
                float lfoRadius = 4.5f + std::abs(modD) * 4.5f;

                // Moderate rotation speed proportional to modSpeed (not too fast)
                float spinHz = 0.15f + modS * 1.8f;
                float rotAngle = animTime * spinHz * 6.28318f;

                // Color palette: High-contrast slate blue / cyan-gray
                Color grayCol = { 190, 210, 235, 255 };
                Color dimGrayCol = { 135, 150, 175, 255 };

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

                // Traveling Packet Dot along Dotted Line (Pure White)
                float pktProgress = std::fmod(animTime * spinHz * 0.8f, 1.0f);
                int px = lfoCx + (int)((dstX - lfoCx) * pktProgress);
                int py = lfoCy + (int)((dstY - lfoCy) * pktProgress);
                Color pktCol = { 255, 255, 255, 255 };
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

            d.lines(pitchWave, { .color = { themeCol.r, themeCol.g, themeCol.b, 255 } });

            break;
        }

        case VIEW_SYNTH2_PAGE1:
        case VIEW_SYNTH2_PAGE2:
        case VIEW_SYNTH2_PAGE3: {
            Color syn2Col = Color { 190, 90, 255, 255 }; // Synth2 Purple

            int innerW = graphW - 12;
            int innerH = graphH - 16;

            // Parameters
            float pitchMidi = synth2.pitch.value;
            float wtPos = std::clamp((synth2.wavetable.value - 1.0f) / 63.0f, 0.0f, 1.0f);
            float cutVal = synth2.cutoff.value;
            float resVal = synth2.resonance.value;
            float modD = synth2.modDepth.value * 0.01f;
            float modS = synth2.modSpeed.value * 0.01f;

            // Optimized 3D Perspective Real Wavetable Waterfall Mesh
            int activeFrameIdx = std::clamp((int)std::round(synth2.wavetable.value - 1.0f), 0, 63);

            // Keyframe depth slices subset for high performance
            std::vector<int> sliceFrames = { 0, 8, 16, 24, 32, 40, 48, 56, 63 };

            // Ensure the currently selected activeFrameIdx is ALWAYS included!
            if (std::find(sliceFrames.begin(), sliceFrames.end(), activeFrameIdx) == sliceFrames.end()) {
                sliceFrames.push_back(activeFrameIdx);
                std::sort(sliceFrames.begin(), sliceFrames.end());
            }

            int numSlices = (int)sliceFrames.size();
            int baseSliceW = innerW - 36;
            int originX = graphX + 6;
            int originY = graphY + graphH - 32;

            std::vector<std::vector<Point>> allSlicePoints(numSlices);

            // Calculate Real 3D Slice Curves from Back to Front
            for (int i = 0; i < numSlices; i++) {
                int frameIdx = sliceFrames[i];
                float z = (float)frameIdx / 63.0f; // 0.0 at back (slice 0), 1.0 at front (slice 63)

                // Perspective projection offset
                int sliceOffsetX = (int)((1.0f - z) * 36.0f);
                int sliceOffsetY = (int)(-(1.0f - z) * 24.0f);
                int sliceW = (int)(baseSliceW * (0.72f + z * 0.28f));
                int sliceH = (int)(13.0f * (0.55f + z * 0.45f));

                int sx0 = originX + sliceOffsetX;
                int sy0 = originY + sliceOffsetY;

                int ptsCount = 28;
                // float driftPhase = animTime * (0.15f + modS * 0.45f);
                float driftPhase = animTime * 0.5f;
                for (int p = 0; p <= ptsCount; p++) {
                    float t = (float)p / (float)ptsCount;
                    float tDrift = std::fmod(t + driftPhase, 1.0f);

                    // Read REAL audio sample from loaded wavetable memory buffer with kinetic phase drift!
                    float rawWave = synth2.wt.getSampleAt(frameIdx, tDrift);

                    // Filter Cutoff Dampening
                    float freqNorm = t;
                    float filterDamp = 1.0f / (1.0f + std::pow(freqNorm / std::max(0.04f, cutVal), 3.0f));

                    float waveH = rawWave * filterDamp * sliceH;
                    if (frameIdx == activeFrameIdx && std::abs(modD) > 0.05f) {
                        // waveH += std::sin(t * 16.0f + animTime * 8.0f) * (modD * 3.5f);
                        waveH += std::sin(t * 16.0f + animTime * 8.0f) * (modD * 3.5f) * (modS * 2.0f);
                    }

                    int px = sx0 + (int)(t * sliceW);
                    int py = sy0 - (int)waveH;
                    allSlicePoints[i].push_back({ px, py });
                }
            }

            // Synth 2 Trigger Pulse Decay
            float s2DecayRate = 12.0f / (std::clamp(synth2.release.value, 10.0f, 8000.0f) + 40.0f);
            synth2PulseLevel = std::max(0.0f, synth2PulseLevel - s2DecayRate);

            // Render Cosmic Particle Dots Behind the 3D Wavetable Mesh (Origin: Top-Back Horizon)
            if (synth2PulseLevel > 0.01f) {
                int horizonX = originX + 36 + (int)(baseSliceW * 0.72f * 0.5f);
                int horizonY = originY - 24;

                float expandProgress = 1.0f - synth2PulseLevel; // 0.0 on trigger -> 1.0 on decay end

                const int numDots = 44;
                for (int k = 0; k < numDots; k++) {
                    // Spread outward and upward into background space behind the wavetable
                    float angle = -0.1f - (float)k * (3.0f / numDots) + std::sin(k * 2.3f) * 0.25f;
                    float maxDistX = (graphW * 0.46f) * (0.35f + std::fmod(k * 0.47f, 0.65f));
                    float maxDistY = (graphH * 0.55f) * (0.35f + std::fmod(k * 0.31f, 0.65f));

                    int px = horizonX + (int)(std::cos(angle) * maxDistX * expandProgress);
                    int py = horizonY + (int)(std::sin(angle) * maxDistY * expandProgress);

                    if (px >= graphX + 2 && px <= graphX + graphW - 3 && py >= graphY + 2 && py <= graphY + graphH - 3) {
                        float shimmer = std::sin(animTime * 20.0f + k * 2.1f);
                        uint8_t pAlpha = (uint8_t)(synth2PulseLevel * std::clamp(140.0f + shimmer * 95.0f, 0.0f, 255.0f));

                        Color pCol;
                        if (k % 4 == 0) pCol = Color { 255, 235, 255, pAlpha };        // Stardust White
                        else if (k % 4 == 1) pCol = Color { 240, 130, 255, pAlpha };   // Neon Magenta
                        else if (k % 4 == 2) pCol = Color { 140, 220, 255, pAlpha };   // Cosmic Cyan
                        else pCol = Color { 190, 110, 240, pAlpha };                  // Deep Purple

                        d.pixel({ px, py }, pCol);
                    }
                }
            }

            // Render Perspective Connecting Lattice Wireframe Lines across keyframes
            for (int i = 0; i < numSlices - 1; i++) {
                float z = (float)sliceFrames[i] / 63.0f;
                uint8_t meshAlpha = (uint8_t)(70 + z * 125.0f);
                Color meshCol = Color { 160, 90, 225, meshAlpha };

                size_t step = 4;
                for (size_t p = 0; p < allSlicePoints[i].size(); p += step) {
                    d.line(allSlicePoints[i][p], allSlicePoints[i + 1][p], { .color = meshCol });
                }
            }

            // Render 3D Slice Curves (Back-to-Front)
            for (int i = 0; i < numSlices; i++) {
                int frameIdx = sliceFrames[i];
                float z = (float)frameIdx / 63.0f;
                const auto& slicePts = allSlicePoints[i];

                if (frameIdx == activeFrameIdx) {
                    // Active Slice: Soft Neon Magenta Energy Fill + Bright Glowing Outline
                    int sliceW = (int)(baseSliceW * (0.72f + z * 0.28f));
                    int sliceOffsetX = (int)((1.0f - z) * 36.0f);
                    int sliceOffsetY = (int)(-(1.0f - z) * 24.0f);
                    int sx0 = originX + sliceOffsetX;
                    int sy0 = originY + sliceOffsetY;

                    std::vector<Point> fillPoly = slicePts;
                    fillPoly.push_back({ sx0 + sliceW, sy0 });
                    fillPoly.push_back({ sx0, sy0 });
                    d.filledPolygon(fillPoly, { .color = { 220, 110, 255, 75 } });

                    // Glowing Active Slice Lines
                    d.lines(slicePts, { .color = { 255, 195, 255, 255 }, .thickness = 1 });
                } else {
                    // Inactive Keyframe Slices: Fading Semi-Transparent Lines
                    uint8_t alpha = (uint8_t)(90 + z * 115);
                    Color depthCol = (frameIdx < activeFrameIdx) ? Color { 150, 80, 210, alpha } : Color { 205, 120, 255, alpha };
                    d.lines(slicePts, { .color = depthCol, .thickness = 1 });
                }
            }

            // Pitch & Frequency Ribbon at the Base
            float pitchHz = 440.0f * std::pow(2.0f, (pitchMidi - 69.0f) / 12.0f);
            int freqY = graphY + graphH - 8;
            std::vector<Point> pitchWave;
            float cycScale = (pitchHz / 110.0f) * 0.15f;
            for (int gx = 0; gx < innerW; gx++) {
                float t = (float)gx / (float)innerW;
                float wave = std::sin(t * (cycScale * 25.0f) + animTime * 4.0f) * 2.5f;
                pitchWave.push_back({ graphX + 6 + gx, freqY + (int)wave });
            }
            d.lines(pitchWave, { .color = { syn2Col.r, syn2Col.g, syn2Col.b, 255 } });

            break;
        }

        case VIEW_MASTER_PAGE1:
        case VIEW_MASTER_PAGE2: {
            Color mstCol = Color { 255, 210, 0, 255 }; // Master Gold

            int gridX = graphX + 4;
            int gridY = graphY + 14;
            int stepStride = 4;
            int cellW = 2; // 2px step box + 2px gap = distinct step separation
            int rowH = 10;

            const char* trackNames[3] = { "KICK", "SYN1", "SYN2" };
            Color trackColors[3] = { { 0, 195, 255, 255 }, { 0, 240, 190, 255 }, { 215, 125, 255, 255 } };

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
                d.filledRect({ gridX + 26, ry - 1 }, { 64 * stepStride + 2, rowH + 2 }, { .color = { 18, 22, 32, 255 } });

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
                        cellBg = { 90, 105, 130, 255 }; // On-beat step marker
                    } else {
                        cellBg = { 55, 62, 80, 255 }; // Off-beat step marker
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
        int barY = winH - 16;
        d.filledRect({ 0, barY }, { winW, 14 }, { .color = { 14, 17, 24, 255 } });

        // Performance status indicators with part colors & Scatter FX [Z X C V]
        Color inactivePadCol = Color { 140, 155, 178, 255 };
        Color kickBadgeCol = (isLatchedA || isPressedA) ? Color { 0, 195, 255, 255 } : inactivePadCol;
        Color sRptBadgeCol = (isLatchedS || isPressedS) ? Color { 255, 210, 0, 255 } : inactivePadCol;
        Color holdBadgeCol = (isDKeyHeld || isLatchedA || isLatchedS || isLatchedZ || isLatchedX || isLatchedC || isLatchedV) ? Color { 255, 185, 0, 255 } : inactivePadCol;

        Color sctZCol = (isLatchedZ || isPressedZ) ? Color { 255, 130, 50, 255 } : inactivePadCol;
        Color sctXCol = (isLatchedX || isPressedX) ? Color { 255, 215, 40, 255 } : inactivePadCol;
        Color sctCCol = (isLatchedC || isPressedC) ? Color { 50, 235, 130, 255 } : inactivePadCol;
        Color sctVCol = (isLatchedV || isPressedV) ? Color { 230, 90, 255, 255 } : inactivePadCol;

        d.text({ 4, barY }, "MUTE", 8, { .color = kickBadgeCol, .font = &PoppinsLight_8 });
        d.text({ 46, barY }, "RPT", 8, { .color = sRptBadgeCol, .font = &PoppinsLight_8 });
        d.text({ 84, barY }, "HOLD", 8, { .color = holdBadgeCol, .font = &PoppinsLight_8 });
        d.text({ 126, barY }, "CRUNCH", 8, { .color = sctZCol, .font = &PoppinsLight_8 });
        d.text({ 178, barY }, "DRIVE", 8, { .color = sctXCol, .font = &PoppinsLight_8 });
        d.text({ 224, barY }, "DIST", 8, { .color = sctCCol, .font = &PoppinsLight_8 });
        d.text({ 266, barY }, "ACID", 8, { .color = sctVCol, .font = &PoppinsLight_8 });
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

        // View Title Badge (Top Center) in Part Theme Color with High-Contrast Background
        d.textCentered({ winW / 2, 34 }, getViewTitle(), 8, { .color = themeColor, .font = &PoppinsLight_8 });

        // Draw Page Indicator Dots on the right side of the View Title Badge
        auto [pageIdx, totalPages] = getViewPageInfo();
        if (totalPages > 1) {
            int dotW = 5;
            int dotH = 3;
            int gap = 2;
            int totalDotsW = totalPages * dotW + (totalPages - 1) * gap;
            int dotsX = winW - 10 - totalDotsW;
            int dotsY = 36;
            for (int p = 0; p < totalPages; p++) {
                Color dotCol = (p + 1 == pageIdx) ? themeColor : Color { 60, 72, 95, 255 };
                d.filledRect({ dotsX + p * (dotW + gap), dotsY }, { dotW, dotH }, { .color = dotCol });
            }
        }
        // Visual Feedback in rest of screen (Y = 46..176)
        drawVisualFeedback(d, winW, winH);

        return true;
    }
};
