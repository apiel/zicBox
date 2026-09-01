#pragma once

#include "audioWorker.h"
#include "draw/draw.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_16.h"
#include "draw/fonts/PoppinsLight_8.h"
#include "studio.h"
#include "ui/uiParams.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

extern std::atomic<bool> keep_running;

enum ViewState {
    VIEW_DRUMS,         // Drums (Volume of each drum)
    VIEW_SYNTH1_PAGE1,  // Synth 1 Osc & Filter
    VIEW_SYNTH1_PAGE2,  // Synth 1 Envelopes & Morph
    VIEW_SYNTH1_PAGE3,  // Synth 1 Modulation & Crush
    VIEW_SYNTH2_PAGE1,  // Synth 2 Wavetable & Filter
    VIEW_SYNTH2_PAGE2,  // Synth 2 Pitch & Envelope
    VIEW_SYNTH2_PAGE3,  // Synth 2 Modulation & Send
    VIEW_MASTER_PAGE1,  // Master Mixer & VU Meters
    VIEW_MASTER_PAGE2,  // Master FX & BPM
    VIEW_SEQUENCER,     // Step Sequencer
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

class UiPixMini {
public:
    ViewState currentView = VIEW_DRUMS;
    int activeEncoderHover = -1;

    DriftSynth1& synth1;
    DriftWavetable& synth2;

    bool isShutdownModalOpen = false;
    int shutdownChoice = 0; // 0 = CANCEL, 1 = SHUTDOWN
    bool isShuttingDown = false;
    bool renderedGoodbye = false;

    // Visual feedback animation state
    float animTime = 0.0f;
    float drumPulseLevel = 0.0f;
    float synth1PulseLevel = 0.0f;
    float synth2PulseLevel = 0.0f;
    int lastSeqStep = -1;

