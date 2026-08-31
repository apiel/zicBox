#pragma once

#include "audioWorker.h"
#include "draw/draw.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_16.h"
#include "draw/fonts/PoppinsLight_8.h"
#include "studio.h"

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
    VIEW_DRUMS_PAGE1,   // [B1] Snare & Clap
    VIEW_DRUMS_PAGE2,   // [B1] Closed & Open HH
    VIEW_SYNTH1_PAGE1,  // [B2] Osc & Filter
    VIEW_SYNTH1_PAGE2,  // [B2] Envelopes
    VIEW_SYNTH1_PAGE3,  // [B2] Env Amt & Send
    VIEW_SYNTH2_PAGE1,  // [B3] Wavetable & Filter
    VIEW_SYNTH2_PAGE2,  // [B3] Envelopes
    VIEW_SYNTH2_PAGE3,  // [B3] Env Amt & Send
    VIEW_MASTER_PAGE1,  // [B4] Mixer Volumes
    VIEW_MASTER_PAGE2,  // [B4] Scatter & Master Filter
    VIEW_SEQUENCER,     // [B5] Step Sequencer
    VIEW_COUNT
};

struct CornerCard {
    std::string title;
    std::string valueStr;
    float normVal = 0.0f;
    std::string pushHint;

    CornerCard() = default;
    CornerCard(std::string t, std::string v, float n, std::string p = "")
        : title(std::move(t))
        , valueStr(std::move(v))
        , normVal(n)
        , pushHint(std::move(p))
    {
    }
};

class UiPixMini {
public:
    ViewState currentView = VIEW_DRUMS_PAGE1;
    int activeEncoderHover = -1;

    bool isShutdownModalOpen = false;
    int shutdownChoice = 0; // 0 = CANCEL, 1 = SHUTDOWN

    bool isShuttingDown = false;
    bool renderedGoodbye = false;

    int seqCurrentLane = 0; // 0=Snare, 1=CHH, 2=OHH, 3=Clap, 4=Synth1, 5=Synth2
    int seqStepCursor = 0;

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

