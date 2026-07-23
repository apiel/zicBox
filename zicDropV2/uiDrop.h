#pragma once

#include "draw/draw.h"
#include "sequenceBrain.h"
#include "audio/engines/drop2.h"
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

enum FocusSection {
    SECTION_BRAIN,
    SECTION_KICK,
    SECTION_NOISE,
    SECTION_SYNTH,
    SECTION_MASTER
};

struct Knob {
    std::string label;
    float* value;
    float minVal;
    float maxVal;
    float x, y;
    float radius = 22.0f;
    bool isDragging = false;
    float dragStartY = 0.0f;
    float startVal = 0.0f;
    std::string unit = "";
    FocusSection section;
    int page = 0;
    std::vector<std::string> displayStrings = {};

    Knob(std::string lbl, float* val, float minV, float maxV, float xx, float yy, float rad, bool isDrag, float dragY, float startV, std::string u, FocusSection sec, int pg = 0, std::vector<std::string> dispStrs = {})
        : label(lbl), value(val), minVal(minV), maxVal(maxV), x(xx), y(yy), radius(rad), isDragging(isDrag), dragStartY(dragY), startVal(startV), unit(u), section(sec), page(pg), displayStrings(dispStrs) {}
};

class UiDrop {
private:
    SequenceBrain& brain;
    Drop& audio;

public:
    std::vector<Knob> knobs;
    Knob* activeKnob = nullptr;
    FocusSection activeSection = SECTION_BRAIN;

