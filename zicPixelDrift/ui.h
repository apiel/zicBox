#pragma once

#include "draw/draw.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"
#include "kickBody.h"
#include "mixer.h"
#include "sequencer.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

enum ViewState {
    VIEW_MIXER,
    VIEW_SEQUENCER,
    VIEW_KICK_BODY1,
    VIEW_KICK_BODY2,
    VIEW_SYNTH1,
    VIEW_SYNTH2,
    VIEW_COUNT
};

struct EncoderKnob {
    std::string label;
    float* value;
    float minVal;
    float maxVal;
    std::string unit;
    float step;
};

class UiPixelDrift {
public:
    ViewState currentView = VIEW_KICK_BODY1;
    KickBody& kick;
    Sequencer& seq;
    Mixer& mixer;

    int activeEncoderHover = -1;
    bool isKickMuted = false;

    UiPixelDrift(KickBody& k, Sequencer& s, Mixer& m)
        : kick(k)
        , seq(s)
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
            currentView = (currentView == VIEW_MIXER) ? VIEW_SEQUENCER : VIEW_MIXER;
        } else if (key == 'w' || key == 'W') {
            currentView = (currentView == VIEW_KICK_BODY1) ? VIEW_KICK_BODY2 : VIEW_KICK_BODY1;
        } else if (key == 'e' || key == 'E') {
            currentView = VIEW_SYNTH1;
        } else if (key == 'r' || key == 'R') {
            currentView = VIEW_SYNTH2;
        }
    }

    void handlePerformancePad(char key, bool pressed, bool& needFullRedraw)
    {
        if (!pressed) return;
        needFullRedraw = true;

        if (key == 'a' || key == 'A') {
            kick.trigger(1.0f);
        } else if (key == 's' || key == 'S') {
            seq.isMutatedFill = !seq.isMutatedFill;
        } else if (key == 'd' || key == 'D') {
            seq.euclidPulses = (seq.euclidPulses % 16) + 1;
            seq.updateEuclideanPattern();
        } else if (key == 'f' || key == 'F') {
            isKickMuted = !isKickMuted;
        }
    }

    std::vector<EncoderKnob> getActiveEncoders()
    {
        std::vector<EncoderKnob> encs;
        if (currentView == VIEW_MIXER) {
            encs = {
                { "KICK VOL", &mixer.kickVol, 0.0f, 1.0f, "", 0.05f },
                { "SYNTH1 VOL", &mixer.synth1Vol, 0.0f, 1.0f, "", 0.05f },
                { "SYNTH2 VOL", &mixer.synth2Vol, 0.0f, 1.0f, "", 0.05f },
                { "MASTER DRV", &mixer.masterDrive, 0.0f, 1.0f, "", 0.05f }
            };
        } else if (currentView == VIEW_SEQUENCER) {
            encs = {
                { "BPM", &seq.bpm, 40.0f, 240.0f, " bpm", 1.0f },
                { "EUCLID FILL", (float*)&seq.euclidPulses, 1.0f, 16.0f, " pulses", 1.0f },
                { "GATE LEN", &seq.gateLength, 0.1f, 1.0f, "", 0.05f },
                { "MUTATE PROB", &seq.mutationRate, 0.0f, 1.0f, "", 0.05f }
            };
        } else if (currentView == VIEW_KICK_BODY1) {
            encs = {
                fromParam(kick.baseFreq),
                fromParam(kick.clickAmt),
                fromParam(kick.duration),
                fromParam(kick.vcoMorph)
            };
        } else if (currentView == VIEW_KICK_BODY2) {
            encs = {
                fromParam(kick.fmDepth),
                fromParam(kick.drive),
                fromParam(kick.rumbleAmt),
                fromParam(kick.rumbleGap)
            };
        } else if (currentView == VIEW_SYNTH1) {
            static float dummy1 = 0.5f, dummy2 = 0.3f, dummy3 = 0.8f, dummy4 = 0.2f;
            encs = {
                { "SYN1 PITCH", &dummy1, 0.0f, 1.0f, "", 0.05f },
                { "SYN1 CUTOFF", &dummy2, 0.0f, 1.0f, "", 0.05f },
                { "SYN1 RESO", &dummy3, 0.0f, 1.0f, "", 0.05f },
                { "SYN1 DECAY", &dummy4, 0.0f, 1.0f, "", 0.05f }
            };
        } else if (currentView == VIEW_SYNTH2) {
            static float dummy1 = 0.2f, dummy2 = 0.6f, dummy3 = 0.4f, dummy4 = 0.9f;
            encs = {
                { "SYN2 PITCH", &dummy1, 0.0f, 1.0f, "", 0.05f },
                { "SYN2 TEXTURE", &dummy2, 0.0f, 1.0f, "", 0.05f },
                { "SYN2 DELAY", &dummy3, 0.0f, 1.0f, "", 0.05f },
                { "SYN2 REVERB", &dummy4, 0.0f, 1.0f, "", 0.05f }
            };
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
            } else if (e.label == "EUCLID FILL") {
                seq.updateEuclideanPattern();
            }
        }
    }

    std::string getViewTitle()
    {
        switch (currentView) {
        case VIEW_MIXER: return "[Q] MASTER MIXER";
        case VIEW_SEQUENCER: return "[Q] EUCLIDEAN SEQUENCER";
        case VIEW_KICK_BODY1: return "[W] KICK: CLICK & VCO MORPH";
        case VIEW_KICK_BODY2: return "[W] KICK: FM, DRIVE & RUMBLE";
        case VIEW_SYNTH1: return "[E] SYNTH 1 TEXTURE";
        case VIEW_SYNTH2: return "[R] SYNTH 2 AMBIENT";
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
            std::stringstream ss;
            ss << std::fixed << std::setprecision(1) << *(e.value) << e.unit;
            d.textCentered({ cardX + (colW - 6) / 2, cardY + 38 }, ss.str(), 12, { .color = { 0, 255, 200, 255 }, .font = &PoppinsLight_12 });

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

        // Bottom Performance Pad Hints (Bottom 30px)
        int botY = 140;
        std::string padHint = "[A] TRIG   [S] FILL: " + std::string(seq.isMutatedFill ? "ON " : "OFF")
            + "   [D] EUCLID: " + std::to_string(seq.euclidPulses)
            + "   [F] MUTE: " + std::string(isKickMuted ? "MUTED" : "ACTIVE");

        d.textCentered({ winW / 2, botY }, padHint, 8, { .color = { 170, 180, 195, 255 }, .font = &PoppinsLight_8 });

        return true;
    }
};