    void handleEncoderTurn(int encIdx, int dir, bool& needFullRedraw)
    {
        needFullRedraw = true;
        std::lock_guard<std::mutex> lock(studio.audioMutex);

        float delta = dir * 0.05f;

        switch (currentView) {
        case VIEW_DRUMS_PAGE1:
            if (encIdx == 0) studio.trackDrums.drums.snare.baseFrequency.set(std::clamp(studio.trackDrums.drums.snare.baseFrequency.value + dir * 5.0f, 30.0f, 400.0f));
            else if (encIdx == 1) studio.trackDrums.drums.snare.bodyDuration.set(std::clamp(studio.trackDrums.drums.snare.bodyDuration.value + dir * 10.0f, 10.0f, 800.0f));
            else if (encIdx == 2) studio.trackDrums.drums.clap.clapNoiseClr.set(std::clamp(studio.trackDrums.drums.clap.clapNoiseClr.value + dir * 2.0f, 0.0f, 100.0f));
            else if (encIdx == 3) studio.trackDrums.drums.clap.hiClapDuration.set(std::clamp(studio.trackDrums.drums.clap.hiClapDuration.value + dir * 10.0f, 20.0f, 1000.0f));
            break;

        case VIEW_DRUMS_PAGE2:
            if (encIdx == 0) studio.trackDrums.drums.hhClosed.hiTightness.set(std::clamp(studio.trackDrums.drums.hhClosed.hiTightness.value + dir * 2.0f, 0.0f, 100.0f));
            else if (encIdx == 1) studio.trackDrums.drums.hhClosed.hiClapDuration.set(std::clamp(studio.trackDrums.drums.hhClosed.hiClapDuration.value + dir * 5.0f, 10.0f, 300.0f));
            else if (encIdx == 2) studio.trackDrums.drums.hhOpen.hiTightness.set(std::clamp(studio.trackDrums.drums.hhOpen.hiTightness.value + dir * 2.0f, 0.0f, 100.0f));
            else if (encIdx == 3) studio.trackDrums.drums.hhOpen.hiClapDuration.set(std::clamp(studio.trackDrums.drums.hhOpen.hiClapDuration.value + dir * 10.0f, 20.0f, 800.0f));
            break;

        case VIEW_SYNTH1_PAGE1:
            if (encIdx == 0) studio.trackSynth1.engine.waveform.set(std::clamp(studio.trackSynth1.engine.waveform.value + delta * 0.2f, 0.0f, 1.0f));
            else if (encIdx == 1) studio.trackSynth1.engine.pitch.set(std::clamp(studio.trackSynth1.engine.pitch.value + dir * 1.0f, 24.0f, 72.0f));
            else if (encIdx == 2) studio.trackSynth1.engine.cutoff.set(std::clamp(studio.trackSynth1.engine.cutoff.value + delta * 0.1f, 0.02f, 0.98f));
            else if (encIdx == 3) studio.trackSynth1.engine.resonance.set(std::clamp(studio.trackSynth1.engine.resonance.value + delta * 0.1f, 0.0f, 0.95f));
            break;

        case VIEW_SYNTH1_PAGE2:
            if (encIdx == 0) studio.trackSynth1.engine.release.set(std::clamp(studio.trackSynth1.engine.release.value + dir * 10.0f, 10.0f, 2000.0f));
            else if (encIdx == 1) studio.trackSynth1.engine.envAmt.set(std::clamp(studio.trackSynth1.engine.envAmt.value + delta * 0.1f, 0.0f, 1.0f));
            else if (encIdx == 2) studio.trackSynth1.engine.filterMorph.set(std::clamp(studio.trackSynth1.engine.filterMorph.value + delta * 0.1f, 0.0f, 1.0f));
            else if (encIdx == 3) studio.trackSynth1.engine.delaySend.set(std::clamp(studio.trackSynth1.engine.delaySend.value + dir * 2.0f, 0.0f, 100.0f));
            break;

        case VIEW_SYNTH1_PAGE3:
            if (encIdx == 0) studio.trackSynth1.engine.modType.set(std::clamp(studio.trackSynth1.engine.modType.value + dir * 1.0f, 0.0f, 15.0f));
            else if (encIdx == 1) studio.trackSynth1.engine.modDepth.set(std::clamp(studio.trackSynth1.engine.modDepth.value + dir * 2.0f, -100.0f, 100.0f));
            else if (encIdx == 2) studio.trackSynth1.volume = std::clamp(studio.trackSynth1.volume + delta * 0.05f, 0.0f, 1.0f);
            else if (encIdx == 3) studio.bpm.store(std::clamp(studio.bpm.load() + dir * 1.0f, 60.0f, 200.0f));
            break;

        case VIEW_SYNTH2_PAGE1:
            if (encIdx == 0) studio.trackSynth2.engine.wavetable.set(std::clamp(studio.trackSynth2.engine.wavetable.value + dir * 0.5f, 1.0f, 64.0f));
            else if (encIdx == 1) studio.trackSynth2.engine.chord.set(std::clamp(studio.trackSynth2.engine.chord.value + dir * 1.0f, 0.0f, 5.0f));
            else if (encIdx == 2) studio.trackSynth2.engine.cutoff.set(std::clamp(studio.trackSynth2.engine.cutoff.value + delta * 0.1f, 0.02f, 0.98f));
            else if (encIdx == 3) studio.trackSynth2.engine.resonance.set(std::clamp(studio.trackSynth2.engine.resonance.value + delta * 0.1f, 0.0f, 0.95f));
            break;

        case VIEW_SYNTH2_PAGE2:
            if (encIdx == 0) studio.trackSynth2.engine.attack.set(std::clamp(studio.trackSynth2.engine.attack.value + dir * 10.0f, 0.0f, 4000.0f));
            else if (encIdx == 1) studio.trackSynth2.engine.release.set(std::clamp(studio.trackSynth2.engine.release.value + dir * 10.0f, 10.0f, 8000.0f));
            else if (encIdx == 2) studio.trackSynth2.engine.delaySend.set(std::clamp(studio.trackSynth2.engine.delaySend.value + dir * 2.0f, 0.0f, 100.0f));
            else if (encIdx == 3) studio.trackSynth2.volume = std::clamp(studio.trackSynth2.volume + delta * 0.05f, 0.0f, 1.0f);
            break;

        case VIEW_SYNTH2_PAGE3:
            if (encIdx == 0) studio.trackSynth2.engine.modType.set(std::clamp(studio.trackSynth2.engine.modType.value + dir * 1.0f, 0.0f, 16.0f));
            else if (encIdx == 1) studio.trackSynth2.engine.modDepth.set(std::clamp(studio.trackSynth2.engine.modDepth.value + dir * 2.0f, -100.0f, 100.0f));
            else if (encIdx == 2) studio.trackSynth2.engine.modSpeed.set(std::clamp(studio.trackSynth2.engine.modSpeed.value + dir * 2.0f, 0.0f, 100.0f));
            else if (encIdx == 3) studio.bpm.store(std::clamp(studio.bpm.load() + dir * 1.0f, 60.0f, 200.0f));
            break;

        case VIEW_MASTER_PAGE1:
            if (encIdx == 0) studio.trackDrums.volume = std::clamp(studio.trackDrums.volume + delta * 0.05f, 0.0f, 1.0f);
            else if (encIdx == 1) studio.trackSynth1.volume = std::clamp(studio.trackSynth1.volume + delta * 0.05f, 0.0f, 1.0f);
            else if (encIdx == 2) studio.trackSynth2.volume = std::clamp(studio.trackSynth2.volume + delta * 0.05f, 0.0f, 1.0f);
            else if (encIdx == 3) studio.bpm.store(std::clamp(studio.bpm.load() + dir * 1.0f, 60.0f, 200.0f));
            break;

        case VIEW_MASTER_PAGE2:
            if (encIdx == 3) studio.bpm.store(std::clamp(studio.bpm.load() + dir * 1.0f, 60.0f, 200.0f));
            break;

        case VIEW_SEQUENCER:
            if (encIdx == 0) {
                seqStepCursor = (seqStepCursor + dir + SEQ_STEPS_MINI) % SEQ_STEPS_MINI;
            } else if (encIdx == 1) {
                seqCurrentLane = (seqCurrentLane + dir + 6) % 6;
            } else if (encIdx == 2) {
                if (seqCurrentLane == 4) {
                    studio.trackSynth1.sequence[seqStepCursor].note = std::clamp(studio.trackSynth1.sequence[seqStepCursor].note + dir, 24, 96);
                } else if (seqCurrentLane == 5) {
                    studio.trackSynth2.sequence[seqStepCursor].note = std::clamp(studio.trackSynth2.sequence[seqStepCursor].note + dir, 24, 96);
                }
            } else if (encIdx == 3) {
                studio.bpm.store(std::clamp(studio.bpm.load() + dir * 1.0f, 60.0f, 200.0f));
            }
            break;

        default:
            break;
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
        case VIEW_DRUMS_PAGE1:
        case VIEW_DRUMS_PAGE2:
            if (encIdx == 0) studio.trackDrums.drums.noteOn(0, 60, 0.9f);      // Snare
            else if (encIdx == 1) studio.trackDrums.drums.noteOn(1, 60, 0.9f); // Closed HH
            else if (encIdx == 2) studio.trackDrums.drums.noteOn(2, 60, 0.9f); // Open HH
            else if (encIdx == 3) studio.trackDrums.drums.noteOn(3, 60, 0.9f); // Clap
            break;

        case VIEW_SYNTH1_PAGE1:
        case VIEW_SYNTH1_PAGE2:
        case VIEW_SYNTH1_PAGE3:
            if (encIdx == 0) studio.trackSynth1.engine.trigger();
            else if (encIdx == 3) studio.trackSynth1.isMuted = !studio.trackSynth1.isMuted;
            break;

        case VIEW_SYNTH2_PAGE1:
        case VIEW_SYNTH2_PAGE2:
        case VIEW_SYNTH2_PAGE3:
            if (encIdx == 0) studio.trackSynth2.engine.trigger();
            else if (encIdx == 3) studio.trackSynth2.isMuted = !studio.trackSynth2.isMuted;
            break;

        case VIEW_MASTER_PAGE1:
            if (encIdx == 0) studio.trackDrums.isMuted = !studio.trackDrums.isMuted;
            else if (encIdx == 1) studio.trackSynth1.isMuted = !studio.trackSynth1.isMuted;
            else if (encIdx == 2) studio.trackSynth2.isMuted = !studio.trackSynth2.isMuted;
            break;

        case VIEW_SEQUENCER:
            if (encIdx == 2 || encIdx == 0) {
                if (seqCurrentLane < 4) {
                    studio.trackDrums.sequence[seqCurrentLane][seqStepCursor].active = !studio.trackDrums.sequence[seqCurrentLane][seqStepCursor].active;
                } else if (seqCurrentLane == 4) {
                    studio.trackSynth1.sequence[seqStepCursor].active = !studio.trackSynth1.sequence[seqStepCursor].active;
                } else if (seqCurrentLane == 5) {
                    studio.trackSynth2.sequence[seqStepCursor].active = !studio.trackSynth2.sequence[seqStepCursor].active;
                }
            } else if (encIdx == 1) {
                seqCurrentLane = (seqCurrentLane + 1) % 6;
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

        if (key == '1' || key == 'z') {
            currentView = (currentView == VIEW_DRUMS_PAGE1) ? VIEW_DRUMS_PAGE2 : VIEW_DRUMS_PAGE1;
        } else if (key == '2' || key == 'x') {
            if (currentView == VIEW_SYNTH1_PAGE1) currentView = VIEW_SYNTH1_PAGE2;
            else if (currentView == VIEW_SYNTH1_PAGE2) currentView = VIEW_SYNTH1_PAGE3;
            else currentView = VIEW_SYNTH1_PAGE1;
        } else if (key == '3' || key == 'c') {
            if (currentView == VIEW_SYNTH2_PAGE1) currentView = VIEW_SYNTH2_PAGE2;
            else if (currentView == VIEW_SYNTH2_PAGE2) currentView = VIEW_SYNTH2_PAGE3;
            else currentView = VIEW_SYNTH2_PAGE1;
        } else if (key == '4' || key == 'v') {
            currentView = (currentView == VIEW_MASTER_PAGE1) ? VIEW_MASTER_PAGE2 : VIEW_MASTER_PAGE1;
        } else if (key == '5' || key == 'b') {
            currentView = VIEW_SEQUENCER;
        } else if (key == '6' || key == 'n') {
            studio.isPlaying = !studio.isPlaying;
        } else if (key == 'p') {
            isShutdownModalOpen = true;
        }
    }

    bool drawUI(Draw& d, int winW = 240, int winH = 320, bool needFullRedraw = true)
    {
        if (isShuttingDown) {
            if (!renderedGoodbye) {
                shutdown(d, winW, winH);
                renderedGoodbye = true;
                return true;
            }
            return false;
        }

        // 1. Dark Slate Background
        d.filledRect({ 0, 0 }, { winW, winH }, { .color = { 12, 16, 24, 255 } });

        // 2. Header Bar (Y: 0..24)
        d.filledRect({ 0, 0 }, { winW, 24 }, { .color = { 20, 26, 38, 255 } });
        d.line({ 0, 24 }, { winW, 24 }, { .color = { 50, 65, 90, 255 } });

        std::string viewTitle = "zicPixMini";
        switch (currentView) {
        case VIEW_DRUMS_PAGE1: viewTitle = "DRUMS: SNARE / CLAP"; break;
        case VIEW_DRUMS_PAGE2: viewTitle = "DRUMS: HI-HATS"; break;
        case VIEW_SYNTH1_PAGE1: viewTitle = "SYNTH 1: OSC / FLT"; break;
        case VIEW_SYNTH1_PAGE2: viewTitle = "SYNTH 1: ENVELOPE"; break;
        case VIEW_SYNTH1_PAGE3: viewTitle = "SYNTH 1: FX / MIX"; break;
        case VIEW_SYNTH2_PAGE1: viewTitle = "SYNTH 2: WT / FLT"; break;
        case VIEW_SYNTH2_PAGE2: viewTitle = "SYNTH 2: ENVELOPE"; break;
        case VIEW_SYNTH2_PAGE3: viewTitle = "SYNTH 2: FX / MIX"; break;
        case VIEW_MASTER_PAGE1: viewTitle = "MASTER: MIXER"; break;
        case VIEW_MASTER_PAGE2: viewTitle = "MASTER: FX / BPM"; break;
        case VIEW_SEQUENCER: viewTitle = "STEP SEQUENCER"; break;
        default: break;
        }

        d.text({ 6, 16 }, viewTitle.c_str(), 8, { .color = { 220, 230, 245, 255 }, .font = &PoppinsLight_8 });

        std::stringstream bpmSs;
        bpmSs << (studio.isPlaying ? "PLAY " : "PAUSE ") << (int)studio.bpm.load() << " BPM";
        d.textRight({ winW - 6, 16 }, bpmSs.str().c_str(), 8, { .color = studio.isPlaying ? Color { 80, 220, 140, 255 } : Color { 220, 120, 100, 255 }, .font = &PoppinsLight_8 });

        // 3. Corner Cards Grid (Y: 28..262)
        CornerCard cards[4];
        getCornerCards(cards);

        int cardW = 112;
        int cardH = 112;
        int cardPos[4][2] = {
            { 5, 28 },    // TL (Enc 1)
            { 123, 28 },  // TR (Enc 2)
            { 5, 145 },   // BL (Enc 3)
            { 123, 145 }  // BR (Enc 4)
        };

        for (int c = 0; c < 4; ++c) {
            int cx = cardPos[c][0];
            int cy = cardPos[c][1];
            bool isHovered = (activeEncoderHover == c);

            Color bgColor = isHovered ? Color { 32, 42, 60, 255 } : Color { 20, 26, 38, 255 };
            Color borderCol = isHovered ? Color { 90, 140, 220, 255 } : Color { 45, 58, 80, 255 };

            d.filledRect({ cx, cy }, { cardW, cardH }, { .color = bgColor });
            d.rect({ cx, cy }, { cardW, cardH }, { .color = borderCol });

            std::string encLabel = "E" + std::to_string(c + 1);
            d.filledRect({ cx + 4, cy + 4 }, { 20, 14 }, { .color = Color { 35, 48, 70, 255 } });
            d.textCentered({ cx + 14, cy + 14 }, encLabel.c_str(), 8, { .color = Color { 140, 180, 240, 255 }, .font = &PoppinsLight_8 });

            d.textCentered({ cx + cardW / 2, cy + 28 }, cards[c].title.c_str(), 8, { .color = Color { 180, 195, 215, 255 }, .font = &PoppinsLight_8 });
            d.textCentered({ cx + cardW / 2, cy + 56 }, cards[c].valueStr.c_str(), 12, { .color = Color { 240, 245, 255, 255 }, .font = &PoppinsLight_12 });

            int barW = 90;
            int barH = 8;
            int barX = cx + (cardW - barW) / 2;
            int barY = cy + 74;
            d.filledRect({ barX, barY }, { barW, barH }, { .color = Color { 10, 14, 22, 255 } });
            d.rect({ barX, barY }, { barW, barH }, { .color = Color { 60, 75, 100, 255 } });

            int fillW = static_cast<int>(std::clamp(cards[c].normVal, 0.0f, 1.0f) * (barW - 2));
            if (fillW > 0) {
                d.filledRect({ barX + 1, barY + 1 }, { fillW, barH - 2 }, { .color = Color { 70, 170, 240, 255 } });
            }

            if (!cards[c].pushHint.empty()) {
                d.textCentered({ cx + cardW / 2, cy + 102 }, cards[c].pushHint.c_str(), 8, { .color = Color { 120, 145, 175, 255 }, .font = &PoppinsLight_8 });
            }
        }

        // 4. Bottom Navigation / Button Bar (Y: 264..320)
        d.filledRect({ 0, 264 }, { winW, 56 }, { .color = { 16, 22, 32, 255 } });
        d.line({ 0, 264 }, { winW, 264 }, { .color = { 50, 65, 90, 255 } });

        int btnW = 72;
        int btnH = 22;
        int btnPos[6][2] = {
            { 5, 268 }, { 84, 268 }, { 163, 268 },
            { 5, 294 }, { 84, 294 }, { 163, 294 }
        };

        const char* btnLabels[6] = { "B1: DRM", "B2: SYN1", "B3: SYN2", "B4: MST", "B5: SEQ", studio.isPlaying ? "B6: ||" : "B6: >" };
        for (int b = 0; b < 6; ++b) {
            bool isActive = false;
            if (b == 0 && (currentView == VIEW_DRUMS_PAGE1 || currentView == VIEW_DRUMS_PAGE2)) isActive = true;
            else if (b == 1 && (currentView == VIEW_SYNTH1_PAGE1 || currentView == VIEW_SYNTH1_PAGE2 || currentView == VIEW_SYNTH1_PAGE3)) isActive = true;
            else if (b == 2 && (currentView == VIEW_SYNTH2_PAGE1 || currentView == VIEW_SYNTH2_PAGE2 || currentView == VIEW_SYNTH2_PAGE3)) isActive = true;
            else if (b == 3 && (currentView == VIEW_MASTER_PAGE1 || currentView == VIEW_MASTER_PAGE2)) isActive = true;
            else if (b == 4 && currentView == VIEW_SEQUENCER) isActive = true;

            Color bCol = isActive ? Color { 45, 85, 140, 255 } : Color { 24, 32, 46, 255 };
            Color tCol = isActive ? Color { 240, 245, 255, 255 } : Color { 140, 160, 190, 255 };

            d.filledRect({ btnPos[b][0], btnPos[b][1] }, { btnW, btnH }, { .color = bCol });
            d.rect({ btnPos[b][0], btnPos[b][1] }, { btnW, btnH }, { .color = Color { 55, 75, 105, 255 } });
            d.textCentered({ btnPos[b][0] + btnW / 2, btnPos[b][1] + 15 }, btnLabels[b], 8, { .color = tCol, .font = &PoppinsLight_8 });
        }

        if (currentView == VIEW_SEQUENCER) {
            drawSequencerOverlay(d, winW, winH);
        }

        if (isShutdownModalOpen) {
            drawShutdownModal(d, winW, winH);
        }

        return true;
    }

    void drawSequencerOverlay(Draw& d, int winW, int winH)
    {
        d.filledRect({ 4, 28 }, { winW - 8, 232 }, { .color = { 14, 18, 28, 250 } });
        d.rect({ 4, 28 }, { winW - 8, 232 }, { .color = { 60, 85, 120, 255 } });

        d.text({ 10, 42 }, "STEP SEQUENCER", 8, { .color = { 200, 220, 250, 255 }, .font = &PoppinsLight_8 });

        const char* laneNames[6] = { "SNR", "CHH", "OHH", "CLP", "SN1", "SN2" };
        int laneY = 54;
        int activeStep = studio.currentStep.load();

        for (int l = 0; l < 6; ++l) {
            bool isCurrent = (l == seqCurrentLane);
            Color textCol = isCurrent ? Color { 255, 215, 0, 255 } : Color { 140, 160, 185, 255 };
            d.text({ 10, laneY + 12 }, laneNames[l], 8, { .color = textCol, .font = &PoppinsLight_8 });

            for (int s = 0; s < 16; ++s) {
                int sx = 45 + s * 11;
                int sy = laneY + 2;
                bool isStepActive = false;

                if (l < 4) isStepActive = studio.trackDrums.sequence[l][s].active;
                else if (l == 4) isStepActive = studio.trackSynth1.sequence[s].active;
                else if (l == 5) isStepActive = studio.trackSynth2.sequence[s].active;

                Color stepBg = isStepActive ? Color { 70, 180, 240, 255 } : Color { 25, 34, 50, 255 };
                if (s == activeStep) stepBg = Color { 240, 100, 80, 255 };

                d.filledRect({ sx, sy }, { 9, 12 }, { .color = stepBg });

                if (s == seqStepCursor && isCurrent) {
                    d.rect({ sx - 1, sy - 1 }, { 11, 14 }, { .color = Color { 255, 255, 255, 255 } });
                }
            }
            laneY += 18;
        }

        std::stringstream ss;
        ss << "Lane: " << laneNames[seqCurrentLane] << " | Step: " << (seqStepCursor + 1);
        if (seqCurrentLane == 4) ss << " | Note: " << (int)studio.trackSynth1.sequence[seqStepCursor].note;
        else if (seqCurrentLane == 5) ss << " | Note: " << (int)studio.trackSynth2.sequence[seqStepCursor].note;

        d.text({ 10, 250 }, ss.str().c_str(), 8, { .color = { 180, 200, 230, 255 }, .font = &PoppinsLight_8 });
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

    void getCornerCards(CornerCard cards[4])
    {
        switch (currentView) {
        case VIEW_DRUMS_PAGE1:
            cards[0] = CornerCard("Snare Freq", std::to_string((int)studio.trackDrums.drums.snare.baseFrequency.value) + " Hz", (studio.trackDrums.drums.snare.baseFrequency.value - 30.0f) / 370.0f, "[P1] TRIG");
            cards[1] = CornerCard("Snare Decay", std::to_string((int)studio.trackDrums.drums.snare.bodyDuration.value) + " ms", studio.trackDrums.drums.snare.bodyDuration.value / 800.0f, "[P2] CHH");
            cards[2] = CornerCard("Clap Noise", std::to_string((int)studio.trackDrums.drums.clap.clapNoiseClr.value) + " %", studio.trackDrums.drums.clap.clapNoiseClr.value / 100.0f, "[P3] OHH");
            cards[3] = CornerCard("Clap Decay", std::to_string((int)studio.trackDrums.drums.clap.hiClapDuration.value) + " ms", studio.trackDrums.drums.clap.hiClapDuration.value / 1000.0f, "[P4] CLAP");
            break;

        case VIEW_DRUMS_PAGE2:
            cards[0] = CornerCard("CHH Tight", std::to_string((int)studio.trackDrums.drums.hhClosed.hiTightness.value) + " %", studio.trackDrums.drums.hhClosed.hiTightness.value / 100.0f, "[P1] SNR");
            cards[1] = CornerCard("CHH Decay", std::to_string((int)studio.trackDrums.drums.hhClosed.hiClapDuration.value) + " ms", studio.trackDrums.drums.hhClosed.hiClapDuration.value / 300.0f, "[P2] CHH");
            cards[2] = CornerCard("OHH Tight", std::to_string((int)studio.trackDrums.drums.hhOpen.hiTightness.value) + " %", studio.trackDrums.drums.hhOpen.hiTightness.value / 100.0f, "[P3] OHH");
            cards[3] = CornerCard("OHH Decay", std::to_string((int)studio.trackDrums.drums.hhOpen.hiClapDuration.value) + " ms", studio.trackDrums.drums.hhOpen.hiClapDuration.value / 800.0f, "[P4] CLP");
            break;

        case VIEW_SYNTH1_PAGE1:
            cards[0] = CornerCard("Osc Wave", std::to_string((int)(studio.trackSynth1.engine.waveform.value * 100.0f)) + " %", studio.trackSynth1.engine.waveform.value, "[P1] TRIG");
            cards[1] = CornerCard("Pitch", std::to_string((int)studio.trackSynth1.engine.pitch.value), (studio.trackSynth1.engine.pitch.value - 24.0f) / 48.0f, "[P2] PAGE");
            cards[2] = CornerCard("Cutoff", std::to_string((int)(studio.trackSynth1.engine.cutoff.value * 100.0f)) + " %", studio.trackSynth1.engine.cutoff.value, "[P3] RESET");
            cards[3] = CornerCard("Resonance", std::to_string((int)(studio.trackSynth1.engine.resonance.value * 100.0f)) + " %", studio.trackSynth1.engine.resonance.value, studio.trackSynth1.isMuted ? "[P4] UNMUTE" : "[P4] MUTE");
            break;

        case VIEW_SYNTH1_PAGE2:
            cards[0] = CornerCard("Release", std::to_string((int)studio.trackSynth1.engine.release.value) + " ms", studio.trackSynth1.engine.release.value / 2000.0f, "[P1] TRIG");
            cards[1] = CornerCard("Env Amt", std::to_string((int)(studio.trackSynth1.engine.envAmt.value * 100.0f)) + " %", studio.trackSynth1.engine.envAmt.value, "[P2] PAGE");
            cards[2] = CornerCard("Filter Morph", std::to_string((int)(studio.trackSynth1.engine.filterMorph.value * 100.0f)) + " %", studio.trackSynth1.engine.filterMorph.value, "[P3] RESET");
            cards[3] = CornerCard("Delay Send", std::to_string((int)studio.trackSynth1.engine.delaySend.value) + " %", studio.trackSynth1.engine.delaySend.value / 100.0f, studio.trackSynth1.isMuted ? "[P4] UNMUTE" : "[P4] MUTE");
            break;

        case VIEW_SYNTH1_PAGE3:
            cards[0] = CornerCard("Mod Type", studio.trackSynth1.engine.modTypeName, studio.trackSynth1.engine.modType.value / 15.0f, "[P1] TRIG");
            cards[1] = CornerCard("Mod Depth", std::to_string((int)studio.trackSynth1.engine.modDepth.value) + " %", (studio.trackSynth1.engine.modDepth.value + 100.0f) / 200.0f, "[P2] PAGE");
            cards[2] = CornerCard("Track Vol", std::to_string((int)(studio.trackSynth1.volume * 100.0f)) + " %", studio.trackSynth1.volume, "[P3] RESET");
            cards[3] = CornerCard("Tempo BPM", std::to_string((int)studio.bpm.load()) + " BPM", (studio.bpm.load() - 60.0f) / 140.0f, studio.trackSynth1.isMuted ? "[P4] UNMUTE" : "[P4] MUTE");
            break;

        case VIEW_SYNTH2_PAGE1:
            cards[0] = CornerCard("WT Morph", std::to_string((int)studio.trackSynth2.engine.wavetable.value), (studio.trackSynth2.engine.wavetable.value - 1.0f) / 63.0f, "[P1] TRIG");
            cards[1] = CornerCard("Chord", studio.trackSynth2.engine.chordName, studio.trackSynth2.engine.chord.value / 5.0f, "[P2] PAGE");
            cards[2] = CornerCard("Cutoff", std::to_string((int)(studio.trackSynth2.engine.cutoff.value * 100.0f)) + " %", studio.trackSynth2.engine.cutoff.value, "[P3] RESET");
            cards[3] = CornerCard("Resonance", std::to_string((int)(studio.trackSynth2.engine.resonance.value * 100.0f)) + " %", studio.trackSynth2.engine.resonance.value, studio.trackSynth2.isMuted ? "[P4] UNMUTE" : "[P4] MUTE");
            break;

        case VIEW_SYNTH2_PAGE2:
            cards[0] = CornerCard("Attack", std::to_string((int)studio.trackSynth2.engine.attack.value) + " ms", studio.trackSynth2.engine.attack.value / 4000.0f, "[P1] TRIG");
            cards[1] = CornerCard("Release", std::to_string((int)studio.trackSynth2.engine.release.value) + " ms", studio.trackSynth2.engine.release.value / 8000.0f, "[P2] PAGE");
            cards[2] = CornerCard("Delay Send", std::to_string((int)studio.trackSynth2.engine.delaySend.value) + " %", studio.trackSynth2.engine.delaySend.value / 100.0f, "[P3] RESET");
            cards[3] = CornerCard("Track Vol", std::to_string((int)(studio.trackSynth2.volume * 100.0f)) + " %", studio.trackSynth2.volume, studio.trackSynth2.isMuted ? "[P4] UNMUTE" : "[P4] MUTE");
            break;

        case VIEW_SYNTH2_PAGE3:
            cards[0] = CornerCard("Mod Type", studio.trackSynth2.engine.modTypeName, studio.trackSynth2.engine.modType.value / 16.0f, "[P1] TRIG");
            cards[1] = CornerCard("Mod Depth", std::to_string((int)studio.trackSynth2.engine.modDepth.value) + " %", (studio.trackSynth2.engine.modDepth.value + 100.0f) / 200.0f, "[P2] PAGE");
            cards[2] = CornerCard("Mod Speed", std::to_string((int)studio.trackSynth2.engine.modSpeed.value) + " %", studio.trackSynth2.engine.modSpeed.value / 100.0f, "[P3] RESET");
            cards[3] = CornerCard("Tempo BPM", std::to_string((int)studio.bpm.load()) + " BPM", (studio.bpm.load() - 60.0f) / 140.0f, studio.trackSynth2.isMuted ? "[P4] UNMUTE" : "[P4] MUTE");
            break;

        case VIEW_MASTER_PAGE1:
            cards[0] = CornerCard("Drums Vol", std::to_string((int)(studio.trackDrums.volume * 100.0f)) + " %", studio.trackDrums.volume, studio.trackDrums.isMuted ? "[P1] UNMUTE" : "[P1] MUTE");
            cards[1] = CornerCard("Synth 1 Vol", std::to_string((int)(studio.trackSynth1.volume * 100.0f)) + " %", studio.trackSynth1.volume, studio.trackSynth1.isMuted ? "[P2] UNMUTE" : "[P2] MUTE");
            cards[2] = CornerCard("Synth 2 Vol", std::to_string((int)(studio.trackSynth2.volume * 100.0f)) + " %", studio.trackSynth2.volume, studio.trackSynth2.isMuted ? "[P3] UNMUTE" : "[P3] MUTE");
            cards[3] = CornerCard("Tempo BPM", std::to_string((int)studio.bpm.load()) + " BPM", (studio.bpm.load() - 60.0f) / 140.0f, "[P4] PLAY");
            break;

        case VIEW_MASTER_PAGE2:
            cards[0] = CornerCard("Scatter FX", "Crunch", 0.25f, "[P1] CRUNCH");
            cards[1] = CornerCard("Scatter FX", "Drive", 0.50f, "[P2] DRIVE");
            cards[2] = CornerCard("Scatter FX", "Distort", 0.75f, "[P3] DIST");
            cards[3] = CornerCard("Tempo BPM", std::to_string((int)studio.bpm.load()) + " BPM", (studio.bpm.load() - 60.0f) / 140.0f, "[P4] ACID");
            break;

        case VIEW_SEQUENCER:
            cards[0] = CornerCard("Move Step", "Step " + std::to_string(seqStepCursor + 1), (float)seqStepCursor / 15.0f, "[P1] PREV");
            cards[1] = CornerCard("Track Lane", "Lane " + std::to_string(seqCurrentLane + 1), (float)seqCurrentLane / 5.0f, "[P2] NEXT");
            cards[2] = CornerCard("Toggle Step", "Step " + std::to_string(seqStepCursor + 1), 0.5f, "[P3] TOGGLE");
            cards[3] = CornerCard("Tempo BPM", std::to_string((int)studio.bpm.load()) + " BPM", (studio.bpm.load() - 60.0f) / 140.0f, "[P4] PLAY");
            break;

        default:
            break;
        }
    }
};
