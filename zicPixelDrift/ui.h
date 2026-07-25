#pragma once

#include "draw/draw.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"
#include "kickBody.h"
#include "mixer.h"
#include "sequencer.h"
#include "spaceBackground.h"

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
    SpaceBackground background;

    int activeEncoderHover = -1;
    bool isKickMuted = false;

    UiPixelDrift(KickBody& k, Sequencer& s, Mixer& m)
        : kick(k)
        , seq(s)
        , mixer(m)
    {
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
            background.triggerKickPulse();
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
                { "BASE FREQ", &kick.baseFreq, 30.0f, 100.0f, " Hz", 1.0f },
                { "SWEEP DEP", &kick.sweepDepth, 0.0f, 200.0f, " Hz", 2.0f },
                { "SWEEP DEC", &kick.sweepDecayMs, 5.0f, 200.0f, " ms", 2.0f },
                { "DURATION", &kick.durationMs, 50.0f, 1200.0f, " ms", 10.0f }
            };
        } else if (currentView == VIEW_KICK_BODY2) {
            encs = {
                { "VCO MORPH", &kick.vcoMorph, 0.0f, 1.0f, "", 0.05f },
                { "WAVEFOLD", &kick.wavefoldDrive, 0.0f, 1.0f, "", 0.05f },
                { "SUB RUMBLE", &kick.rumbleLevel, 0.0f, 1.0f, "", 0.05f },
                { "RUMBLE GAP", &kick.rumbleGapMs, 10.0f, 300.0f, " ms", 5.0f }
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
            } else if (e.label == "SWEEP DEC") {
                kick.updateSweepRatio();
            }
        }
    }

    std::string getViewTitle()
    {
        switch (currentView) {
        case VIEW_MIXER: return "[Q] MASTER MIXER";
        case VIEW_SEQUENCER: return "[Q] EUCLIDEAN SEQUENCER";
        case VIEW_KICK_BODY1: return "[W] KICK PITCH & BODY";
        case VIEW_KICK_BODY2: return "[W] KICK WAVEFOLD & RUMBLE";
        case VIEW_SYNTH1: return "[E] SYNTH 1 TEXTURE";
        case VIEW_SYNTH2: return "[R] SYNTH 2 AMBIENT";
        default: return "zicPixelDrift";
        }
    }

    bool drawUI(Draw& d, int winW, int winH, bool& needFullRedraw)
    {
        // Render 3D Spatial Background
        background.updateAndDraw(d, seq.bpm, kick.wavefoldDrive, winW, winH);

        // Header Navigation Bar (Top 26px)
        d.filledRect({ 5, 5 }, { winW - 10, 24 }, { .color = { 20, 20, 28, 220 } });
        d.rect({ 5, 5 }, { winW - 10, 24 }, { .color = { 0, 180, 255, 255 } });
        d.text({ 12, 11 }, "PIXEL DRIFT  |  " + getViewTitle(), 12, { .color = { 0, 220, 255, 255 }, .font = &PoppinsLight_12 });

        // Step Sequencer Indicator Dots (Top right of header)
        for (int i = 0; i < 16; ++i) {
            bool isCurrent = (seq.currentStep == i);
            bool isEuclid = seq.activeEuclid[i];
            Color dotCol = isCurrent ? Color { 255, 255, 255, 255 } : (isEuclid ? Color { 0, 255, 170, 255 } : Color { 45, 45, 60, 255 });
            d.filledRect({ winW - 145 + i * 8, 12 }, { 6, 10 }, { .color = dotCol });
        }

        // Active Encoders Area (Middle 4 Columns)
        auto encs = getActiveEncoders();
        int colW = (winW - 20) / 4;
        int cardH = 95;
        int cardY = 35;

        for (size_t i = 0; i < encs.size(); ++i) {
            const auto& e = encs[i];
            int cardX = 10 + i * colW;

            Color cardBg = { 15, 18, 26, 210 };
            Color cardBorder = (activeEncoderHover == (int)i) ? Color { 0, 230, 255, 255 } : Color { 45, 55, 75, 255 };

            d.filledRect({ cardX, cardY }, { colW - 6, cardH }, { .color = cardBg });
            d.rect({ cardX, cardY }, { colW - 6, cardH }, { .color = cardBorder });

            // Encoder Label
            d.textCentered({ cardX + (colW - 6) / 2, cardY + 12 }, e.label, 8, { .color = { 200, 210, 225, 255 }, .font = &PoppinsLight_8 });

            // Value Text
            std::stringstream ss;
            ss << std::fixed << std::setprecision(1) << *(e.value) << e.unit;
            d.textCentered({ cardX + (colW - 6) / 2, cardY + 34 }, ss.str(), 12, { .color = { 0, 255, 200, 255 }, .font = &PoppinsLight_12 });

            // Horizontal Progress Bar Gauge
            float pct = (*(e.value) - e.minVal) / (e.maxVal - e.minVal);
            pct = std::clamp(pct, 0.0f, 1.0f);
            int barW = colW - 24;
            int barX = cardX + 9;
            int barY = cardY + 62;

            d.filledRect({ barX, barY }, { barW, 10 }, { .color = { 25, 30, 42, 255 } });
            d.filledRect({ barX, barY }, { (int)(barW * pct), 10 }, { .color = { 0, 200, 255, 255 } });
            d.rect({ barX, barY }, { barW, 10 }, { .color = { 70, 90, 120, 255 } });
        }

        // Bottom Performance Pad Hints (Bottom 35px)
        int botY = 136;
        d.filledRect({ 5, botY }, { winW - 10, 35 }, { .color = { 15, 15, 22, 220 } });
        d.rect({ 5, botY }, { winW - 10, 35 }, { .color = { 50, 50, 70, 255 } });

        std::string padHint = "[A] TRIG KICK   [S] FILL: " + std::string(seq.isMutatedFill ? "ON " : "OFF")
            + "   [D] EUCLID: " + std::to_string(seq.euclidPulses)
            + "   [F] KICK MUTE: " + std::string(isKickMuted ? "MUTED" : "ACTIVE");

        d.textCentered({ winW / 2, botY + 12 }, padHint, 8, { .color = { 220, 220, 230, 255 }, .font = &PoppinsLight_8 });

        return true;
    }
};
