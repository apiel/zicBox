#pragma once

#include "draw/draw.h"
#include "sequenceBrain.h"
#include "audio/engines/drop.h"
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
    SECTION_ACID,
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
    int acidPage = 0; // 0 = first 4 knobs, 1 = remaining 2 knobs

    UiDrop(SequenceBrain& b, Drop& a) : brain(b), audio(a) {
        // 1. Clock / Generator knobs
        knobs.push_back({"BPM", &brain.bpm, 40.0f, 280.0f, 70.0f, 130.0f, 20.0f, false, 0.0f, 0.0f, " bpm", SECTION_BRAIN});
        knobs.push_back({"GEN tribe vel", &brain.kickP1, 0.0f, 1.0f, 150.0f, 130.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_BRAIN});
        knobs.push_back({"GEN ghost", &brain.kickP2, 0.0f, 1.0f, 230.0f, 130.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_BRAIN});
        knobs.push_back({"GEN end rumble", &brain.kickP3, 0.0f, 1.0f, 310.0f, 130.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_BRAIN});

        std::vector<std::string> stepDisplayStrings = {"1", "2", "4", "8", "16", "32"};
        std::vector<std::string> noteCountDisplayStrings = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"};
        std::vector<std::string> arpDisplayStrings = {
            "UP", "DOWN", "UP-DOWN", "DOWN-UP", "RAND", "RAND-NR", "DRUNK", "CONVERGE", "DIVERGE", "U2-D1", 
            "D2-U1", "TRILL-0", "TRILL-M", "STEP-DUP", "SKIP-1", "SKIP-2", "TUR-3", "TUR-5", "OCT-JMP", "STACCATO"
        };

        knobs.push_back({"BASE PITCH", &audio.acidBasePitch.value, 24.0f, 72.0f, 70.0f, 210.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_BRAIN});
        knobs.push_back({"TRIG STEP", &brain.synthTriggerStep, 0.0f, 5.0f, 150.0f, 210.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_BRAIN, 0, stepDisplayStrings});
        knobs.push_back({"NOTE COUNT", &brain.synthNoteCount, 0.0f, 11.0f, 230.0f, 210.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_BRAIN, 0, noteCountDisplayStrings});
        knobs.push_back({"ARP STYLE", &brain.synthArpStyle, 0.0f, 19.0f, 310.0f, 210.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_BRAIN, 0, arpDisplayStrings});

        // 2. Kick knobs
        knobs.push_back({"TUNE", &audio.kickTune.value, 30.0f, 150.0f, 420.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, " Hz", SECTION_KICK});
        knobs.push_back({"DECAY", &audio.kickDecay.value, 30.0f, 1000.0f, 510.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, " ms", SECTION_KICK});
        knobs.push_back({"SWEEP DEP", &audio.kickPitchEnvAmt.value, 0.0f, 150.0f, 600.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_KICK});
        knobs.push_back({"SWEEP LEN", &audio.kickSweepLen.value, 0.0f, 100.0f, 690.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, " %", SECTION_KICK});
        knobs.push_back({"SWEEP SHP", &audio.kickSweepShp.value, 0.0f, 100.0f, 780.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, " %", SECTION_KICK});

        knobs.push_back({"VCO MORPH", &audio.kickVcoMorph.value, 0.0f, 1.0f, 420.0f, 200.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_KICK});
        knobs.push_back({"VCO2 LVL", &audio.kickVco2Level.value, 0.0f, 1.0f, 510.0f, 200.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_KICK});
        knobs.push_back({"VCO2 HARM", &audio.kickVco2Harm.value, 1.0f, 12.0f, 600.0f, 200.0f, 22.0f, false, 0.0f, 0.0f, "x", SECTION_KICK});
        knobs.push_back({"VCO2 MRP", &audio.kickVco2Morph.value, 0.0f, 1.0f, 690.0f, 200.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_KICK});
        knobs.push_back({"CLICK AMT", &audio.kickClickAmt.value, 0.0f, 1.0f, 780.0f, 200.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_KICK});

        knobs.push_back({"CLICK DEC", &audio.kickClickDecay.value, 2.0f, 200.0f, 420.0f, 280.0f, 22.0f, false, 0.0f, 0.0f, " ms", SECTION_KICK});
        knobs.push_back({"KICK DRV", &audio.kickDrive.value, 0.0f, 1.0f, 510.0f, 280.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_KICK});
        knobs.push_back({"KICK SHP", &audio.kickWaveshape.value, 0.0f, 1.0f, 600.0f, 280.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_KICK});
        knobs.push_back({"KICK COMP", &audio.kickCompress.value, 0.0f, 1.0f, 690.0f, 280.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_KICK});
        knobs.push_back({"KICK CLIP", &audio.kickClipping.value, 0.0f, 1.0f, 780.0f, 280.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_KICK});

        // 4. Acid / Drone knobs
        knobs.push_back({"CUTOFF", &audio.acidCutoff.value, 0.02f, 0.98f, 65.0f, 385.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_ACID, 0});
        knobs.push_back({"RESO", &audio.acidResonance.value, 0.0f, 0.99f, 145.0f, 385.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_ACID, 0});
        knobs.push_back({"GLIDE", &audio.acidGlide.value, 0.0f, 600.0f, 225.0f, 385.0f, 22.0f, false, 0.0f, 0.0f, " ms", SECTION_ACID, 0});
        knobs.push_back({"WAVE", &audio.acidWaveform.value, 0.0f, 1.0f, 305.0f, 385.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_ACID, 0});

        knobs.push_back({"DEC", &audio.acidDecay.value, 10.0f, 1000.0f, 65.0f, 465.0f, 22.0f, false, 0.0f, 0.0f, " ms", SECTION_ACID, 0});
        knobs.push_back({"ENV AMT", &audio.acidEnvAmt.value, 0.0f, 1.0f, 145.0f, 465.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_ACID, 0});

        std::vector<std::string> modDisplayStrings = {
            "ENV Cutoff", "ENV Pitch", "ENV Wave", 
            "LFO Tri Cut", "LFO Tri Pit", "LFO Tri Wave", "LFO Tri Lvl", 
            "LFO Saw Cut", "LFO Saw Pit", "LFO Saw Wave", 
            "LFO S&H Cut", "LFO S&H Pit"
        };
        knobs.push_back({"MOD TYPE", &audio.acidModType.value, 0.0f, 11.0f, 225.0f, 465.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_ACID, 0, modDisplayStrings});
        knobs.push_back({"MOD DEPTH", &audio.acidModDepth.value, -100.0f, 100.0f, 305.0f, 465.0f, 22.0f, false, 0.0f, 0.0f, " %", SECTION_ACID, 0});

        knobs.push_back({"MOD SPEED", &audio.acidModSpeed.value, 0.0f, 100.0f, 65.0f, 545.0f, 22.0f, false, 0.0f, 0.0f, " %", SECTION_ACID, 0});
        knobs.push_back({"DLY MIX", &audio.acidDelayMix.value, 0.0f, 1.0f, 145.0f, 545.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_ACID, 0});
        knobs.push_back({"DLY TIME", &audio.acidDelayTime.value, 10.0f, 1000.0f, 225.0f, 545.0f, 22.0f, false, 0.0f, 0.0f, " ms", SECTION_ACID, 0});
        knobs.push_back({"DLY FEED", &audio.acidDelayFeedback.value, 0.0f, 0.95f, 305.0f, 545.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_ACID, 0});

        // 5. Master / Slices knobs
        knobs.push_back({"KICK LVL", &audio.kickLevel.value, 0.0f, 1.0f, 450.0f, 440.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_MASTER});
        knobs.push_back({"SYNTH LVL", &audio.synthLevel.value, 0.0f, 1.0f, 550.0f, 440.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_MASTER});
        knobs.push_back({"SAT DRIVE", &audio.masterDrive.value, 0.0f, 1.0f, 650.0f, 440.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_MASTER});
        knobs.push_back({"VOLUME", &audio.masterVolume.value, 0.0f, 1.0f, 750.0f, 440.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_MASTER});
    }

    void checkRegen(Knob& k, float oldVal, float newVal) {
        if (oldVal == newVal) return;
        if (k.label == "K.GEN 1" || k.label == "K.GEN 2" || k.label == "K.GEN 3") {
            brain.regenerateKick();
        }
    }

    bool handleMouseButtonPressed(float mx, float my) {
        // Detect section click to focus
        if (mx >= 15.0f && mx <= 355.0f && my >= 15.0f && my <= 325.0f) activeSection = SECTION_BRAIN;
        else if (mx >= 370.0f && mx <= 820.0f && my >= 15.0f && my <= 325.0f) activeSection = SECTION_KICK;
        else if (mx >= 835.0f && mx <= 965.0f && my >= 15.0f && my <= 325.0f) activeSection = SECTION_NOISE;
        else if (mx >= 15.0f && mx <= 355.0f && my >= 345.0f && my <= 585.0f) activeSection = SECTION_ACID;
        else if (mx >= 370.0f && mx <= 965.0f && my >= 345.0f && my <= 585.0f) activeSection = SECTION_MASTER;

        for (auto& k : knobs) {
            if (k.section == SECTION_ACID && k.page != acidPage) {
                continue;
            }
            float dx = mx - k.x;
            float dy = my - k.y;
            if (std::sqrt(dx*dx + dy*dy) <= k.radius + 5.0f) {
                k.isDragging = true;
                k.dragStartY = my;
                k.startVal = *(k.value);
                activeKnob = &k;
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
            if (k.section == SECTION_ACID && k.page != acidPage) {
                continue;
            }
            float dx = mx - k.x;
            float dy = my - k.y;
            if (std::sqrt(dx*dx + dy*dy) <= k.radius + 5.0f) {
                float range = k.maxVal - k.minVal;
                float step = 0.02f * range;
                float oldVal = *(k.value);
                float newVal = oldVal + delta * step;
                newVal = std::clamp(newVal, k.minVal, k.maxVal);
                *(k.value) = newVal;
                checkRegen(k, oldVal, newVal);
                return true;
            }
        }
        return false;
    }

    void handleEncoder(int encoderId, int direction) {
        // Find knobs belonging to activeSection
        std::vector<Knob*> sectionKnobs;
        for (auto& k : knobs) {
            if (k.section == activeSection) {
                if (k.section == SECTION_ACID && k.page != acidPage) {
                    continue;
                }
                sectionKnobs.push_back(&k);
            }
        }

        int idx = encoderId - 1;
        if (idx >= 0 && idx < (int)sectionKnobs.size()) {
            Knob* k = sectionKnobs[idx];
            float range = k->maxVal - k->minVal;
            float step = 0.02f * range * direction;
            float oldVal = *(k->value);
            float newVal = oldVal + step;
            newVal = std::clamp(newVal, k->minVal, k->maxVal);
            *(k->value) = newVal;
            checkRegen(*k, oldVal, newVal);
        }
    }

    void drawKnob(Draw& d, const Knob& k) {
        // Outer dark ring
        Color ringCol = { 80, 80, 100, 255 };
        if (k.section == activeSection) {
            ringCol = { 0, 195, 255, 255 };
        }
        d.circle({ (int)k.x, (int)k.y }, (int)k.radius, { .color = ringCol });

        // Value pointer line
        float pct = (*(k.value) - k.minVal) / (k.maxVal - k.minVal);
        float angle = -135.0f + pct * 270.0f;
        float rad = (angle - 90.0f) * M_PI / 180.0f;

        d.line(
            { (int)k.x, (int)k.y }, 
            { (int)(k.x + std::cos(rad) * k.radius), (int)(k.y + std::sin(rad) * k.radius) }, 
            { .color = { 0, 255, 170, 255 } }
        );

        // Text rendering
        d.textCentered({ (int)k.x, (int)(k.y - k.radius - 12.0f) }, k.label, 8, { .color = { 180, 180, 190, 255 }, .font = &PoppinsLight_8 });

        std::stringstream ss;
        if (!k.displayStrings.empty()) {
            int idx = std::clamp((int)std::round(*(k.value)), 0, (int)k.displayStrings.size() - 1);
            ss << k.displayStrings[idx];
        } else {
            ss << std::fixed << std::setprecision(2) << *(k.value) << k.unit;
        }
        d.textCentered({ (int)k.x, (int)(k.y + k.radius + 12.0f) }, ss.str(), 8, { .color = { 140, 150, 160, 255 }, .font = &PoppinsLight_8 });
    }

    bool draw(Draw& d, const int winW, const int winH, bool& needFullRedraw, const SequenceBrain& brain, const Drop& audio) {
        if (needFullRedraw) {
            d.clear();
        }

        // Draw Panels
        // Panel 1: Brain
        Color brainOutline = (activeSection == SECTION_BRAIN) ? Color{ 0, 195, 255, 255 } : Color{ 45, 45, 55, 255 };
        d.filledRect({ 15, 15 }, { 340, 310 }, { .color = { 20, 20, 25, 255 } });
        d.rect({ 15, 15 }, { 340, 310 }, { .color = brainOutline });

        // Panel 2: Kick
        Color kickOutline = (activeSection == SECTION_KICK) ? Color{ 255, 100, 100, 255 } : Color{ 60, 45, 50, 255 };
        d.filledRect({ 370, 15 }, { 450, 310 }, { .color = { 25, 20, 22, 255 } });
        d.rect({ 370, 15 }, { 450, 310 }, { .color = kickOutline });

        // Panel 4: Acid
        Color acidOutline = (activeSection == SECTION_ACID) ? Color{ 230, 230, 80, 255 } : Color{ 55, 55, 45, 255 };
        d.filledRect({ 15, 345 }, { 340, 240 }, { .color = { 24, 24, 20, 255 } });
        d.rect({ 15, 345 }, { 340, 240 }, { .color = acidOutline });

        // Panel 5: Master
        Color masterOutline = (activeSection == SECTION_MASTER) ? Color{ 255, 120, 0, 255 } : Color{ 65, 45, 45, 255 };
        d.filledRect({ 370, 345 }, { 595, 240 }, { .color = { 28, 20, 20, 255 } });
        d.rect({ 370, 345 }, { 595, 240 }, { .color = masterOutline });

        // Headers
        d.text({ 25, 25 }, "GENERATIVE SEQUENCE BRAIN", 12, { .color = { 0, 195, 255, 255 }, .font = &PoppinsLight_12 });
        d.text({ 380, 25 }, "FAT KICK ENGINE", 12, { .color = { 255, 100, 100, 255 }, .font = &PoppinsLight_12 });
        
        d.text({ 25, 355 }, "ACID SYNTH", 12, { .color = { 230, 230, 80, 255 }, .font = &PoppinsLight_12 });
        d.text({ 380, 355 }, "MASTER SLICES / OVERRIDE", 12, { .color = { 255, 120, 0, 255 }, .font = &PoppinsLight_12 });

        // Shift Register Visualizer
        for (int i = 0; i < 16; ++i) {
            bool bitOn = (brain.shiftRegister & (1 << (15 - i))) != 0;
            Color fill = bitOn ? Color{ 0, 255, 170, 255 } : Color{ 40, 40, 50, 255 };
            Color outline = bitOn ? Color{ 255, 255, 255, 255 } : Color{ 60, 60, 75, 255 };
            d.filledRect({ 35 + i * 19, 60 }, { 14, 14 }, { .color = fill });
            d.rect({ 35 + i * 19, 60 }, { 14, 14 }, { .color = outline });
        }

        // Performance touchpad
        Color touchpadFill = brain.spacebarHeld ? Color{ 255, 60, 0, 255 } : Color{ 80, 20, 20, 255 };
        Color touchpadBorder = brain.spacebarHeld ? Color{ 255, 255, 255, 255 } : Color{ 160, 40, 40, 255 };
        d.filledRect({ 510, 530 }, { 300, 30 }, { .color = touchpadFill });
        d.rect({ 510, 530 }, { 300, 30 }, { .color = touchpadBorder });
        d.textCentered({ 660, 538 }, "SPACEBAR CLICK ONLY", 12, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_12 });

        // Draw Knobs
        for (const auto& k : knobs) {
            if (k.section == SECTION_ACID && k.page != acidPage) {
                continue;
            }
            drawKnob(d, k);
        }

        return true;
    }
};
