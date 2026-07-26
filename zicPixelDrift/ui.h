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
    VIEW_KICK_BODY1, // [Q] Page 1
    VIEW_KICK_BODY2, // [Q] Page 2
    VIEW_SYNTH1_PAGE1, // [W] Page 1
    VIEW_SYNTH1_PAGE2, // [W] Page 2
    VIEW_SYNTH1_PAGE3, // [E] Page 1
    VIEW_SYNTH2_PAGE1, // [E] Page 2
    VIEW_SYNTH2_PAGE2, // [R] Page 1
    VIEW_SYNTH2_PAGE3, // [R] Page 2
    VIEW_MASTER_PAGE1, // [F] Page 1
    VIEW_MASTER_PAGE2, // [F] Page 2
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
                { "Mod Type", &synth1.modType.value, 0.0f, 11.0f, "", 1.0f, { "ENV Cutoff", "ENV Pitch", "ENV Wave", "LFO Tri Cut", "LFO Tri Pit", "LFO Tri Wave", "LFO Tri Lvl", "LFO Saw Cut", "LFO Saw Pit", "LFO Saw Wave", "LFO S&H Cut", "LFO S&H Pit" } },
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

    std::string getViewTitle()
    {
        switch (currentView) {
        case VIEW_KICK_BODY1:
            return "KICK 1: CLICK & VCO MORPH";
        case VIEW_KICK_BODY2:
            return "KICK 2: FM, DRIVE & RUMBLE";
        case VIEW_SYNTH1_PAGE1:
            return "SYNTH 1: TONE & FILTER";
        case VIEW_SYNTH1_PAGE2:
            return "SYNTH 1: ENV & DELAY SEND";
        case VIEW_SYNTH1_PAGE3:
            return "SYNTH 1: MOD & SYNTH MIX";
        case VIEW_SYNTH2_PAGE1:
            return "SYNTH 2: DRONE & CHORD";
        case VIEW_SYNTH2_PAGE2:
            return "SYNTH 2: AMBIENT SHIMMER";
        case VIEW_SYNTH2_PAGE3:
            return "SYNTH 2: DRIFT & FADES";
        case VIEW_MASTER_PAGE1:
            return "MASTER: VOL, MIX & DELAY";
        case VIEW_MASTER_PAGE2:
            return "SEQUENCER: BPM & KICK GEN";
        default:
            return "zicPixelDrift";
        }
    }

    static constexpr int PARAM_ROW_H = 34;

    void drawParamCard(Draw& d, const EncoderKnob& e, int x, int y, int colW, bool isActiveHover)
    {
        int cardH = PARAM_ROW_H - 2; // 32px height
        int cardW = colW - 2;

        Color cardBg = isActiveHover ? Color { 28, 35, 48, 255 } : Color { 20, 22, 30, 255 };
        Color cardBorder = isActiveHover ? Color { 0, 220, 255, 255 } : Color { 45, 52, 68, 255 };
        Color pColor = Color { 0, 200, 255, 255 };

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
        spaceBg.updateAndDraw(d, seq.bpm, kick.drive.value, winW, winH, 0, feedbackY, feedbackW, feedbackH);

        // 2. Render Page-Specific Graph / Feedback Overlay
        int graphX = 8;
        int graphY = feedbackY + 4;
        int graphW = winW - 16;
        int graphH = feedbackH - 8;

        // Overlay semi-transparent visualizer box frame
        d.rect({ graphX - 1, graphY - 1 }, { graphW + 2, graphH + 2 }, { .color = { 0, 180, 255, 120 } });

        switch (currentView) {
        case VIEW_KICK_BODY1:
        case VIEW_KICK_BODY2: {
            // Kick Pitch Decay Curve & Oscillator Wave Preview
            d.text({ graphX + 4, graphY + 3 }, "KICK PITCH DECAY & DRIVE DISTORTION", 8, { .color = { 0, 230, 255, 255 }, .font = &PoppinsLight_8 });

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
                // Envelope drop from (baseF + clickA*350) to baseF
                float freqEnv = baseF + (clickA * 350.0f) * std::exp(-t * (4.0f / (dur * 0.01f + 0.1f)));
                float normalizedY = (freqEnv - 20.0f) / 400.0f;
                normalizedY = std::clamp(normalizedY, 0.05f, 0.95f);

                // Add subtle drive sine wave modulation to curve
                float driveWave = std::sin(t * 30.0f * (1.0f + drv * 2.0f)) * (drv * 4.0f);
                int drawY = centerY - (int)(normalizedY * (innerH - 8)) + (int)driveWave;
                drawY = std::clamp(drawY, graphY + 14, graphY + graphH - 4);

                points.push_back({ graphX + 4 + gx, drawY });
            }

            Color c = { 0, 240, 255, 255 };
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
            d.lines(rumblePoints, { .color = { 255, 100, 50, 180 } });
            break;
        }

        case VIEW_SYNTH1_PAGE1:
        case VIEW_SYNTH1_PAGE2:
        case VIEW_SYNTH1_PAGE3: {
            // Synth 1 Waveform Morph & SVF Filter Response Graph
            d.text({ graphX + 4, graphY + 3 }, "SYNTH 1 WAVEFORM MORPH & SVF FILTER CURVE", 8, { .color = { 0, 230, 255, 255 }, .font = &PoppinsLight_8 });

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
            d.lines(wavePoints, { .color = { 0, 255, 180, 255 } });

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

                // LP / BP / HP curve blend
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
            Color fc = { 255, 180, 0, 220 };
            d.lines(filterPoints, { .color = fc });
            fc.a = 30;
            d.filledPolygon(filterPoints, { .color = fc });

            // Cutoff vertical marker
            int cutX = graphX + 4 + (int)(cut * innerW);
            d.line({ cutX, graphY + 14 }, { cutX, graphY + graphH - 4 }, { .color = { 255, 220, 0, 150 } });
            break;
        }

        case VIEW_SYNTH2_PAGE1:
        case VIEW_SYNTH2_PAGE2:
        case VIEW_SYNTH2_PAGE3: {
            // Synth 2 Chord Harmonics & Ambient Drift Spectrum
            d.text({ graphX + 4, graphY + 3 }, "SYNTH 2 CHORD HARMONICS & AMBIENT DRIFT", 8, { .color = { 0, 230, 255, 255 }, .font = &PoppinsLight_8 });

            int innerW = graphW - 8;
            int innerH = graphH - 18;
            int baselineY = graphY + graphH - 6;

            int chordType = (int)std::round(synth2.chord.value);
            float chordFreqs[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            if (chordType == 1) {
                chordFreqs[1] = 1.5f;
            } // Fifth
            else if (chordType == 2) {
                chordFreqs[1] = 2.0f;
            } // Octave
            else if (chordType == 3) {
                chordFreqs[1] = 1.25f;
                chordFreqs[2] = 1.5f;
                chordFreqs[3] = 1.875f;
            } // Maj 7
            else if (chordType == 4) {
                chordFreqs[1] = 1.2f;
                chordFreqs[2] = 1.5f;
                chordFreqs[3] = 1.78f;
            } // Min 7
            else if (chordType == 5) {
                chordFreqs[1] = 1.33f;
                chordFreqs[2] = 1.5f;
            } // Sus 4

            int barW = 12;
            int numBars = 8;
            int spacing = (innerW - (numBars * barW)) / (numBars + 1);

            for (int b = 0; b < numBars; b++) {
                int bx = graphX + 4 + spacing + b * (barW + spacing);
                float hFactor = (b < 4) ? chordFreqs[b] : (1.0f + b * 0.25f);
                float hAmp = std::sin(b * 0.8f + synth2.driftSpeed.value * 0.05f) * 0.2f + 0.6f;
                int bh = (int)(innerH * (0.3f + 0.4f / hFactor) * hAmp);
                bh = std::clamp(bh, 6, innerH);

                Color bColor = (b < 4) ? Color { 0, 230, 255, 255 } : Color { 180, 100, 255, 200 };
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
            d.lines(driftPoints, { .color = { 255, 120, 220, 220 } });
            break;
        }

        case VIEW_MASTER_PAGE1:
        case VIEW_MASTER_PAGE2: {
            // 16-Step Euclidean Sequencer Grid & Master Bus VU
            d.text({ graphX + 4, graphY + 3 }, "16-STEP EUCLIDEAN SEQUENCER MATRIX", 8, { .color = { 0, 230, 255, 255 }, .font = &PoppinsLight_8 });

            int gridX = graphX + 4;
            int gridY = graphY + 14;
            int stepW = (graphW - 40) / 16;
            int rowH = 10;

            const char* trackNames[3] = { "KICK", "SYN1", "SYN2" };
            Color trackColors[3] = { { 255, 100, 50, 255 }, { 0, 255, 180, 255 }, { 180, 100, 255, 255 } };

            for (int r = 0; r < 3; r++) {
                int ry = gridY + r * (rowH + 3);
                d.text({ gridX, ry + 1 }, trackNames[r], 8, { .color = trackColors[r], .font = &PoppinsLight_8 });

                for (int s = 0; s < 16; s++) {
                    int sx = gridX + 28 + s * stepW;
                    bool isHit = false;
                    if (r == 0) isHit = seq.kickPattern[s];
                    else if (r == 1) isHit = seq.shouldTrigSynth((int)std::round(seq.synth1TrigMode), s, seq.kickPattern[s]);
                    else if (r == 2) isHit = seq.shouldTrigSynth((int)std::round(seq.synth2TrigMode), s, seq.kickPattern[s]);

                    Color cellBg = isHit ? trackColors[r] : Color { 35, 40, 50, 255 };
                    if (s == seq.currentStep) {
                        cellBg = { 255, 255, 255, 255 }; // Playhead highlight
                    }
                    d.filledRect({ sx, ry }, { stepW - 2, rowH }, { .color = cellBg });
                }
            }

            // Master Volume / Delay Feedback Rings & VU Level
            int vuY = gridY + 3 * (rowH + 3) + 2;
            d.text({ gridX, vuY }, "MASTER BUS & DELAY FEEDBACK", 8, { .color = { 200, 210, 225, 255 }, .font = &PoppinsLight_8 });

            int vuBarW = graphW - 36;
            int vuBarH = 5;
            int vuX = gridX + 28;
            int vuYPos = vuY + 10;

            float masterVol = mixer.volume;
            d.filledRect({ vuX, vuYPos }, { vuBarW, vuBarH }, { .color = { 30, 35, 45, 255 } });
            d.filledRect({ vuX, vuYPos }, { (int)(vuBarW * masterVol), vuBarH }, { .color = { 0, 230, 255, 255 } });
            d.rect({ vuX, vuYPos }, { vuBarW, vuBarH }, { .color = { 70, 85, 110, 255 } });
            break;
        }

        default:
            break;
        }
    }

    bool drawUI(Draw& d, int winW, int winH, bool& needFullRedraw)
    {
        // Clear screen with clean dark background
        d.filledRect({ 0, 0 }, { winW, winH }, { .color = { 18, 18, 24, 255 } });

        // Active Encoders Area (4 Columns across top)
        auto encs = getActiveEncoders();
        int colW = winW / 4;
        int rowY = 0;

        for (size_t i = 0; i < encs.size(); ++i) {
            int x = 1 + (int)i * colW;
            bool isActive = (activeEncoderHover == (int)i);
            drawParamCard(d, encs[i], x, rowY, colW, isActive);
        }

        // View Title Badge (Top Center)
        d.textCentered({ winW / 2, 36 }, getViewTitle(), 8, { .color = { 0, 220, 255, 255 }, .font = &PoppinsLight_8 });

        // Visual Feedback in rest of screen (Y = 46..176)
        drawVisualFeedback(d, winW, winH);

        return true;
    }
};