    // Smooth VU meters & peak hold
    float smoothVu[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float peakHoldVal[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float peakHoldDecay[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    int seqCurrentLane = 0; // 0=Snare, 1=CHH, 2=OHH, 3=Clap, 4=Synth1, 5=Synth2
    int seqStepCursor = 0;

    UiPixMini()
        : synth1(studio.trackSynth1.engine)
        , synth2(studio.trackSynth2.engine)
    {
    }

    EncoderKnob fromParam(Param& p)
    {
        return { p.label, &p.value, p.min, p.max, p.unit ? std::string(p.unit) : "", p.step, {}, &p.string, &p };
    }

    void triggerShutdown()
    {
        isShuttingDown = true;
    }

    void shutdown(Draw& d, int winW = 240, int winH = 320)
    {
        d.filledRect({ 0, 0 }, { winW, winH }, { .color = { 8, 10, 15, 255 } });

        int cx = winW / 2;
        int cy = winH / 2;

        struct Star { int x, y; uint8_t brightness; };
        const Star stars[] = {
            { 20, 30, 160 }, { 60, 50, 210 }, { 110, 20, 140 }, { 180, 40, 220 },
            { 210, 90, 180 }, { 40, 140, 190 }, { 200, 160, 200 }, { 90, 220, 170 },
            { 170, 270, 210 }, { 30, 290, 150 }
        };
        for (const auto& s : stars) {
            d.pixel({ s.x, s.y }, { .color = { s.brightness, s.brightness, (uint8_t)std::min(255, s.brightness + 30), 255 } });
        }

        int boxW = 200;
        int boxH = 90;
        int boxX = cx - boxW / 2;
        int boxY = cy - boxH / 2;

        d.filledRect({ boxX, boxY }, { boxW, boxH }, { .color = Color { 16, 20, 30, 240 } });
        d.rect({ boxX, boxY }, { boxW, boxH }, { .color = Color { 60, 80, 110, 255 } });

        d.textCentered({ cx, boxY + 14 }, "zicPixMini", 8, { .color = Color { 140, 170, 210, 255 }, .font = &PoppinsLight_8 });
        d.textCentered({ cx, boxY + 38 }, "GOODBYE", 16, { .color = Color { 240, 245, 255, 255 }, .font = &PoppinsLight_16 });
        d.textCentered({ cx, boxY + 66 }, "POWERING OFF...", 8, { .color = Color { 110, 130, 160, 255 }, .font = &PoppinsLight_8 });
    }

    void halt()
    {
#if defined(IS_RPI)
        int exitCode = std::system("sudo halt || systemctl poweroff || halt");
        (void)exitCode;
#else
        std::this_thread::sleep_for(std::chrono::seconds(2));
#endif
        keep_running = false;
    }

    std::vector<EncoderKnob> getActiveEncoders()
    {
        std::vector<EncoderKnob> encs;
        switch (currentView) {
        case VIEW_DRUMS:
            encs = {
                { "SNARE VOL", &studio.trackDrums.drums.snareVol, 0.0f, 1.0f, "%", 0.05f },
                { "CHH VOL", &studio.trackDrums.drums.hhVol, 0.0f, 1.0f, "%", 0.05f },
                { "OHH VOL", &studio.trackDrums.drums.hhOpenVol, 0.0f, 1.0f, "%", 0.05f },
                { "CLAP VOL", &studio.trackDrums.drums.clapVol, 0.0f, 1.0f, "%", 0.05f }
            };
            break;

        case VIEW_SYNTH1_PAGE1:
            encs = {
                fromParam(synth1.waveform),
                fromParam(synth1.pitch),
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
                { "Mod Type", &synth1.modType.value, 0.0f, 15.0f, "", 1.0f, {
                    "ENV Cutoff", "ENV Pitch", "ENV Wave", "ENV Crsh/FM",
                    "LFO Tri Cut", "LFO Tri Pit", "LFO Tri Wave", "LFO Tri Lvl", "LFO Tri CFM",
                    "LFO Saw Cut", "LFO Saw Pit", "LFO Saw Wave", "LFO Saw CFM",
                    "LFO S&H Cut", "LFO S&H Pit", "LFO S&H CFM"
                }},
                fromParam(synth1.modDepth),
                fromParam(synth1.modSpeed),
                fromParam(synth1.crushFm)
            };
            break;

        case VIEW_SYNTH2_PAGE1:
            encs = {
                fromParam(synth2.wtSelect),
                fromParam(synth2.wavetable),
                fromParam(synth2.cutoff),
                fromParam(synth2.resonance)
            };
            break;

        case VIEW_SYNTH2_PAGE2:
            encs = {
                fromParam(synth2.pitch),
                { "Chord", &synth2.chord.value, 0.0f, 5.0f, "", 1.0f, { "Unison", "Fifth", "Octave", "Maj 7th", "Min 7th", "Sus 4" } },
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
                { "DRUMS VOL", &studio.trackDrums.volume, 0.0f, 1.0f, "%", 0.05f },
                { "SYN1 VOL", &studio.trackSynth1.volume, 0.0f, 1.0f, "%", 0.05f },
                { "SYN2 VOL", &studio.trackSynth2.volume, 0.0f, 1.0f, "%", 0.05f },
                { "BPM", (float*)&studio.bpm, 40.0f, 260.0f, " bpm", 1.0f }
            };
            break;

        case VIEW_MASTER_PAGE2:
            encs = {
                { "BPM", (float*)&studio.bpm, 40.0f, 260.0f, " bpm", 1.0f },
                { "DRUM MUTE", (float*)&studio.trackDrums.isMuted, 0.0f, 1.0f, "", 1.0f, { "Active", "Muted" } },
                { "SYN1 MUTE", (float*)&studio.trackSynth1.isMuted, 0.0f, 1.0f, "", 1.0f, { "Active", "Muted" } },
                { "SYN2 MUTE", (float*)&studio.trackSynth2.isMuted, 0.0f, 1.0f, "", 1.0f, { "Active", "Muted" } }
            };
            break;

        case VIEW_SEQUENCER:
            encs = {
                { "STEP", (float*)&seqStepCursor, 0.0f, 15.0f, "", 1.0f },
                { "LANE", (float*)&seqCurrentLane, 0.0f, 5.0f, "", 1.0f, { "Snare", "CHH", "OHH", "Clap", "Synth1", "Synth2" } },
                { "TOGGLE", (float*)&seqStepCursor, 0.0f, 15.0f, "", 1.0f },
                { "BPM", (float*)&studio.bpm, 40.0f, 260.0f, " bpm", 1.0f }
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
        if (isShutdownModalOpen) {
            shutdownChoice = (shutdownChoice == 0) ? 1 : 0;
            return;
        }

        std::lock_guard<std::mutex> lock(studio.audioMutex);
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

            if (currentView == VIEW_SEQUENCER) {
                if (encoderIdx == 0) seqStepCursor = (int)newVal;
                else if (encoderIdx == 1) seqCurrentLane = (int)newVal;
            }
        }
    }

    void handleEncoderPush(int encIdx, bool pressed, bool& needFullRedraw)
    {
        if (!pressed) return;
        needFullRedraw = true;

        if (isShutdownModalOpen) {
            if (encIdx == 0) isShutdownModalOpen = false;
            else if (encIdx == 1) triggerShutdown();
            return;
        }

        std::lock_guard<std::mutex> lock(studio.audioMutex);

        switch (currentView) {
        case VIEW_DRUMS:
            studio.trackDrums.drums.noteOn(encIdx, 60, 0.9f);
            break;

        case VIEW_SYNTH1_PAGE1:
        case VIEW_SYNTH1_PAGE2:
        case VIEW_SYNTH1_PAGE3:
            if (encIdx == 0) synth1.trigger();
            else if (encIdx == 3) studio.trackSynth1.isMuted = !studio.trackSynth1.isMuted;
            break;

        case VIEW_SYNTH2_PAGE1:
        case VIEW_SYNTH2_PAGE2:
        case VIEW_SYNTH2_PAGE3:
            if (encIdx == 0) synth2.trigger();
            else if (encIdx == 3) studio.trackSynth2.isMuted = !studio.trackSynth2.isMuted;
            break;

        case VIEW_MASTER_PAGE1:
            if (encIdx == 0) studio.trackDrums.isMuted = !studio.trackDrums.isMuted;
            else if (encIdx == 1) studio.trackSynth1.isMuted = !studio.trackSynth1.isMuted;
            else if (encIdx == 2) studio.trackSynth2.isMuted = !studio.trackSynth2.isMuted;
            break;

        case VIEW_SEQUENCER:
            if (seqCurrentLane < 4) {
                studio.trackDrums.sequence[seqCurrentLane][seqStepCursor].active = !studio.trackDrums.sequence[seqCurrentLane][seqStepCursor].active;
            } else if (seqCurrentLane == 4) {
                studio.trackSynth1.sequence[seqStepCursor].active = !studio.trackSynth1.sequence[seqStepCursor].active;
            } else if (seqCurrentLane == 5) {
                studio.trackSynth2.sequence[seqStepCursor].active = !studio.trackSynth2.sequence[seqStepCursor].active;
            }
            break;

        default:
            break;
        }
    }

    void handleButtonKey(char key, bool pressed, bool& needFullRedraw)
    {
        if (!pressed) return;
        needFullRedraw = true;

        // Button Layout: Row 1: A, S, D | Row 2: Z, X, C
        if (key == 'a' || key == 'A') {
            currentView = VIEW_DRUMS;
        } else if (key == 's' || key == 'S') {
            if (currentView == VIEW_SYNTH1_PAGE1) currentView = VIEW_SYNTH1_PAGE2;
            else if (currentView == VIEW_SYNTH1_PAGE2) currentView = VIEW_SYNTH1_PAGE3;
            else currentView = VIEW_SYNTH1_PAGE1;
        } else if (key == 'd' || key == 'D') {
            if (currentView == VIEW_SYNTH2_PAGE1) currentView = VIEW_SYNTH2_PAGE2;
            else if (currentView == VIEW_SYNTH2_PAGE2) currentView = VIEW_SYNTH2_PAGE3;
            else currentView = VIEW_SYNTH2_PAGE1;
        } else if (key == 'z' || key == 'Z') {
            currentView = (currentView == VIEW_MASTER_PAGE1) ? VIEW_MASTER_PAGE2 : VIEW_MASTER_PAGE1;
        } else if (key == 'x' || key == 'X') {
            currentView = VIEW_SEQUENCER;
        } else if (key == 'c' || key == 'C') {
            studio.isPlaying = !studio.isPlaying;
        } else if (key == 'p' || key == 'P') {
            isShutdownModalOpen = true;
        }
    }

    Color getViewThemeColor(ViewState view)
    {
        switch (view) {
        case VIEW_DRUMS:
            return Color { 0, 195, 255, 255 }; // Electric Drum Blue

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
        case VIEW_SEQUENCER:
            return Color { 255, 210, 0, 255 }; // Bright Master Gold

        default:
            return Color { 0, 210, 255, 255 };
        }
    }

    std::pair<int, int> getViewPageInfo() const
    {
        switch (currentView) {
        case VIEW_DRUMS: return { 1, 1 };
        case VIEW_SYNTH1_PAGE1: return { 1, 3 };
        case VIEW_SYNTH1_PAGE2: return { 2, 3 };
        case VIEW_SYNTH1_PAGE3: return { 3, 3 };
        case VIEW_SYNTH2_PAGE1: return { 1, 3 };
        case VIEW_SYNTH2_PAGE2: return { 2, 3 };
        case VIEW_SYNTH2_PAGE3: return { 3, 3 };
        case VIEW_MASTER_PAGE1: return { 1, 2 };
        case VIEW_MASTER_PAGE2: return { 2, 2 };
        case VIEW_SEQUENCER: return { 1, 1 };
        default: return { 1, 1 };
        }
    }

    std::string getViewTitle()
    {
        switch (currentView) {
        case VIEW_DRUMS: return "DRUMS VOL";
        case VIEW_SYNTH1_PAGE1:
        case VIEW_SYNTH1_PAGE2:
        case VIEW_SYNTH1_PAGE3: return "SYNTH 1";
        case VIEW_SYNTH2_PAGE1:
        case VIEW_SYNTH2_PAGE2:
        case VIEW_SYNTH2_PAGE3: return "SYNTH 2";
        case VIEW_MASTER_PAGE1:
        case VIEW_MASTER_PAGE2: return "MASTER";
        case VIEW_SEQUENCER: return "SEQUENCER";
        default: return "zicPixMini";
        }
    }

    int calculatePrecision(float step)
    {
        if (step >= 1.0f) return 0;
        if (step >= 0.1f) return 1;
        return 2;
    }

    void drawParamCard(Draw& d, const EncoderKnob& e, int x, int y, int colW, bool isActiveHover, const Color& themeColor)
    {
        Param tempParam;
        if (e.paramPtr) {
            tempParam = *e.paramPtr;
        } else {
            tempParam.label = e.label.c_str();
            tempParam.value = e.value ? *(e.value) : 0.0f;
            tempParam.min = e.minVal;
            tempParam.max = e.maxVal;
            tempParam.step = e.step;
            tempParam.unit = e.unit.empty() ? nullptr : e.unit.c_str();
            tempParam.precision = calculatePrecision(e.step);
        }

        std::string displayStr;
        if (e.stringPtr && *e.stringPtr && strlen(*e.stringPtr) > 0) {
            tempParam.string = *e.stringPtr;
            tempParam.type = VALUE_STRING;
        } else if (!e.displayOptions.empty() && e.value) {
            int optIdx = std::clamp((int)std::round(*(e.value)), 0, (int)e.displayOptions.size() - 1);
            displayStr = e.displayOptions[optIdx];
            tempParam.string = (char*)displayStr.c_str();
            tempParam.type = VALUE_STRING;
        }

        Color cardBg = isActiveHover ? Color { 40, 52, 75, 255 } : Color { 28, 33, 46, 255 };
        Color cardBorder = isActiveHover ? themeColor : Color { 75, 88, 115, 255 };
        Color pColor = themeColor;

        UiParams::Style driftStyle = {
            .labelColor = Color { 240, 245, 255, 255 },
            .valueColor = Color { 220, 235, 255, 255 },
            .barBgColor = Color { 60, 68, 85, 255 },
            .inactiveSegColor = Color { 90, 105, 130, 255 },
            .midLineColor = Color { 180, 195, 220, 255 },
            .borderColor = cardBorder
        };

        UiParams::param(d, tempParam, colW, d.screenSize.w, x, y, cardBg, pColor, driftStyle);
    }

    void drawVisualFeedback(Draw& d, int winW, int winH)
    {
        int feedbackY = 86;
        int feedbackH = 174; // Y = 86..260 (174px height)
        int feedbackW = winW;

        // Advance animation frame timer
        animTime += 0.05f;

        // Auto-trigger pulse on step hit
        if (studio.isPlaying && studio.currentStep.load() != lastSeqStep) {
            lastSeqStep = studio.currentStep.load();
            if (studio.drumPulseTrigger.exchange(false)) drumPulseLevel = 1.0f;
            if (studio.synth1PulseTrigger.exchange(false)) synth1PulseLevel = 1.0f;
            if (studio.synth2PulseTrigger.exchange(false)) synth2PulseLevel = 1.0f;
        }

        int graphX = 6;
        int graphY = feedbackY;
        int graphW = winW - 12;
        int graphH = feedbackH - 4;

        Color themeColor = getViewThemeColor(currentView);

        // Solid graph box background
        d.filledRect({ graphX - 1, graphY - 1 }, { graphW + 2, graphH + 2 }, { .color = { 12, 14, 20, 255 } });
        d.rect({ graphX - 1, graphY - 1 }, { graphW + 2, graphH + 2 }, { .color = { themeColor.r, themeColor.g, themeColor.b, 220 } });

        switch (currentView) {
        case VIEW_DRUMS: {
            // "For the drum keep it empty."
            d.textCentered({ winW / 2, graphY + graphH / 2 }, "TribeDrums", 8, { .color = Color { 60, 80, 110, 180 }, .font = &PoppinsLight_8 });
            break;
        }

        case VIEW_SYNTH1_PAGE1:
        case VIEW_SYNTH1_PAGE2:
        case VIEW_SYNTH1_PAGE3: {
            Color themeCol = getViewThemeColor(currentView);

            int cx = graphX + graphW / 2;
            int cy = graphY + (graphH / 2) - 10;
            int halfW = 26;
            int halfH = 20;

            float pitchMidi = synth1.pitch.value; // 24 .. 72
            float wf = synth1.waveform.value;     // 0.0 .. 1.0
            float cutVal = synth1.cutoff.value;   // 0.02 .. 0.98
            float resVal = synth1.resonance.value; // 0.0 .. 0.95

            float modD = synth1.modDepth.value * 0.01f;
            float modS = synth1.modSpeed.value * 0.01f;
            float lfoHz = 0.05f + (modS * modS * 39.95f);
            float lfoPhase = std::fmod(animTime * lfoHz * 0.5f, 1.0f);

            int routeIdx = std::clamp((int)std::round(synth1.modType.value), 0, DriftSynth1::TOTAL_MOD_TYPES - 1);
            auto currentRoute = DriftSynth1::modMatrix[routeIdx];

            float lfoVal = 0.0f;
            switch (currentRoute.source) {
            case DriftSynth1::SRC_ENV: lfoVal = synth1PulseLevel; break;
            case DriftSynth1::SRC_LFO_TRI: lfoVal = (lfoPhase < 0.5f) ? (4.0f * lfoPhase - 1.0f) : (3.0f - 4.0f * lfoPhase); break;
            case DriftSynth1::SRC_LFO_SAW: lfoVal = 2.0f * lfoPhase - 1.0f; break;
            case DriftSynth1::SRC_LFO_SH: {
                float stepIdx = std::floor(lfoPhase * 10.0f);
                lfoVal = std::sin(stepIdx * 17.13f + 1.5f);
                break;
            }
            }

            float modAmount = lfoVal * modD;
            if (currentRoute.dest == DriftSynth1::DST_MORPH) {
                wf = std::clamp(wf + modAmount * 0.4f, 0.0f, 1.0f);
            }

            int innerW = graphW - 12;
            int cutX = graphX + 6 + (int)(std::clamp(cutVal, 0.02f, 0.98f) * innerW);

            // Morphing Geometry (Tri -> Saw -> Sq -> Noise)
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

            float noiseFactor = (wf > 0.666f) ? std::clamp((wf - 0.666f) / 0.334f, 0.0f, 1.0f) : 0.0f;
            float baseJitterX = 1.6f + noiseFactor * 5.4f;
            float baseJitterY = 1.4f + noiseFactor * 4.6f;

            std::vector<Point> morphedShape;
            for (size_t i = 0; i < baseShape.size(); ++i) {
                float noiseSeed = animTime * 15.0f + i * 2.3f;
                int jitterX = (int)(std::sin(noiseSeed * 3.7f) * baseJitterX);
                int jitterY = (int)(std::cos(noiseSeed * 4.1f) * baseJitterY);
                morphedShape.push_back({ baseShape[i].x + jitterX, baseShape[i].y + jitterY });
            }

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

            // Draw Polygon Core & Closed Bottom Edge Line!
            d.filledPolygon(morphedShape, { .color = { themeCol.r, themeCol.g, themeCol.b, 60 } });
            d.lines(morphedShape, { .color = themeCol, .thickness = 1 });
            d.line(morphedShape.back(), morphedShape.front(), { .color = themeCol, .thickness = 1 });

            // Dynamic Noise Particle Swarm
            if (noiseFactor > 0.01f) {
                int particleCount = (int)(noiseFactor * 70.0f);
                for (int p = 0; p < particleCount; p++) {
                    float pAngle = p * 0.418f + animTime * (1.2f + (p % 5) * 0.4f);
                    float pDist = std::fmod((float)(p * 7 + animTime * 35.0f), 28.0f);
                    int px = cx + (int)(std::cos(pAngle) * pDist);
                    int py = cy + (int)(std::sin(pAngle) * (pDist * 0.7f));

                    px = std::clamp(px, graphX + 6, graphX + graphW - 6);
                    py = std::clamp(py, graphY + 12, graphY + graphH - 14);

                    uint8_t pAlpha = (uint8_t)((100 + (p * 17 + (int)(animTime * 120)) % 155) * noiseFactor);
                    Color pCol = (p % 3 == 0) ? Color { 255, 255, 255, pAlpha } : Color { 0, 255, 210, pAlpha };
                    d.pixel({ px, py }, pCol);
                }
            }

            // LFO Spinning Icon & Dotted Pointer
            if (std::abs(synth1.modDepth.value) > 1.0f) {
                int lfoCx = graphX + 16;
                int lfoCy = graphY + 16;
                float lfoRadius = 4.5f + std::abs(modD) * 4.5f;
                float spinHz = 0.15f + modS * 1.8f;
                float rotAngle = animTime * spinHz * 6.28318f;

                Color grayCol = { 190, 210, 235, 255 };
                Color dimGrayCol = { 135, 150, 175, 255 };

                std::vector<Point> iconPts;
                if (currentRoute.source == DriftSynth1::SRC_LFO_TRI) {
                    for (int i = 0; i < 3; i++) {
                        float a = rotAngle + i * (6.28318f / 3.0f) - 1.5708f;
                        iconPts.push_back({ lfoCx + (int)(std::cos(a) * lfoRadius), lfoCy + (int)(std::sin(a) * lfoRadius) });
                    }
                    d.lines(iconPts, { .color = grayCol, .thickness = 1 });
                    d.line(iconPts.back(), iconPts.front(), { .color = grayCol, .thickness = 1 });
                } else if (currentRoute.source == DriftSynth1::SRC_LFO_SAW) {
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
                } else {
                    d.circle({ lfoCx, lfoCy }, (int)lfoRadius, { .color = dimGrayCol });
                }

                int dstX = cx;
                int dstY = cy;
                if (currentRoute.dest == DriftSynth1::DST_FILTER) {
                    dstX = cutX;
                    dstY = graphY + graphH - 22;
                }

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

                float pktProgress = std::fmod(animTime * spinHz * 0.8f, 1.0f);
                int px = lfoCx + (int)((dstX - lfoCx) * pktProgress);
                int py = lfoCy + (int)((dstY - lfoCy) * pktProgress);
                d.pixel({ px, py }, Color { 255, 255, 255, 255 });
            }

            // SVF Filter Curve & Resonance Halo Circles
            float envModAmt = synth1.envAmt.value;
            float filterModOffset = (currentRoute.dest == DriftSynth1::DST_FILTER) ? modAmount * 0.35f : 0.0f;
            float modulatedCut = std::clamp(cutVal + (synth1PulseLevel * envModAmt * 0.45f) + filterModOffset, 0.02f, 0.98f);
            cutX = graphX + 6 + (int)(modulatedCut * innerW);

            int baseY = graphY + graphH - 10;
            int passbandH = 16 + (int)(synth1PulseLevel * envModAmt * 6.0f);
            int peakY = baseY - passbandH;
            float fMorph = synth1.filterMorph.value;

            std::vector<Point> svfPoints;
            for (int gx = graphX + 6; gx <= graphX + graphW - 6; gx += 4) {
                float freqNorm = (float)(gx - (graphX + 6)) / (float)innerW;
                float dist = freqNorm - modulatedCut;

                float lpResp = 1.0f / (1.0f + std::pow(freqNorm / std::max(0.04f, modulatedCut), 4.0f));
                float hpResp = 1.0f - lpResp;
                float bpResp = std::exp(-dist * dist * (25.0f + resVal * 50.0f));

                float baseCurve = 0.0f;
                if (fMorph < 0.5f) {
                    baseCurve = lpResp * (1.0f - fMorph * 2.0f) + bpResp * (fMorph * 2.0f);
                } else {
                    baseCurve = bpResp * (1.0f - (fMorph - 0.5f) * 2.0f) + hpResp * ((fMorph - 0.5f) * 2.0f);
                }

                float totalResp = baseCurve + bpResp * (resVal * 2.2f);
                int drawH = std::clamp((int)(totalResp * passbandH), 0, graphH - 20);
                int sy = baseY - drawH;
                svfPoints.push_back({ gx, sy });

                if (std::abs(gx - cutX) <= 4) {
                    peakY = sy;
                }
            }

            if (svfPoints.size() >= 2) {
                std::vector<Point> svfPoly = svfPoints;
                svfPoly.push_back({ graphX + graphW - 6, baseY });
                svfPoly.push_back({ graphX + 6, baseY });
                d.filledPolygon(svfPoly, { .color = { 0, 255, 220, 35 } });
                d.lines(svfPoints, { .color = { 0, 255, 220, 255 }, .thickness = 1 });
            }

            // Resonance Halo Circles around Cutoff Peak!
            if (resVal > 0.01f) {
                for (int h = 0; h < 2; h++) {
                    float haloPulse = std::sin(animTime * 8.0f + h * 1.5f) * 1.5f;
                    int r = (int)(4 + h * 5 + resVal * 6.0f + haloPulse);
                    uint8_t hAlpha = (uint8_t)(std::clamp(180.0f * resVal - h * 50.0f, 0.0f, 255.0f));
                    d.circle({ cutX, peakY }, r, { .color = { 0, 255, 220, hAlpha } });
                }
            }

            // Pitch & Frequency Ribbon
            float pitchHz = 440.0f * std::pow(2.0f, (pitchMidi - 69.0f) / 12.0f);
            int freqY = graphY + graphH - 8;
            std::vector<Point> pitchWave;
            float cycScale = (pitchHz / 110.0f) * 0.15f;
            for (int gx = 0; gx < innerW; gx++) {
                float t = (float)gx / (float)innerW;
                float wave = std::sin(t * (cycScale * 25.0f) + animTime * 4.0f) * 2.5f;
                pitchWave.push_back({ graphX + 6 + gx, freqY + (int)wave });
            }
            d.lines(pitchWave, { .color = { themeCol.r, themeCol.g, themeCol.b, 255 } });

            break;
        }

        case VIEW_SYNTH2_PAGE1:
        case VIEW_SYNTH2_PAGE2:
        case VIEW_SYNTH2_PAGE3: {
            Color syn2Col = Color { 215, 125, 255, 255 };
            int innerW = graphW - 12;

            float pitchMidi = synth2.pitch.value;
            float cutVal = synth2.cutoff.value;
            float modD = synth2.modDepth.value * 0.01f;
            float modS = synth2.modSpeed.value * 0.01f;
            int activeFrameIdx = std::clamp((int)std::round(synth2.wavetable.value - 1.0f), 0, 63);

            // 3D Perspective Real Wavetable Waterfall Mesh - Centered & Utilizing Full Screen Height!
            std::vector<int> sliceFrames = { 0, 8, 16, 24, 32, 40, 48, 56, 63 };
            if (std::find(sliceFrames.begin(), sliceFrames.end(), activeFrameIdx) == sliceFrames.end()) {
                sliceFrames.push_back(activeFrameIdx);
                std::sort(sliceFrames.begin(), sliceFrames.end());
            }

            int numSlices = (int)sliceFrames.size();
            int baseSliceW = innerW - 28;
            int originX = graphX + 6;
            int originY = graphY + graphH - 24;

            std::vector<std::vector<Point>> allSlicePoints(numSlices);
            float driftPhase = animTime * 0.5f;

            for (int i = 0; i < numSlices; i++) {
                int frameIdx = sliceFrames[i];
                float z = (float)frameIdx / 63.0f;
                int sliceOffsetX = (int)((1.0f - z) * 28.0f);
                int sliceOffsetY = (int)(-(1.0f - z) * 22.0f);
                int sliceW = (int)(baseSliceW * (0.72f + z * 0.28f));
                int sliceH = (int)(22.0f * (0.55f + z * 0.45f));

                int sx0 = originX + sliceOffsetX;
                int sy0 = originY + sliceOffsetY;

                int ptsCount = 24;
                for (int p = 0; p <= ptsCount; p++) {
                    float t = (float)p / (float)ptsCount;
                    float tDrift = std::fmod(t + driftPhase, 1.0f);
                    float rawWave = synth2.wt.getSampleAt(frameIdx, tDrift);
                    float filterDamp = 1.0f / (1.0f + std::pow(t / std::max(0.04f, cutVal), 3.0f));
                    float waveH = rawWave * filterDamp * sliceH;

                    if (frameIdx == activeFrameIdx && std::abs(modD) > 0.05f) {
                        waveH += std::sin(t * 16.0f + animTime * 8.0f) * (modD * 3.5f) * (modS * 2.0f);
                    }

                    int px = sx0 + (int)(t * sliceW);
                    int py = sy0 - (int)waveH;
                    allSlicePoints[i].push_back({ px, py });
                }
            }

            // Synth 2 Trigger Stardust Particle Swarm (Dot Swarm Expanding in All Directions!)
            float s2DecayRate = 12.0f / (std::clamp(synth2.release.value, 10.0f, 8000.0f) + 40.0f);
            synth2PulseLevel = std::max(0.0f, synth2PulseLevel - s2DecayRate);

            if (synth2PulseLevel > 0.01f) {
                int horizonX = originX + 28 + (int)(baseSliceW * 0.72f * 0.5f);
                int horizonY = originY - 22;
                float expandProgress = 1.0f - synth2PulseLevel;

                const int numDots = 44;
                for (int k = 0; k < numDots; k++) {
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

            // Render Wireframe Connecting Lattice Lines
            for (int i = 0; i < numSlices - 1; i++) {
                float z = (float)sliceFrames[i] / 63.0f;
                uint8_t meshAlpha = (uint8_t)(70 + z * 125.0f);
                Color meshCol = Color { 160, 90, 225, meshAlpha };

                for (size_t p = 0; p < allSlicePoints[i].size(); p += 4) {
                    d.line(allSlicePoints[i][p], allSlicePoints[i + 1][p], { .color = meshCol });
                }
            }

            // Render 3D Slice Curves (Back-to-Front)
            for (int i = 0; i < numSlices; i++) {
                int frameIdx = sliceFrames[i];
                float z = (float)frameIdx / 63.0f;
                const auto& slicePts = allSlicePoints[i];

                if (frameIdx == activeFrameIdx) {
                    int sliceW = (int)(baseSliceW * (0.72f + z * 0.28f));
                    int sliceOffsetX = (int)((1.0f - z) * 28.0f);
                    int sliceOffsetY = (int)(-(1.0f - z) * 22.0f);
                    int sx0 = originX + sliceOffsetX;
                    int sy0 = originY + sliceOffsetY;

                    std::vector<Point> fillPoly = slicePts;
                    fillPoly.push_back({ sx0 + sliceW, sy0 });
                    fillPoly.push_back({ sx0, sy0 });
                    d.filledPolygon(fillPoly, { .color = { 220, 110, 255, 75 } });
                    d.lines(slicePts, { .color = { 255, 195, 255, 255 }, .thickness = 1 });
                } else {
                    uint8_t alpha = (uint8_t)(90 + z * 115);
                    Color depthCol = (frameIdx < activeFrameIdx) ? Color { 150, 80, 210, alpha } : Color { 205, 120, 255, alpha };
                    d.lines(slicePts, { .color = depthCol, .thickness = 1 });
                }
            }

            // Pitch & Frequency Ribbon
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

        case VIEW_MASTER_PAGE1: {
            Color goldCol = Color { 255, 210, 0, 255 };

            // 4 Channel Strips: DRM, SYN1, SYN2, MAIN
            const char* channelLabels[4] = { "DRM", "SYN1", "SYN2", "MAIN" };
            Color channelColors[4] = {
                { 0, 195, 255, 255 },  // DRM: Electric Blue
                { 0, 240, 190, 255 },  // SYN1: Cyan
                { 215, 125, 255, 255 },// SYN2: Purple
                goldCol                // MAIN: Gold
            };
            float channelLevels[4] = { studio.trackDrums.volume, studio.trackSynth1.volume, studio.trackSynth2.volume, 0.85f };

            float liveDrumsPeak = studio.peakDrums.load();
            float liveSynth1Peak = studio.peakSynth1.load();
            float liveSynth2Peak = studio.peakSynth2.load();
            float liveMasterPeak = studio.peakMaster.load();

            // Active responsive target signals combining real-time audio peaks + trigger pulses + baseline motion
            float animDrums = std::max(liveDrumsPeak, std::max(drumPulseLevel * 0.85f, 0.40f + 0.35f * std::sin(animTime * 4.0f)));
            float animSynth1 = std::max(liveSynth1Peak, std::max(synth1PulseLevel * 0.85f, 0.50f + 0.30f * std::cos(animTime * 3.5f)));
            float animSynth2 = std::max(liveSynth2Peak, std::max(synth2PulseLevel * 0.85f, 0.35f + 0.35f * std::sin(animTime * 5.2f)));
            float animMaster = std::max(liveMasterPeak, std::max({ animDrums, animSynth1, animSynth2 }) * 0.85f);

            float targetSignals[4] = {
                std::clamp(animDrums * studio.trackDrums.volume * (studio.trackDrums.isMuted ? 0.0f : 1.0f), 0.0f, 1.0f),
                std::clamp(animSynth1 * studio.trackSynth1.volume * (studio.trackSynth1.isMuted ? 0.0f : 1.0f), 0.0f, 1.0f),
                std::clamp(animSynth2 * studio.trackSynth2.volume * (studio.trackSynth2.isMuted ? 0.0f : 1.0f), 0.0f, 1.0f),
                std::clamp(animMaster, 0.0f, 1.0f)
            };

            // Smooth Low-Pass Filter & Peak Hold Animation Update
            for (int ch = 0; ch < 4; ch++) {
                float tgt = std::clamp(targetSignals[ch], 0.0f, 1.0f);
                if (tgt > smoothVu[ch]) {
                    smoothVu[ch] += (tgt - smoothVu[ch]) * 0.40f; // Responsive fast attack
                } else {
                    smoothVu[ch] += (tgt - smoothVu[ch]) * 0.12f; // Smooth exponential decay
                }

                if (smoothVu[ch] >= peakHoldVal[ch]) {
                    peakHoldVal[ch] = smoothVu[ch];
                    peakHoldDecay[ch] = 0.5f;
                } else {
                    if (peakHoldDecay[ch] > 0.0f) {
                        peakHoldDecay[ch] -= 0.04f;
                    } else {
                        peakHoldVal[ch] = std::max(smoothVu[ch], peakHoldVal[ch] - 0.015f);
                    }
                }
            }

            int totalStrips = 4;
            int stripW = (graphW - 8) / totalStrips; // ~55px per channel strip

            for (int ch = 0; ch < 4; ch++) {
                int colX = graphX + 4 + ch * stripW;
                Color themeCol = channelColors[ch];
                float lvl = std::clamp(channelLevels[ch], 0.0f, 1.0f);

                // Channel Label
                d.text({ colX + 4, graphY + 4 }, channelLabels[ch], 8, { .color = themeCol, .font = &PoppinsLight_8 });

                // Fader slot track
                int fX = colX + 4;
                int fY = graphY + 16;
                int fW = 10;
                int fH = 110;

                // Fader track background & border
                d.filledRect({ fX, fY }, { fW, fH }, { .color = Color { 20, 24, 34, 255 } });
                d.rect({ fX, fY }, { fW, fH }, { .color = Color { 50, 60, 80, 255 } });

                // Level fill
                int fillH = (int)(fH * lvl);
                if (fillH > 0) {
                    d.filledRect({ fX + 1, fY + fH - fillH }, { fW - 2, fillH }, { .color = { themeCol.r, themeCol.g, themeCol.b, 180 } });
                }

                // Fader handle / cap
                int capY = fY + fH - fillH - 1;
                capY = std::clamp(capY, fY, fY + fH - 2);
                d.filledRect({ fX - 2, capY }, { fW + 4, 3 }, { .color = Color { 245, 250, 255, 255 } });
                d.rect({ fX - 2, capY }, { fW + 4, 3 }, { .color = themeCol });

                // Live VU Meter next to channel fader (Matching Pixel Drift Console VU!)
                int vuX = colX + 18;
                int vuY = graphY + 16;
                int vuW = 10;
                int vuH = 110;

                // VU Track Container
                d.filledRect({ vuX, vuY }, { vuW, vuH }, { .color = Color { 14, 18, 26, 255 } });
                d.rect({ vuX, vuY }, { vuW, vuH }, { .color = Color { 45, 55, 75, 255 } });

                float sigVal = std::clamp(smoothVu[ch], 0.0f, 1.0f);
                int actVuH = (int)((vuH - 2) * sigVal);

                if (actVuH > 0) {
                    // Continuous gradient bar with theme color
                    for (int py = 0; py < actVuH; py++) {
                        float normY = (float)py / (float)(vuH - 2);
                        uint8_t alpha = (uint8_t)(140 + normY * 115.0f);

                        Color segCol = {
                            (uint8_t)std::min(255, (int)(themeCol.r * (0.85f + normY * 0.35f))),
                            (uint8_t)std::min(255, (int)(themeCol.g * (0.85f + normY * 0.35f))),
                            (uint8_t)std::min(255, (int)(themeCol.b * (0.85f + normY * 0.35f))),
                            alpha
                        };

                        if (sigVal > 0.90f && py >= actVuH - 3) {
                            segCol = Color { 255, 90, 70, 255 }; // Hot signal warning
                        }

                        d.line({ vuX + 1, vuY + vuH - 2 - py }, { vuX + vuW - 2, vuY + vuH - 2 - py }, { .color = segCol });
                    }

                    // Faint horizontal LED grid divisions (every 4px)
                    for (int gy = vuY + vuH - 5; gy > vuY + 1; gy -= 4) {
                        d.line({ vuX + 1, gy }, { vuX + vuW - 2, gy }, { .color = Color { 10, 14, 20, 180 } });
                    }
                }

                // Glowing Peak Hold Cap Line
                float pkVal = std::clamp(peakHoldVal[ch], 0.0f, 1.0f);
                if (pkVal > 0.02f) {
                    int pkY = vuY + vuH - 2 - (int)((vuH - 3) * pkVal);
                    pkY = std::clamp(pkY, vuY + 1, vuY + vuH - 2);
                    Color pkCol = (pkVal > 0.90f) ? Color { 255, 80, 60, 255 } : Color { 245, 250, 255, 240 };
                    d.line({ vuX + 1, pkY }, { vuX + vuW - 2, pkY }, { .color = pkCol });
                }
            }

            // Audio Scope Ribbon at Bottom
            int scopeY = graphY + graphH - 10;
            int innerW = graphW - 12;
            std::vector<Point> scopeWave;
            for (int gx = 0; gx < innerW; gx++) {
                float t = (float)gx / (float)innerW;
                float wave = std::sin(t * 18.0f + animTime * 6.0f) * (smoothVu[3] * 6.0f);
                scopeWave.push_back({ graphX + 6 + gx, scopeY + (int)wave });
            }
            d.lines(scopeWave, { .color = goldCol, .thickness = 1 });

            break;
        }

        case VIEW_MASTER_PAGE2: {
            Color goldCol = Color { 255, 210, 0, 255 };
            std::stringstream ssT;
            ssT << "MASTER TEMPO: " << (int)studio.bpm.load() << " BPM";
            d.textCentered({ winW / 2, graphY + 12 }, ssT.str(), 8, { .color = goldCol, .font = &PoppinsLight_8 });

            int timelineY = graphY + 60;
            int startX = graphX + 16;
            int endX = graphX + graphW - 16;

            d.line({ startX, timelineY }, { endX, timelineY }, { .color = Color { 80, 95, 120, 255 } });
            d.filledRect({ startX - 2, timelineY - 3 }, { 5, 7 }, { .color = Color { 0, 240, 190, 255 } });

            int tapStepPx = 28;
            for (int tapIdx = 1; tapIdx <= 5; tapIdx++) {
                int tapX = startX + tapIdx * tapStepPx;
                if (tapX >= endX) break;

                float currentAmp = std::pow(0.65f, (float)tapIdx);
                int barH = (int)(24.0f * currentAmp);
                uint8_t tapAlpha = (uint8_t)(currentAmp * 255.0f);
                Color tapCol = (tapIdx % 2 == 0) ? goldCol : Color { 0, 240, 190, 255 };

                d.filledRect({ tapX - 2, timelineY - barH }, { 5, barH * 2 }, { .color = { tapCol.r, tapCol.g, tapCol.b, tapAlpha } });
                d.text({ tapX - 4, timelineY - barH - 9 }, "#" + std::to_string(tapIdx), 8, { .color = tapCol, .font = &PoppinsLight_8 });
            }

            int echoWaveY = graphY + graphH - 10;
            int innerW = graphW - 12;
            std::vector<Point> delayWave;
            for (int gx = 0; gx < innerW; gx++) {
                float t = (float)gx / (float)innerW;
                float wave = std::sin(t * 25.0f + animTime * 5.0f) * 3.0f;
                delayWave.push_back({ graphX + 6 + gx, echoWaveY + (int)wave });
            }
            d.lines(delayWave, { .color = Color { 0, 240, 190, 200 }, .thickness = 1 });

            break;
        }

        case VIEW_SEQUENCER: {
            drawSequencerOverlay(d, winW, winH);
            break;
        }

        default:
            break;
        }
    }

    void drawSequencerOverlay(Draw& d, int winW, int winH)
    {
        d.filledRect({ 4, 84 }, { winW - 8, 176 }, { .color = { 14, 18, 28, 250 } });
        d.rect({ 4, 84 }, { winW - 8, 176 }, { .color = { 60, 85, 120, 255 } });

        d.text({ 10, 92 }, "STEP SEQUENCER", 8, { .color = { 200, 220, 250, 255 }, .font = &PoppinsLight_8 });

        const char* laneNames[6] = { "SNR", "CHH", "OHH", "CLP", "SN1", "SN2" };
        int laneY = 104;
        int activeStep = studio.currentStep.load();

        for (int l = 0; l < 6; ++l) {
            bool isCurrent = (l == seqCurrentLane);
            Color textCol = isCurrent ? Color { 255, 215, 0, 255 } : Color { 140, 160, 185, 255 };
            d.text({ 10, laneY + 10 }, laneNames[l], 8, { .color = textCol, .font = &PoppinsLight_8 });

            for (int s = 0; s < 16; ++s) {
                int sx = 45 + s * 11;
                int sy = laneY + 2;
                bool isStepActive = false;

                if (l < 4) isStepActive = studio.trackDrums.sequence[l][s].active;
                else if (l == 4) isStepActive = studio.trackSynth1.sequence[s].active;
                else if (l == 5) isStepActive = studio.trackSynth2.sequence[s].active;

                Color stepBg = isStepActive ? Color { 70, 180, 240, 255 } : Color { 25, 34, 50, 255 };
                if (s == activeStep) stepBg = Color { 240, 100, 80, 255 };

                d.filledRect({ sx, sy }, { 9, 10 }, { .color = stepBg });

                if (s == seqStepCursor && isCurrent) {
                    d.rect({ sx - 1, sy - 1 }, { 11, 12 }, { .color = Color { 255, 255, 255, 255 } });
                }
            }
            laneY += 16;
        }

        std::stringstream ss;
        ss << "Lane: " << laneNames[seqCurrentLane] << " | Step: " << (seqStepCursor + 1);
        if (seqCurrentLane == 4) ss << " | Note: " << (int)studio.trackSynth1.sequence[seqStepCursor].note;
        else if (seqCurrentLane == 5) ss << " | Note: " << (int)studio.trackSynth2.sequence[seqStepCursor].note;

        d.text({ 10, 246 }, ss.str().c_str(), 8, { .color = { 180, 200, 230, 255 }, .font = &PoppinsLight_8 });
    }

    void drawShutdownModal(Draw& d, int winW, int winH)
    {
        int boxW = 200;
        int boxH = 100;
        int boxX = (winW - boxW) / 2;
        int boxY = (winH - boxH) / 2;

        d.filledRect({ boxX, boxY }, { boxW, boxH }, { .color = Color { 20, 26, 38, 250 } });
        d.rect({ boxX, boxY }, { boxW, boxH }, { .color = Color { 220, 90, 80, 255 } });

        d.textCentered({ winW / 2, boxY + 20 }, "SHUTDOWN PI?", 12, { .color = Color { 255, 255, 255, 255 }, .font = &PoppinsLight_12 });
        d.textCentered({ winW / 2, boxY + 45 }, "Halt system hardware?", 8, { .color = Color { 180, 190, 210, 255 }, .font = &PoppinsLight_8 });

        d.filledRect({ boxX + 15, boxY + 65 }, { 75, 22 }, { .color = Color { 45, 58, 80, 255 } });
        d.textCentered({ boxX + 52, boxY + 80 }, "[E1] CANCEL", 8, { .color = Color { 220, 230, 245, 255 }, .font = &PoppinsLight_8 });

        d.filledRect({ boxX + 110, boxY + 65 }, { 75, 22 }, { .color = Color { 180, 50, 40, 255 } });
        d.textCentered({ boxX + 147, boxY + 80 }, "[E2] HALT", 8, { .color = Color { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
    }

    bool drawUI(Draw& d, int winW = 240, int winH = 320, bool needFullRedraw = true)
    {
        if (isShuttingDown) {
            if (!renderedGoodbye) {
                shutdown(d, winW, winH);
                renderedGoodbye = true;
                return true;
            } else {
                halt();
                return false;
            }
        }

        // Clear screen
        d.filledRect({ 0, 0 }, { winW, winH }, { .color = { 18, 18, 24, 255 } });

        Color themeColor = getViewThemeColor(currentView);

        // 1. Top Section: 4 Active Parameters in 2 Rows of 2 Params (Y = 0 .. 68)
        auto encs = getActiveEncoders();
        int colW = winW / 2; // 120px
        int rowH = 34;

        for (size_t i = 0; i < encs.size() && i < 4; ++i) {
            int row = (int)i / 2;
            int col = (int)i % 2;
            int x = col * colW;
            int y = row * (rowH + 2);
            bool isActive = (activeEncoderHover == (int)i);
            drawParamCard(d, encs[i], x, y, colW, isActive, themeColor);
        }

        // 2. View Title & Pagination Dots (Y = 68 .. 84)
        d.filledRect({ 0, 74 }, { winW, 16 }, { .color = { 14, 18, 26, 255 } });
        d.textCentered({ winW / 2, 74 }, getViewTitle(), 8, { .color = themeColor, .font = &PoppinsLight_8 });

        auto [pageIdx, totalPages] = getViewPageInfo();
        if (totalPages > 1) {
            int dotW = 5;
            int dotH = 3;
            int gap = 2;
            int totalDotsW = totalPages * dotW + (totalPages - 1) * gap;
            int dotsX = winW - 10 - totalDotsW;
            int dotsY = 74;
            for (int p = 0; p < totalPages; p++) {
                Color dotCol = (p + 1 == pageIdx) ? themeColor : Color { 60, 72, 95, 255 };
                d.filledRect({ dotsX + p * (dotW + gap), dotsY }, { dotW, dotH }, { .color = dotCol });
            }
        }

        // 3. Middle Section Animation / Graph (Y = 84 .. 260)
        drawVisualFeedback(d, winW, winH);

        // 4. Bottom Section Navigation / Button Status Bar (Y = 264 .. 320)
        d.filledRect({ 0, 264 }, { winW, 56 }, { .color = { 14, 17, 24, 255 } });
        d.line({ 0, 264 }, { winW, 264 }, { .color = { 50, 65, 90, 255 } });

        // 2 Rows of 3 Button Items (Row 0: DRM, SYN1, SYN2 | Row 1: MST, SEQ, PLAY/PAUSE)
        auto drawButtonItem = [&](int x, int y, const std::string& label, int targetR, int targetC, Color activeCol, bool isActive) {
            Color txtCol = isActive ? activeCol : Color { 140, 155, 178, 255 };

            int miniGridX = x;
            int miniGridY = y + 2;

            for (int r = 0; r < 2; r++) {
                for (int c = 0; c < 3; c++) {
                    int cx = miniGridX + c * 4;
                    int cy = miniGridY + r * 4;
                    bool isTargetKey = (r == targetR && c == targetC);
                    Color kCol = isTargetKey ? (isActive ? activeCol : Color { 220, 230, 245, 255 }) : Color { 40, 48, 65, 255 };
                    d.filledRect({ cx, cy }, { 2, 2 }, { .color = kCol });
                }
            }

            d.text({ x + 16, y }, label, 8, { .color = txtCol, .font = &PoppinsLight_8 });
        };

        // Row 1 (Lower Y = 280): DRM | SYN1 | SYN2
        drawButtonItem(8, 300, "Drums", 0, 0, Color { 0, 195, 255, 255 }, currentView == VIEW_DRUMS);
        drawButtonItem(84, 300, "Synth1", 0, 1, Color { 0, 240, 190, 255 }, currentView == VIEW_SYNTH1_PAGE1 || currentView == VIEW_SYNTH1_PAGE2 || currentView == VIEW_SYNTH1_PAGE3);
        drawButtonItem(162, 300, "Synth2", 0, 2, Color { 215, 125, 255, 255 }, currentView == VIEW_SYNTH2_PAGE1 || currentView == VIEW_SYNTH2_PAGE2 || currentView == VIEW_SYNTH2_PAGE3);

        // Row 2 (Lower Y = 296): MST | SEQ | PLAY
        drawButtonItem(8, 310, "Master", 1, 0, Color { 255, 210, 0, 255 }, currentView == VIEW_MASTER_PAGE1 || currentView == VIEW_MASTER_PAGE2);
        drawButtonItem(84, 310, "Sequencer", 1, 1, Color { 255, 210, 0, 255 }, currentView == VIEW_SEQUENCER);
        drawButtonItem(162, 310, studio.isPlaying ? "||" : ">", 1, 2, studio.isPlaying ? Color { 80, 220, 140, 255 } : Color { 220, 120, 100, 255 }, studio.isPlaying);

        if (isShutdownModalOpen) {
            drawShutdownModal(d, winW, winH);
        }

        return true;
    }
};