    UiDrop(SequenceBrain& b, Drop& a) : brain(b), audio(a) {
        std::vector<std::string> stepDisplayStrings = {
            "follow", "1", "2", "2-off", "4", "4-off", "4-rumble", "8", "8-off", "8-rumble", "16", "16-off", "16-rumble", "32", "32-off", "32-rumble"
        };
        std::vector<std::string> modDisplayStrings = {
            "ENV Cutoff", "ENV Pitch", "ENV Wave", 
            "LFO Tri Cut", "LFO Tri Pit", "LFO Tri Wave", "LFO Tri Lvl", 
            "LFO Saw Cut", "LFO Saw Pit", "LFO Saw Wave", 
            "LFO S&H Cut", "LFO S&H Pit"
        };

        // =========================================================================
        // ROW 1 (Y = 115 px): Brain & Kick Sub Core (Knobs 1 to 8)
        // =========================================================================
        knobs.push_back({"BPM", &brain.bpm, 40.0f, 280.0f, 75.0f, 115.0f, 20.0f, false, 0.0f, 0.0f, " bpm", SECTION_BRAIN});
        knobs.push_back({"GEN KICK", &brain.kickGenParam, 0.0f, 1.0f, 199.0f, 115.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_BRAIN});
        knobs.push_back({"BASE PITCH", &audio.synthBasePitch.value, 24.0f, 72.0f, 323.0f, 115.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_BRAIN});
        knobs.push_back({"TRIG STEP", &brain.synthTriggerStep, 0.0f, 15.0f, 447.0f, 115.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_BRAIN, 0, stepDisplayStrings});
        knobs.push_back({"TUNE", &audio.kickTune.value, 30.0f, 150.0f, 571.0f, 115.0f, 20.0f, false, 0.0f, 0.0f, " Hz", SECTION_KICK});
        knobs.push_back({"DECAY", &audio.kickDecay.value, 30.0f, 2500.0f, 695.0f, 115.0f, 20.0f, false, 0.0f, 0.0f, " ms", SECTION_KICK});
        knobs.push_back({"SWEEP DEP", &audio.kickPitchEnvAmt.value, 0.0f, 150.0f, 819.0f, 115.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_KICK});
        knobs.push_back({"SWEEP LEN", &audio.kickSweepLen.value, 0.0f, 100.0f, 943.0f, 115.0f, 20.0f, false, 0.0f, 0.0f, " %", SECTION_KICK});

        // =========================================================================
        // ROW 2 (Y = 215 px): Kick Click, Drive, Rumble & Master Mix/Vol (Knobs 9 to 16)
        // =========================================================================
        knobs.push_back({"VCO MORPH", &audio.kickVcoMorph.value, 0.0f, 1.0f, 75.0f, 215.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_KICK});
        knobs.push_back({"CLICK AMT", &audio.kickClickAmt.value, 0.0f, 1.0f, 199.0f, 215.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_KICK});
        knobs.push_back({"CLICK DEC", &audio.kickClickDecay.value, 2.0f, 200.0f, 323.0f, 215.0f, 20.0f, false, 0.0f, 0.0f, " ms", SECTION_KICK});
        knobs.push_back({"KICK DRV", &audio.kickDrive.value, 0.0f, 1.0f, 447.0f, 215.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_KICK});
        knobs.push_back({"RUMBLE", &audio.rumbleAmt.value, 0.0f, 100.0f, 571.0f, 215.0f, 20.0f, false, 0.0f, 0.0f, " %", SECTION_KICK});
        knobs.push_back({"RUM. GAP", &audio.rumbleGap.value, 10.0f, 400.0f, 695.0f, 215.0f, 20.0f, false, 0.0f, 0.0f, " ms", SECTION_KICK});
        knobs.push_back({"TEX MIX", &audio.mix.value, 0.0f, 1.0f, 819.0f, 215.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_MASTER});
        knobs.push_back({"VOLUME", &audio.masterVolume.value, 0.0f, 1.0f, 943.0f, 215.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_MASTER});

        // =========================================================================
        // ROW 3 (Y = 315 px): Texture Synth Core & Wave Shaping (Knobs 17 to 24)
        // =========================================================================
        knobs.push_back({"CUTOFF", &audio.synthCutoff.value, 0.02f, 0.98f, 75.0f, 315.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH});
        knobs.push_back({"RESO", &audio.synthResonance.value, 0.0f, 0.99f, 199.0f, 315.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH});
        knobs.push_back({"WAVE", &audio.synthWaveform.value, 0.0f, 1.0f, 323.0f, 315.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH});
        knobs.push_back({"REL", &audio.synthRelease.value, 10.0f, 2000.0f, 447.0f, 315.0f, 20.0f, false, 0.0f, 0.0f, " ms", SECTION_SYNTH});
        knobs.push_back({"ENV AMT", &audio.synthEnvAmt.value, 0.0f, 1.0f, 571.0f, 315.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH});
        knobs.push_back({"FILT MORPH", &audio.synthFilterMorph.value, 0.0f, 1.0f, 695.0f, 315.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH});
        knobs.push_back({"SYNTH SHP", &audio.synthWaveshape.value, 0.0f, 1.0f, 819.0f, 315.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH});
        knobs.push_back({"TEX DRV", &audio.synthDrive.value, 0.0f, 1.0f, 943.0f, 315.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH});

        // =========================================================================
        // ROW 4 (Y = 415 px): Modulation, FM, Delay & Master Distortion (Knobs 25 to 32)
        // =========================================================================
        knobs.push_back({"MOD TYPE", &audio.synthModType.value, 0.0f, 11.0f, 75.0f, 415.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH, 0, modDisplayStrings});
        knobs.push_back({"MOD DEPTH", &audio.synthModDepth.value, -100.0f, 100.0f, 199.0f, 415.0f, 20.0f, false, 0.0f, 0.0f, " %", SECTION_SYNTH});
        knobs.push_back({"MOD SPEED", &audio.synthModSpeed.value, 0.0f, 100.0f, 323.0f, 415.0f, 20.0f, false, 0.0f, 0.0f, " %", SECTION_SYNTH});
        knobs.push_back({"FM MORPH", &audio.synthFmAmt.value, 0.0f, 1.0f, 447.0f, 415.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH});
        knobs.push_back({"DLY MIX", &audio.synthDelayMix.value, 0.0f, 1.0f, 571.0f, 415.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH});
        knobs.push_back({"DLY TIME", &audio.synthDelayTime.value, 10.0f, 1000.0f, 695.0f, 415.0f, 20.0f, false, 0.0f, 0.0f, " ms", SECTION_SYNTH});
        knobs.push_back({"DLY FEED", &audio.synthDelayFeedback.value, 0.0f, 0.95f, 819.0f, 415.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH});
        knobs.push_back({"HARD FOLD", &audio.mstFold.value, 0.0f, 1.0f, 943.0f, 415.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_MASTER});
    }

