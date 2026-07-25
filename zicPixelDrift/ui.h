#pragma once

#include "draw/draw.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"
#include "kickBody.h"
#include "mixer.h"
#include "sequencer.h"
#include "synth1.h"
#include "synth2.h"

#include <algorithm>
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

    std::string getViewTitle()
    {
        switch (currentView) {
        case VIEW_KICK_BODY1: return "[Q] KICK 1: CLICK & VCO MORPH";
        case VIEW_KICK_BODY2: return "[Q] KICK 2: FM, DRIVE & RUMBLE";
        case VIEW_SYNTH1_PAGE1: return "[W] SYNTH 1: TONE & FILTER";
        case VIEW_SYNTH1_PAGE2: return "[W] SYNTH 1: ENV & DELAY SEND";
        case VIEW_SYNTH1_PAGE3: return "[E] SYNTH 1: MOD & SYNTH MIX";
        case VIEW_SYNTH2_PAGE1: return "[E] SYNTH 2: DRONE & CHORD";
        case VIEW_SYNTH2_PAGE2: return "[R] SYNTH 2: AMBIENT SHIMMER";
        case VIEW_SYNTH2_PAGE3: return "[R] SYNTH 2: DRIFT & FADES";
        case VIEW_MASTER_PAGE1: return "[F] MASTER: VOL, MIX & DELAY";
        case VIEW_MASTER_PAGE2: return "[F] SEQUENCER: BPM & KICK GEN";
        default: return "zicPixelDrift";
        }
    }

    bool drawUI(Draw& d, int winW, int winH, bool& needFullRedraw)
    {
        // Clear screen with clean dark background
        d.filledRect({ 0, 0 }, { winW, winH }, { .color = { 18, 18, 24, 255 } });

        // View Title Badge (Top Center)
        d.textCentered({ winW / 2, 14 }, getViewTitle(), 12, { .color = { 0, 210, 255, 255 }, .font = &PoppinsLight_12 });

        // Active Encoders Area (4 Columns)
        auto encs = getActiveEncoders();
        int colW = (winW - 20) / 4;
        int cardH = 100;
        int cardY = 32;

        for (size_t i = 0; i < encs.size(); ++i) {
            const auto& e = encs[i];
            int cardX = 10 + i * colW;

            Color cardBg = { 26, 28, 36, 255 };
            Color cardBorder = (activeEncoderHover == (int)i) ? Color { 0, 230, 255, 255 } : Color { 50, 60, 80, 255 };

            d.filledRect({ cardX, cardY }, { colW - 6, cardH }, { .color = cardBg });
            d.rect({ cardX, cardY }, { colW - 6, cardH }, { .color = cardBorder });

            // Encoder Label
            d.textCentered({ cardX + (colW - 6) / 2, cardY + 14 }, e.label, 8, { .color = { 200, 210, 225, 255 }, .font = &PoppinsLight_8 });

            // Value Text
            std::string valStr = "";
            if (!e.displayOptions.empty()) {
                int optIdx = std::clamp((int)std::round(*(e.value)), 0, (int)e.displayOptions.size() - 1);
                valStr = e.displayOptions[optIdx];
            } else {
                std::stringstream ss;
                ss << std::fixed << std::setprecision(1) << *(e.value) << e.unit;
                valStr = ss.str();
            }

            d.textCentered({ cardX + (colW - 6) / 2, cardY + 38 }, valStr, 12, { .color = { 0, 255, 200, 255 }, .font = &PoppinsLight_12 });

            // Horizontal Gauge
            float pct = (*(e.value) - e.minVal) / (e.maxVal - e.minVal);
            pct = std::clamp(pct, 0.0f, 1.0f);
            int barW = colW - 24;
            int barX = cardX + 9;
            int barY = cardY + 68;

            d.filledRect({ barX, barY }, { barW, 10 }, { .color = { 15, 18, 26, 255 } });
            d.filledRect({ barX, barY }, { (int)(barW * pct), 10 }, { .color = { 0, 200, 255, 255 } });
            d.rect({ barX, barY }, { barW, 10 }, { .color = { 70, 90, 120, 255 } });
        }

        return true;
    }
};
