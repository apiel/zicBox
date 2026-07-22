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
        // 1. Clock / Generator knobs
        knobs.push_back({"BPM", &brain.bpm, 40.0f, 280.0f, 80.0f, 140.0f, 20.0f, false, 0.0f, 0.0f, " bpm", SECTION_BRAIN});
        knobs.push_back({"GEN tribe vel", &brain.kickP1, 0.0f, 1.0f, 190.0f, 140.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_BRAIN});
        knobs.push_back({"GEN ghost", &brain.kickP2, 0.0f, 1.0f, 80.0f, 230.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_BRAIN});
        knobs.push_back({"GEN end rumble", &brain.kickP3, 0.0f, 1.0f, 190.0f, 230.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_BRAIN});

        std::vector<std::string> stepDisplayStrings = {
            "follow", "1", "2", "2-off", "4", "4-off", "4-rumble", "8", "8-off", "8-rumble", "16", "16-off", "16-rumble", "32", "32-off", "32-rumble"
        };

        knobs.push_back({"BASE PITCH", &audio.synthBasePitch.value, 24.0f, 72.0f, 80.0f, 320.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_BRAIN});
        knobs.push_back({"TRIG STEP", &brain.synthTriggerStep, 0.0f, 15.0f, 190.0f, 320.0f, 20.0f, false, 0.0f, 0.0f, "", SECTION_BRAIN, 0, stepDisplayStrings});

        // 2. Kick knobs (VCO2 removed, Texture replaces VCO2!)
        knobs.push_back({"TUNE", &audio.kickTune.value, 30.0f, 150.0f, 320.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, " Hz", SECTION_KICK});
        knobs.push_back({"DECAY", &audio.kickDecay.value, 30.0f, 2500.0f, 410.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, " ms", SECTION_KICK});
        knobs.push_back({"SWEEP DEP", &audio.kickPitchEnvAmt.value, 0.0f, 150.0f, 500.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_KICK});
        knobs.push_back({"SWEEP LEN", &audio.kickSweepLen.value, 0.0f, 100.0f, 590.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, " %", SECTION_KICK});
        knobs.push_back({"SWEEP SHP", &audio.kickSweepShp.value, 0.0f, 100.0f, 680.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, " %", SECTION_KICK});

        knobs.push_back({"VCO MORPH", &audio.kickVcoMorph.value, 0.0f, 1.0f, 320.0f, 190.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_KICK});
        knobs.push_back({"CLICK AMT", &audio.kickClickAmt.value, 0.0f, 1.0f, 410.0f, 190.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_KICK});
        knobs.push_back({"CLICK DEC", &audio.kickClickDecay.value, 2.0f, 200.0f, 500.0f, 190.0f, 22.0f, false, 0.0f, 0.0f, " ms", SECTION_KICK});
        knobs.push_back({"KICK DRV", &audio.kickDrive.value, 0.0f, 1.0f, 590.0f, 190.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_KICK});

        knobs.push_back({"RUMBLE", &audio.rumbleAmt.value, 0.0f, 100.0f, 320.0f, 280.0f, 22.0f, false, 0.0f, 0.0f, " %", SECTION_KICK});
        knobs.push_back({"RUM. GAP", &audio.rumbleGap.value, 10.0f, 400.0f, 410.0f, 280.0f, 22.0f, false, 0.0f, 0.0f, " ms", SECTION_KICK});
        knobs.push_back({"RUM. LP", &audio.rumbleFilter.value, 0.0f, 100.0f, 500.0f, 280.0f, 22.0f, false, 0.0f, 0.0f, " %", SECTION_KICK});

        // 3. Texture / Synth knobs (Streamlined)
        knobs.push_back({"CUTOFF", &audio.synthCutoff.value, 0.02f, 0.98f, 915.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH, 0});
        knobs.push_back({"RESO", &audio.synthResonance.value, 0.0f, 0.99f, 1020.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH, 0});
        knobs.push_back({"WAVE", &audio.synthWaveform.value, 0.0f, 1.0f, 1125.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH, 0});
        knobs.push_back({"REL", &audio.synthRelease.value, 10.0f, 2000.0f, 1230.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, " ms", SECTION_SYNTH, 0});
        knobs.push_back({"ENV AMT", &audio.synthEnvAmt.value, 0.0f, 1.0f, 1335.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH, 0});

        std::vector<std::string> modDisplayStrings = {
            "ENV Cutoff", "ENV Pitch", "ENV Wave", 
            "LFO Tri Cut", "LFO Tri Pit", "LFO Tri Wave", "LFO Tri Lvl", 
            "LFO Saw Cut", "LFO Saw Pit", "LFO Saw Wave", 
            "LFO S&H Cut", "LFO S&H Pit"
        };
        knobs.push_back({"MOD TYPE", &audio.synthModType.value, 0.0f, 11.0f, 915.0f, 190.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH, 0, modDisplayStrings});
        knobs.push_back({"MOD DEPTH", &audio.synthModDepth.value, -100.0f, 100.0f, 1020.0f, 190.0f, 22.0f, false, 0.0f, 0.0f, " %", SECTION_SYNTH, 0});
        knobs.push_back({"MOD SPEED", &audio.synthModSpeed.value, 0.0f, 100.0f, 1125.0f, 190.0f, 22.0f, false, 0.0f, 0.0f, " %", SECTION_SYNTH, 0});
        knobs.push_back({"TEX DRV", &audio.synthDrive.value, 0.0f, 1.0f, 1230.0f, 190.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH, 0});
        knobs.push_back({"SYNTH SHP", &audio.synthWaveshape.value, 0.0f, 1.0f, 1335.0f, 190.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH, 0});
        knobs.push_back({"DLY MIX", &audio.synthDelayMix.value, 0.0f, 1.0f, 915.0f, 280.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH, 0});
        knobs.push_back({"DLY TIME", &audio.synthDelayTime.value, 10.0f, 1000.0f, 1020.0f, 280.0f, 22.0f, false, 0.0f, 0.0f, " ms", SECTION_SYNTH, 0});
        knobs.push_back({"DLY FEED", &audio.synthDelayFeedback.value, 0.0f, 0.95f, 1125.0f, 280.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH, 0});
        knobs.push_back({"FM AMT", &audio.synthFmAmt.value, 0.0f, 5.0f, 1230.0f, 280.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH, 0});
        knobs.push_back({"FM RATIO", &audio.synthFmRatio.value, 1.0f, 8.0f, 1335.0f, 280.0f, 22.0f, false, 0.0f, 0.0f, "x", SECTION_SYNTH, 0});
        knobs.push_back({"FILT MORPH", &audio.synthFilterMorph.value, 0.0f, 1.0f, 915.0f, 370.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_SYNTH, 0});

        // 4. Master knobs
        knobs.push_back({"TEX MIX", &audio.mix.value, 0.0f, 1.0f, 1730.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_MASTER});
        knobs.push_back({"SAT DRIVE", &audio.masterDrive.value, 0.0f, 1.0f, 1840.0f, 100.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_MASTER});
        knobs.push_back({"VOLUME", &audio.masterVolume.value, 0.0f, 1.0f, 1730.0f, 190.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_MASTER});
        knobs.push_back({"SCREAM", &audio.mstScream.value, 0.0f, 1.0f, 1840.0f, 190.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_MASTER});
        knobs.push_back({"FOLD", &audio.mstFold.value, 0.0f, 1.0f, 1730.0f, 280.0f, 22.0f, false, 0.0f, 0.0f, "", SECTION_MASTER});
    }

    void checkRegen(Knob& k, float oldVal, float newVal) {
        if (oldVal == newVal) return;
        if (k.label == "GEN tribe vel" || k.label == "GEN ghost" || k.label == "GEN end rumble") {
            brain.regenerateKick();
        }
    }

    bool handleMouseButtonPressed(float mx, float my) {
        if (mx >= 15.0f && mx <= 255.0f && my >= 15.0f && my <= 465.0f) activeSection = SECTION_BRAIN;
        else if (mx >= 270.0f && mx <= 850.0f && my >= 15.0f && my <= 465.0f) activeSection = SECTION_KICK;
        else if (mx >= 865.0f && mx <= 1650.0f && my >= 15.0f && my <= 465.0f) activeSection = SECTION_SYNTH;
        else if (mx >= 1665.0f && mx <= 1905.0f && my >= 15.0f && my <= 465.0f) activeSection = SECTION_MASTER;

        for (auto& k : knobs) {
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
        std::vector<Knob*> sectionKnobs;
        for (auto& k : knobs) {
            if (k.section == activeSection) {
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
        Color ringCol = { 80, 80, 100, 255 };
        if (k.section == activeSection) {
            ringCol = { 0, 195, 255, 255 };
        }
        d.circle({ (int)k.x, (int)k.y }, (int)k.radius, { .color = ringCol });

        float pct = (*(k.value) - k.minVal) / (k.maxVal - k.minVal);
        float angle = -135.0f + pct * 270.0f;
        float rad = (angle - 90.0f) * M_PI / 180.0f;

        d.line(
            { (int)k.x, (int)k.y }, 
            { (int)(k.x + std::cos(rad) * k.radius), (int)(k.y + std::sin(rad) * k.radius) }, 
            { .color = { 0, 255, 170, 255 } }
        );

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
        Color brainOutline = (activeSection == SECTION_BRAIN) ? Color{ 0, 195, 255, 255 } : Color{ 45, 45, 55, 255 };
        d.filledRect({ 15, 15 }, { 240, 450 }, { .color = { 20, 20, 25, 255 } });
        d.rect({ 15, 15 }, { 240, 450 }, { .color = brainOutline });

        Color kickOutline = (activeSection == SECTION_KICK) ? Color{ 255, 100, 100, 255 } : Color{ 60, 45, 50, 255 };
        d.filledRect({ 270, 15 }, { 580, 450 }, { .color = { 25, 20, 22, 255 } });
        d.rect({ 270, 15 }, { 580, 450 }, { .color = kickOutline });

        Color synthOutline = (activeSection == SECTION_SYNTH) ? Color{ 230, 230, 80, 255 } : Color{ 55, 55, 45, 255 };
        d.filledRect({ 865, 15 }, { 785, 450 }, { .color = { 24, 24, 20, 255 } });
        d.rect({ 865, 15 }, { 785, 450 }, { .color = synthOutline });

        Color masterOutline = (activeSection == SECTION_MASTER) ? Color{ 255, 120, 0, 255 } : Color{ 65, 45, 45, 255 };
        d.filledRect({ 1665, 15 }, { 240, 450 }, { .color = { 28, 20, 20, 255 } });
        d.rect({ 1665, 15 }, { 240, 450 }, { .color = masterOutline });

        // Headers
        d.text({ 25, 25 }, "GENERATIVE SEQUENCE BRAIN", 12, { .color = { 0, 195, 255, 255 }, .font = &PoppinsLight_12 });
        d.text({ 280, 25 }, "FAT KICK ENGINE", 12, { .color = { 255, 100, 100, 255 }, .font = &PoppinsLight_12 });
        d.text({ 875, 25 }, "TEXTURE (VCO2 REPLACEMENT)", 12, { .color = { 230, 230, 80, 255 }, .font = &PoppinsLight_12 });
        d.text({ 1675, 25 }, "MASTER SLICES", 12, { .color = { 255, 120, 0, 255 }, .font = &PoppinsLight_12 });

        // Shift Register Visualizer
        for (int i = 0; i < 16; ++i) {
            bool bitOn = (brain.shiftRegister & (1 << (15 - i))) != 0;
            Color fill = bitOn ? Color{ 0, 255, 170, 255 } : Color{ 40, 40, 50, 255 };
            Color outline = bitOn ? Color{ 255, 255, 255, 255 } : Color{ 60, 60, 75, 255 };
            d.filledRect({ 30 + i * 12, 60 }, { 9, 12 }, { .color = fill });
            d.rect({ 30 + i * 12, 60 }, { 9, 12 }, { .color = outline });
        }

        // Performance touchpad
        Color touchpadFill = brain.spacebarHeld ? Color{ 255, 60, 0, 255 } : Color{ 80, 20, 20, 255 };
        Color touchpadBorder = brain.spacebarHeld ? Color{ 255, 255, 255, 255 } : Color{ 160, 40, 40, 255 };
        d.filledRect({ 1685, 390 }, { 200, 30 }, { .color = touchpadFill });
        d.rect({ 1685, 390 }, { 200, 30 }, { .color = touchpadBorder });
        d.textCentered({ 1785, 398 }, "SPACEBAR CLICK ONLY", 12, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_12 });

        // Draw Synth Waveform Visualizer
        int wfX = 1350;
        int wfY = 20;
        int wfW = 200;
        int wfH = 36;
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

        for (const auto& k : knobs) {
            drawKnob(d, k);
        }

        return true;
    }
};