    void checkRegen(Knob& k, float oldVal, float newVal) {
        if (oldVal == newVal) return;
        if (k.label == "GEN KICK" || k.label == "GEN RUMBLE" || k.label == "GEN GHOST") {
            brain.regenerateKick();
        }
    }

    bool handleMouseButtonPressed(float mx, float my) {
        for (auto& k : knobs) {
            float dx = mx - k.x;
            float dy = my - k.y;
            if (std::sqrt(dx*dx + dy*dy) <= k.radius + 5.0f) {
                k.isDragging = true;
                k.dragStartY = my;
                k.startVal = *(k.value);
                activeKnob = &k;
                activeSection = k.section;
                return true;
            }
        }
        return false;
    }

    bool handleMouseMoved(float mx, float my) {
        if (activeKnob && activeKnob->isDragging) {
            float deltaY = activeKnob->dragStartY - my;
            float range = activeKnob->maxVal - activeKnob->minVal;
            float sensitivity = 0.005f * range;
            float newVal = activeKnob->startVal + deltaY * sensitivity;
            newVal = std::clamp(newVal, activeKnob->minVal, activeKnob->maxVal);
            float oldVal = *(activeKnob->value);
            *(activeKnob->value) = newVal;
            checkRegen(*activeKnob, oldVal, newVal);
            return true;
        }
        return false;
    }

    bool handleMouseButtonReleased() {
        if (activeKnob) {
            activeKnob->isDragging = false;
            activeKnob = nullptr;
            return true;
        }
        return false;
    }

    bool handleMouseWheel(float mx, float my, float delta) {
        for (auto& k : knobs) {
            float dx = mx - k.x;
            float dy = my - k.y;
            if (std::sqrt(dx*dx + dy*dy) <= k.radius + 5.0f) {
                float range = k.maxVal - k.minVal;
                float step = 0.02f * range;
                float oldVal = *(k.value);
                float newVal = oldVal + delta * step;
                newVal = std::clamp(newVal, k.minVal, k.maxVal);
                *(k.value) = newVal;
                activeSection = k.section;
                checkRegen(k, oldVal, newVal);
                return true;
            }
        }
        return false;
    }

    void handleEncoder(int encoderId, int direction) {
        int idx = encoderId - 1;
        if (idx >= 0 && idx < (int)knobs.size()) {
            Knob* k = &knobs[idx];
            float range = k->maxVal - k->minVal;
            float step = 0.02f * range * direction;
            float oldVal = *(k->value);
            float newVal = oldVal + step;
            newVal = std::clamp(newVal, k->minVal, k->maxVal);
            *(k->value) = newVal;
            activeSection = k->section;
            checkRegen(*k, oldVal, newVal);
        }
    }

    void drawKnob(Draw& d, const Knob& k) {
        // Distinct Category Background Tiles
        Color tileFill = { 25, 25, 30, 255 };
        Color tileBorder = { 45, 45, 55, 255 };
        Color ringCol = { 80, 80, 100, 255 };

        if (k.section == SECTION_BRAIN) {
            tileFill = { 14, 28, 42, 255 };
            tileBorder = { 0, 120, 170, 255 };
            ringCol = (k.section == activeSection) ? Color{ 0, 210, 255, 255 } : Color{ 0, 150, 200, 255 };
        } else if (k.section == SECTION_KICK) {
            tileFill = { 42, 18, 22, 255 };
            tileBorder = { 170, 50, 60, 255 };
            ringCol = (k.section == activeSection) ? Color{ 255, 100, 110, 255 } : Color{ 200, 70, 80, 255 };
        } else if (k.section == SECTION_SYNTH) {
            tileFill = { 36, 34, 16, 255 };
            tileBorder = { 160, 150, 40, 255 };
            ringCol = (k.section == activeSection) ? Color{ 240, 220, 70, 255 } : Color{ 190, 170, 50, 255 };
        } else if (k.section == SECTION_MASTER) {
            tileFill = { 42, 24, 16, 255 };
            tileBorder = { 180, 85, 25, 255 };
            ringCol = (k.section == activeSection) ? Color{ 255, 140, 30, 255 } : Color{ 210, 100, 30, 255 };
        }

        // Draw Category Tile
        int tx = (int)k.x - 56;
        int ty = (int)k.y - 42;
        d.filledRect({ tx, ty }, { 112, 84 }, { .color = tileFill });
        d.rect({ tx, ty }, { 112, 84 }, { .color = tileBorder });

        // Draw Knob Circle
        d.circle({ (int)k.x, (int)k.y }, (int)k.radius, { .color = ringCol });

        float pct = (*(k.value) - k.minVal) / (k.maxVal - k.minVal);
        float angle = -135.0f + pct * 270.0f;
        float rad = (angle - 90.0f) * M_PI / 180.0f;

        d.line(
            { (int)k.x, (int)k.y }, 
            { (int)(k.x + std::cos(rad) * k.radius), (int)(k.y + std::sin(rad) * k.radius) }, 
            { .color = { 0, 255, 170, 255 } }
        );

        d.textCentered({ (int)k.x, (int)(k.y - k.radius - 10.0f) }, k.label, 8, { .color = { 220, 220, 230, 255 }, .font = &PoppinsLight_8 });

        std::stringstream ss;
        if (!k.displayStrings.empty()) {
            int idx = std::clamp((int)std::round(*(k.value)), 0, (int)k.displayStrings.size() - 1);
            ss << k.displayStrings[idx];
        } else {
            ss << std::fixed << std::setprecision(2) << *(k.value) << k.unit;
        }
        d.textCentered({ (int)k.x, (int)(k.y + k.radius + 10.0f) }, ss.str(), 8, { .color = { 170, 180, 190, 255 }, .font = &PoppinsLight_8 });
    }

