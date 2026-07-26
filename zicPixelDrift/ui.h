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
        return { p.label, &p.value, p.min, p.max, p.unit ? std::string(p.unit) : "", p.step };
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
        } else if (key == 's' || key == 'S') {
            synth1.trigger();
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
                fromParam(synth2.wavetable),
                fromParam(synth2.subDrone)
            };
            break;

        case VIEW_SYNTH2_PAGE2:
            encs = {
                fromParam(synth2.cutoff),
                fromParam(synth2.resonance),
                fromParam(synth2.shimmer),
                fromParam(synth2.delaySend)
            };
            break;

        case VIEW_SYNTH2_PAGE3:
            encs = {
                fromParam(synth2.attack),
                fromParam(synth2.release),
                fromParam(synth2.driftSpeed),
                fromParam(synth2.driftDepth)
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
            *(e.value) = newVal;

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
        case VIEW_KICK_BODY1: return "KICK 1: CLICK & VCO MORPH";
        case VIEW_KICK_BODY2: return "KICK 2: FM, DRIVE & RUMBLE";
        case VIEW_SYNTH1_PAGE1: return "SYNTH 1: TONE & FILTER";
        case VIEW_SYNTH1_PAGE2: return "SYNTH 1: ENV & DELAY SEND";
        case VIEW_SYNTH1_PAGE3: return "SYNTH 1: MOD & SYNTH MIX";
        case VIEW_SYNTH2_PAGE1: return "SYNTH 2: DRONE & CHORD";
        case VIEW_SYNTH2_PAGE2: return "SYNTH 2: AMBIENT SHIMMER";
        case VIEW_SYNTH2_PAGE3: return "SYNTH 2: DRIFT & FADES";
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
        if (!e.displayOptions.empty()) {
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
        case VIEW_KICK_BODY2: {
            Color kickCol = Color { 0, 180, 255, 255 }; // Electric Kick Blue
            // Kick Pitch Decay Curve & Oscillator Wave Preview
            d.text({ graphX + 4, graphY + 3 }, "KICK PITCH DECAY & DRIVE DISTORTION", 8, { .color = kickCol, .font = &PoppinsLight_8 });

            std::vector<Point> points;
            int innerW = graphW - 8;
            int innerH = graphH - 16;
            int centerY = graphY + 12 + innerH;

            float baseF = kick.baseFreq.value;
            float clickA = kick.clickAmt.value;
            float dur = kick.duration.value;
            float drv = kick.drive.value;

            for (int gx = 0; gx < innerW; gx++) {
                float t = (float)gx / (float)innerW;
                float freqEnv = baseF + (clickA * 350.0f) * std::exp(-t * (4.0f / (dur * 0.01f + 0.1f)));
                float normalizedY = (freqEnv - 20.0f) / 400.0f;
                normalizedY = std::clamp(normalizedY, 0.05f, 0.95f);

                float driveWave = std::sin(t * 30.0f * (1.0f + drv * 2.0f)) * (drv * 4.0f);
                int drawY = centerY - (int)(normalizedY * (innerH - 8)) + (int)driveWave;
                drawY = std::clamp(drawY, graphY + 14, graphY + graphH - 4);

                points.push_back({ graphX + 4 + gx, drawY });
            }

            Color c = kickCol;
            d.lines(points, { .color = c });
            c.a = 40;
            d.filledPolygon(points, { .color = c });

            // Rumble Sub Waveform line
            std::vector<Point> rumblePoints;
            float rumble = kick.rumbleAmt.value;
            for (int gx = 0; gx < innerW; gx++) {
                float t = (float)gx / (float)innerW;
                float rWave = std::sin(t * 12.0f * (1.0f + kick.rumbleGap.value * 2.0f)) * (rumble * 8.0f);
                rumblePoints.push_back({ graphX + 4 + gx, graphY + graphH - 8 + (int)rWave });
            }
            d.lines(rumblePoints, { .color = { 100, 200, 255, 200 } });
            break;
        }

        case VIEW_SYNTH1_PAGE1:
        case VIEW_SYNTH1_PAGE2:
        case VIEW_SYNTH1_PAGE3: {
            Color syn1Col = Color { 0, 230, 180, 255 }; // Synth1 Teal
            // Synth 1 Waveform Morph & SVF Filter Response Graph
            d.text({ graphX + 4, graphY + 3 }, "SYNTH 1 WAVEFORM MORPH & SVF FILTER CURVE", 8, { .color = syn1Col, .font = &PoppinsLight_8 });

            std::vector<Point> wavePoints;
            int innerW = graphW - 8;
            int innerH = (graphH - 16) / 2;
            int centerY = graphY + 14 + innerH / 2;

            float wf = synth1.waveform.value;
            for (int gx = 0; gx < innerW; gx++) {
                float phase = (float)gx / (float)(innerW / 4.0f);
                float ph = phase - std::floor(phase);

                float tri = 2.0f * std::abs(2.0f * (ph - std::floor(ph + 0.5f))) - 1.0f;
                float saw = 2.0f * ph - 1.0f;
                float sq = (ph < 0.5f) ? 0.8f : -0.8f;
                float oscSig = 0.0f;

                if (wf < 0.333f) oscSig = tri + (saw - tri) * (wf * 3.0f);
                else if (wf < 0.666f) oscSig = saw + (sq - saw) * ((wf - 0.333f) * 3.0f);
                else oscSig = sq;

                int drawY = centerY - (int)(oscSig * (innerH / 2.4f));
                wavePoints.push_back({ graphX + 4 + gx, drawY });
            }
            d.lines(wavePoints, { .color = syn1Col });

            // SVF Filter Response Curve
            std::vector<Point> filterPoints;
            int fCenterY = graphY + graphH - 6;
            float cut = synth1.cutoff.value;
            float res = synth1.resonance.value;
            float fm = synth1.filterMorph.value;

            for (int gx = 0; gx < innerW; gx++) {
                float freqNorm = (float)gx / (float)innerW;
                float dist = freqNorm - cut;
                float response = 0.0f;

                float lpResp = 1.0f / (1.0f + std::pow(freqNorm / std::max(0.05f, cut), 4.0f));
                float peak = std::exp(-dist * dist * (20.0f + res * 80.0f)) * (1.0f + res * 3.0f);
                float hpResp = 1.0f - lpResp;
                float bpResp = peak;

                if (fm < 0.5f) response = lpResp * (1.0f - fm * 2.0f) + bpResp * (fm * 2.0f);
                else response = bpResp * (1.0f - (fm - 0.5f) * 2.0f) + hpResp * ((fm - 0.5f) * 2.0f);

                response += peak * res * 0.4f;
                int drawY = fCenterY - (int)(std::clamp(response, 0.0f, 2.0f) * (innerH * 0.45f));
                filterPoints.push_back({ graphX + 4 + gx, drawY });
            }
            Color fc = syn1Col;
            d.lines(filterPoints, { .color = fc });
            fc.a = 30;
            d.filledPolygon(filterPoints, { .color = fc });

            // Cutoff vertical marker
            int cutX = graphX + 4 + (int)(cut * innerW);
            d.line({ cutX, graphY + 14 }, { cutX, graphY + graphH - 4 }, { .color = { 0, 255, 220, 180 } });
            break;
        }

        case VIEW_SYNTH2_PAGE1:
        case VIEW_SYNTH2_PAGE2:
        case VIEW_SYNTH2_PAGE3: {
            Color syn2Col = Color { 190, 90, 255, 255 }; // Synth2 Purple
            // Synth 2 Chord Harmonics & Ambient Drift Spectrum
            d.text({ graphX + 4, graphY + 3 }, "SYNTH 2 CHORD HARMONICS & AMBIENT DRIFT", 8, { .color = syn2Col, .font = &PoppinsLight_8 });

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
                float hAmp = std::sin(b * 0.8f + synth2.driftSpeed.value * 0.05f) * 0.2f + 0.6f;
                int bh = (int)(innerH * (0.3f + 0.4f / hFactor) * hAmp);
                bh = std::clamp(bh, 6, innerH);

                Color bColor = (b < 4) ? syn2Col : Color { 220, 140, 255, 200 };
                d.filledRect({ bx, baselineY - bh }, { barW, bh }, { .color = bColor });
            }

            // Shimmer / Drift flowing wave curve
            std::vector<Point> driftPoints;
            float shimmer = synth2.shimmer.value;
            for (int gx = 0; gx < innerW; gx++) {
                float t = (float)gx / (float)innerW;
                float dWave = std::sin(t * 8.0f + synth2.driftSpeed.value * 0.1f) * (synth2.driftDepth.value * 12.0f);
                float sWave = std::sin(t * 24.0f) * (shimmer * 6.0f);
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