    bool draw(Draw& d, const int winW, const int winH, bool& needFullRedraw, const SequenceBrain& brain, const Drop& audio) {
        if (needFullRedraw) {
            d.clear();
        }

        // Draw Main Controller Chassis
        d.filledRect({ 10, 10 }, { 1100, 495 }, { .color = { 15, 15, 18, 255 } });
        d.rect({ 10, 10 }, { 1100, 495 }, { .color = { 45, 45, 55, 255 } });

        // Controller Header
        d.text({ 20, 22 }, "zicDrop 2.0  |  32-ENCODER HARDWARE SYSTEM", 12, { .color = { 0, 195, 255, 255 }, .font = &PoppinsLight_12 });

        // Shift Register Visualizer
        for (int i = 0; i < 16; ++i) {
            bool bitOn = (brain.shiftRegister & (1 << (15 - i))) != 0;
            Color fill = bitOn ? Color{ 0, 255, 170, 255 } : Color{ 35, 35, 45, 255 };
            Color outline = bitOn ? Color{ 255, 255, 255, 255 } : Color{ 55, 55, 70, 255 };
            d.filledRect({ 380 + i * 11, 22 }, { 8, 12 }, { .color = fill });
            d.rect({ 380 + i * 11, 22 }, { 8, 12 }, { .color = outline });
        }

        // Draw Synth Waveform Visualizer
        int wfX = 580;
        int wfY = 16;
        int wfW = 160;
        int wfH = 26;
        d.filledRect({ wfX, wfY }, { wfW, wfH }, { .color = { 20, 20, 15, 255 } });
        d.rect({ wfX, wfY }, { wfW, wfH }, { .color = { 60, 60, 50, 255 } });

        std::vector<Point> points;
        int centerY = wfY + (wfH / 2);
        float amplitude = wfH * 0.40f;

        for (int i = 0; i < wfW; i++) {
            float phase = (float)i / (float)wfW;
            float sample = const_cast<Drop&>(audio).draw(phase);
            int drawY = centerY - (int)(sample * amplitude);
            points.push_back({ wfX + i, drawY });
        }
        d.lines(points, { .color = { 230, 230, 80, 255 } });

        // Performance touchpad
        Color touchpadFill = brain.spacebarHeld ? Color{ 255, 60, 0, 255 } : Color{ 70, 20, 20, 255 };
        Color touchpadBorder = brain.spacebarHeld ? Color{ 255, 255, 255, 255 } : Color{ 150, 40, 40, 255 };
        d.filledRect({ 870, 16 }, { 220, 26 }, { .color = touchpadFill });
        d.rect({ 870, 16 }, { 220, 26 }, { .color = touchpadBorder });
        d.textCentered({ 980, 22 }, "SPACEBAR CLICK ONLY", 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

        for (const auto& k : knobs) {
            drawKnob(d, k);
        }

        return true;
    }
};
