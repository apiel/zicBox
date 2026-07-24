#include <string.h>
#include <string>
#include <algorithm>
#include <cmath>
#include <stdio.h>

#include "libDaisy/src/dev/oled_ssd130x.h"
#include "daisy_seed.h"

#include "fonts/drawText.h"

#include "audio/engines/drop2.h"
#include "zicDropV2/sequenceBrain.h"

using namespace daisy;

DaisySeed hw;
Encoder encoder;

Drop audio(44100.0);
SequenceBrain brain(44100.0);

SSD130xI2c64x32Driver display;
SSD130xI2c64x32Driver::Config displayCfg;

constexpr Pin KNOB_1 = seed::A8;   // Kick VCO Morph
constexpr Pin KNOB_2 = seed::A11;  // Kick Drive
constexpr Pin KNOB_3 = seed::A10;  // Kick Tune
constexpr Pin KNOB_4 = seed::A4;   // Kick Decay
constexpr Pin KNOB_5 = seed::A5;   // Synth Cutoff
constexpr Pin KNOB_6 = seed::A6;   // Synth Resonance
constexpr Pin KNOB_7 = seed::A3;   // Synth Waveform
constexpr Pin KNOB_8 = seed::A2;   // Synth Filter Morph
constexpr Pin KNOB_9 = seed::A0;   // Mix
constexpr Pin KNOB_10 = seed::A1;  // Master Volume

enum PotKnob {
    POT_KICK_VCO_MORPH = 0,
    POT_KICK_DRIVE,
    POT_KICK_TUNE,
    POT_KICK_DECAY,
    POT_SYNTH_CUTOFF,
    POT_SYNTH_RESO,
    POT_SYNTH_WAVE,
    POT_SYNTH_FILT_MORPH,
    POT_MIX,
    POT_MASTER_VOL,
    NUM_POTS
};

AdcChannelConfig knobCfgs[NUM_POTS];
float smoothedPot[NUM_POTS] = {0.0f};
float activePotVal[NUM_POTS] = {-1.0f};

// Encoder hardware pins
constexpr Pin ENC_A_PIN = seed::D8;
constexpr Pin ENC_B_PIN = seed::D10;
constexpr Pin ENC_CLICK_PIN = seed::D9;

// Menu parameter structure
typedef void (*UpdateCb)();

void cbRegenKick() {
    brain.regenerateKick();
}

struct MenuItem {
    const char* name;
    Param* param;       // If non-null, points to Engine Param
    float* varPtr;      // If engine param is null, points to float variable
    float minVal;
    float maxVal;
    float stepVal;
    const char* unitStr;
    bool isInteger;
    UpdateCb onUpdate;
};

MenuItem menuItems[] = {
    { "BPM", nullptr, &brain.bpm, 60.0f, 240.0f, 1.0f, " BPM", true, nullptr },
    { "Kick Gen", nullptr, &brain.kickGenParam, 0.0f, 1.0f, 0.05f, "%", false, cbRegenKick },
    { "Synth Trig", nullptr, &brain.synthTriggerStep, 0.0f, 15.0f, 1.0f, "", true, nullptr },
    { "VCO Morph", &audio.kickVcoMorph, nullptr, 0, 0, 0, "", false, nullptr },
    { "Kick Drive", &audio.kickDrive, nullptr, 0, 0, 0, "", false, nullptr },
    { "Kick Tune", &audio.kickTune, nullptr, 0, 0, 0, " Hz", false, nullptr },
    { "Kick Decay", &audio.kickDecay, nullptr, 0, 0, 0, " ms", false, nullptr },
    { "Kick Pitch", &audio.kickPitchEnvAmt, nullptr, 0, 0, 0, "", false, nullptr },
    { "Kick Sweep", &audio.kickSweepLen, nullptr, 0, 0, 0, " %", false, nullptr },
    { "Click Amt", &audio.kickClickAmt, nullptr, 0, 0, 0, "", false, nullptr },
    { "Click Dec", &audio.kickClickDecay, nullptr, 0, 0, 0, " ms", false, nullptr },
    { "Rumble Amt", &audio.rumbleAmt, nullptr, 0, 0, 0, " %", false, nullptr },
    { "Rumble Gap", &audio.rumbleGap, nullptr, 0, 0, 0, " ms", false, nullptr },
    { "Cutoff", &audio.synthCutoff, nullptr, 0, 0, 0, "", false, nullptr },
    { "Resonance", &audio.synthResonance, nullptr, 0, 0, 0, "", false, nullptr },
    { "Waveform", &audio.synthWaveform, nullptr, 0, 0, 0, "", false, nullptr },
    { "Filt Morph", &audio.synthFilterMorph, nullptr, 0, 0, 0, "", false, nullptr },
    { "Release", &audio.synthRelease, nullptr, 0, 0, 0, " ms", false, nullptr },
    { "Env Amt", &audio.synthEnvAmt, nullptr, 0, 0, 0, "", false, nullptr },
    { "Synth Drive", &audio.synthDrive, nullptr, 0, 0, 0, "", false, nullptr },
    { "Synth Shape", &audio.synthWaveshape, nullptr, 0, 0, 0, "", false, nullptr },
    { "FM Morph", &audio.synthFmAmt, nullptr, 0, 0, 0, "", false, nullptr },
    { "Mod Type", &audio.synthModType, nullptr, 0, 0, 0, "", true, nullptr },
    { "Mod Depth", &audio.synthModDepth, nullptr, 0, 0, 0, " %", false, nullptr },
    { "Mod Speed", &audio.synthModSpeed, nullptr, 0, 0, 0, " %", false, nullptr },
    { "Delay Mix", &audio.synthDelayMix, nullptr, 0, 0, 0, "", false, nullptr },
    { "Delay Time", &audio.synthDelayTime, nullptr, 0, 0, 0, " ms", false, nullptr },
    { "Delay Feed", &audio.synthDelayFeedback, nullptr, 0, 0, 0, "", false, nullptr },
    { "Wavefold", &audio.mstFold, nullptr, 0, 0, 0, "", false, nullptr },
    { "Base Pitch", &audio.synthBasePitch, nullptr, 0, 0, 0, "", true, nullptr },
    { "Mix", &audio.mix, nullptr, 0, 0, 0, "", false, nullptr },
    { "Volume", &audio.masterVolume, nullptr, 0, 0, 0, "", false, nullptr }
};

constexpr int TOTAL_MENU_ITEMS = sizeof(menuItems) / sizeof(menuItems[0]);

int currentMenuItem = 0;
bool isEditing = false;

// Pot overlay state
int lastMovedPotIndex = -1;
int32_t potOverlayTimer = 0;

static void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                           AudioHandle::InterleavingOutputBuffer out,
                           size_t size)
{
    for (size_t i = 0; i < size; i += 2)
    {
        bool ticked = brain.processSample();
        if (ticked) {
            audio.performanceMode = brain.performanceMode;
            if (brain.triggerKick) {
                audio.triggerKickVoice();
            }
            if (brain.triggerSynth) {
                audio.triggerSynthVoice(audio.synthBasePitch.value + brain.currentPitch);
            }
        }
        float sampleVal = audio.process();
        out[i] = sampleVal;
        out[i + 1] = sampleVal;
    }
}

float getADCValue(PotKnob pot) {
    return std::clamp(hw.adc.GetFloat(pot), 0.0f, 0.99f);
}

std::string getFormattedValue(const MenuItem& item) {
    char buf[32];
    if (item.param != nullptr) {
        if (item.param == &audio.synthModType) {
            int idx = std::clamp((int)std::round(audio.synthModType.value), 0, Drop::TOTAL_MOD_TYPES - 1);
            return Drop::modMatrix[idx].name;
        }
        if (item.param->step == 1.0f || item.isInteger) {
            snprintf(buf, sizeof(buf), "%d%s", (int)std::round(item.param->value), item.param->unit ? item.param->unit : "");
        } else {
            snprintf(buf, sizeof(buf), "%.2f%s", item.param->value, item.param->unit ? item.param->unit : "");
        }
    } else if (item.varPtr != nullptr) {
        if (item.varPtr == &brain.synthTriggerStep) {
            int idx = std::clamp((int)std::round(*item.varPtr), 0, 15);
            return SequenceBrain::trigOptions[idx].label;
        }
        if (item.isInteger) {
            snprintf(buf, sizeof(buf), "%d%s", (int)std::round(*item.varPtr), item.unitStr);
        } else {
            snprintf(buf, sizeof(buf), "%.2f%s", *item.varPtr, item.unitStr);
        }
    }
    return std::string(buf);
}

void applyPotValue(PotKnob pot, float normVal) {
    switch (pot) {
        case POT_KICK_VCO_MORPH:
            audio.kickVcoMorph.set(audio.kickVcoMorph.min + normVal * (audio.kickVcoMorph.max - audio.kickVcoMorph.min));
            break;
        case POT_KICK_DRIVE:
            audio.kickDrive.set(audio.kickDrive.min + normVal * (audio.kickDrive.max - audio.kickDrive.min));
            break;
        case POT_KICK_TUNE:
            audio.kickTune.set(audio.kickTune.min + normVal * (audio.kickTune.max - audio.kickTune.min));
            break;
        case POT_KICK_DECAY:
            audio.kickDecay.set(audio.kickDecay.min + normVal * (audio.kickDecay.max - audio.kickDecay.min));
            break;
        case POT_SYNTH_CUTOFF:
            audio.synthCutoff.set(audio.synthCutoff.min + normVal * (audio.synthCutoff.max - audio.synthCutoff.min));
            break;
        case POT_SYNTH_RESO:
            audio.synthResonance.set(audio.synthResonance.min + normVal * (audio.synthResonance.max - audio.synthResonance.min));
            break;
        case POT_SYNTH_WAVE:
            audio.synthWaveform.set(audio.synthWaveform.min + normVal * (audio.synthWaveform.max - audio.synthWaveform.min));
            break;
        case POT_SYNTH_FILT_MORPH:
            audio.synthFilterMorph.set(audio.synthFilterMorph.min + normVal * (audio.synthFilterMorph.max - audio.synthFilterMorph.min));
            break;
        case POT_MIX:
            audio.mix.set(audio.mix.min + normVal * (audio.mix.max - audio.mix.min));
            break;
        case POT_MASTER_VOL:
            audio.masterVolume.set(audio.masterVolume.min + normVal * (audio.masterVolume.max - audio.masterVolume.min));
            break;
        default:
            break;
    }
}

const char* getPotName(PotKnob pot) {
    switch (pot) {
        case POT_KICK_VCO_MORPH: return "Kick VCO Morph";
        case POT_KICK_DRIVE: return "Kick Drive";
        case POT_KICK_TUNE: return "Kick Tune";
        case POT_KICK_DECAY: return "Kick Decay";
        case POT_SYNTH_CUTOFF: return "Synth Cutoff";
        case POT_SYNTH_RESO: return "Synth Reso";
        case POT_SYNTH_WAVE: return "Synth Wave";
        case POT_SYNTH_FILT_MORPH: return "Filt Morph";
        case POT_MIX: return "Mix";
        case POT_MASTER_VOL: return "Master Vol";
        default: return "";
    }
}

std::string getPotFormattedValue(PotKnob pot) {
    char buf[32];
    switch (pot) {
        case POT_KICK_VCO_MORPH: snprintf(buf, sizeof(buf), "%.2f", audio.kickVcoMorph.value); break;
        case POT_KICK_DRIVE: snprintf(buf, sizeof(buf), "%.2f", audio.kickDrive.value); break;
        case POT_KICK_TUNE: snprintf(buf, sizeof(buf), "%.0f Hz", audio.kickTune.value); break;
        case POT_KICK_DECAY: snprintf(buf, sizeof(buf), "%.0f ms", audio.kickDecay.value); break;
        case POT_SYNTH_CUTOFF: snprintf(buf, sizeof(buf), "%.2f", audio.synthCutoff.value); break;
        case POT_SYNTH_RESO: snprintf(buf, sizeof(buf), "%.2f", audio.synthResonance.value); break;
        case POT_SYNTH_WAVE: snprintf(buf, sizeof(buf), "%.2f", audio.synthWaveform.value); break;
        case POT_SYNTH_FILT_MORPH: snprintf(buf, sizeof(buf), "%.2f", audio.synthFilterMorph.value); break;
        case POT_MIX: snprintf(buf, sizeof(buf), "%.0f %%", audio.mix.value * 100.0f); break;
        case POT_MASTER_VOL: snprintf(buf, sizeof(buf), "%.0f %%", audio.masterVolume.value * 100.0f); break;
        default: return "";
    }
    return std::string(buf);
}

void updateItemValue(MenuItem& item, int dir) {
    if (item.param != nullptr) {
        item.param->inc((float)dir);
    } else if (item.varPtr != nullptr) {
        float val = *item.varPtr + (dir * item.stepVal);
        *item.varPtr = std::clamp(val, item.minVal, item.maxVal);
    }
    if (item.onUpdate != nullptr) {
        item.onUpdate();
    }
}

void renderDisplay() {
    display.Fill(false);

    if (potOverlayTimer > 0) {
        // Render Pot takeover screen
        std::string potTitle = getPotName((PotKnob)lastMovedPotIndex);
        std::string potVal = getPotFormattedValue((PotKnob)lastMovedPotIndex);
        text(display, 0, 0, potTitle, PoppinsLight_8);
        text(display, 0, 16, potVal, PoppinsLight_12);
    } else {
        // Render Encoder Menu
        MenuItem& item = menuItems[currentMenuItem];
        char header[32];
        snprintf(header, sizeof(header), "%d.%s", currentMenuItem + 1, item.name);
        text(display, 0, 0, std::string(header), PoppinsLight_8);

        std::string valStr = getFormattedValue(item);
        if (isEditing) {
            valStr = "> " + valStr + " <";
        }
        text(display, 0, 16, valStr, PoppinsLight_12);
    }

    display.Update();
}

void processPots() {
    for (int i = 0; i < NUM_POTS; i++) {
        float raw = getADCValue((PotKnob)i);
        // Exponential Moving Average filter (alpha = 0.15)
        smoothedPot[i] += 0.15f * (raw - smoothedPot[i]);

        // Hysteresis threshold: only trigger if changed by more than 1.5%
        if (activePotVal[i] < 0.0f) {
            activePotVal[i] = smoothedPot[i];
            applyPotValue((PotKnob)i, activePotVal[i]);
        } else if (std::abs(smoothedPot[i] - activePotVal[i]) > 0.015f) {
            activePotVal[i] = smoothedPot[i];
            applyPotValue((PotKnob)i, activePotVal[i]);

            lastMovedPotIndex = i;
            potOverlayTimer = 1500; // Show pot takeover for 1.5 seconds
            renderDisplay();
        }
    }
}

int main(void) {
    hw.Init();
    hw.SetAudioBlockSize(4);
    hw.StartAudio(AudioCallback);

    encoder.Init(ENC_A_PIN, ENC_B_PIN, ENC_CLICK_PIN);

    displayCfg.transport_config.i2c_config.pin_config.sda = seed::D12;
    displayCfg.transport_config.i2c_config.pin_config.scl = seed::D11;
    display.Init(displayCfg);

    knobCfgs[POT_KICK_VCO_MORPH].InitSingle(KNOB_1);
    knobCfgs[POT_KICK_DRIVE].InitSingle(KNOB_2);
    knobCfgs[POT_KICK_TUNE].InitSingle(KNOB_3);
    knobCfgs[POT_KICK_DECAY].InitSingle(KNOB_4);
    knobCfgs[POT_SYNTH_CUTOFF].InitSingle(KNOB_5);
    knobCfgs[POT_SYNTH_RESO].InitSingle(KNOB_6);
    knobCfgs[POT_SYNTH_WAVE].InitSingle(KNOB_7);
    knobCfgs[POT_SYNTH_FILT_MORPH].InitSingle(KNOB_8);
    knobCfgs[POT_MIX].InitSingle(KNOB_9);
    knobCfgs[POT_MASTER_VOL].InitSingle(KNOB_10);

    hw.adc.Init(knobCfgs, NUM_POTS);
    hw.adc.Start();

    // Initial ADC read & warm-up filter
    for (int cycle = 0; cycle < 10; cycle++) {
        for (int i = 0; i < NUM_POTS; i++) {
            float raw = getADCValue((PotKnob)i);
            smoothedPot[i] = raw;
        }
    }
    processPots();
    renderDisplay();

    while (1) {
        encoder.Debounce();

        // Encoder turn logic
        int32_t inc = encoder.Increment();
        if (inc != 0) {
            potOverlayTimer = 0; // Immediately cancel pot takeover on encoder action
            if (isEditing) {
                updateItemValue(menuItems[currentMenuItem], inc > 0 ? 1 : -1);
            } else {
                currentMenuItem += (inc > 0 ? 1 : -1);
                if (currentMenuItem < 0) currentMenuItem = TOTAL_MENU_ITEMS - 1;
                if (currentMenuItem >= TOTAL_MENU_ITEMS) currentMenuItem = 0;
            }
            renderDisplay();
        }

        // Encoder press click logic
        if (encoder.RisingEdge()) {
            potOverlayTimer = 0;
            isEditing = !isEditing;
            renderDisplay();
        }

        // Read & filter physical 10 Pots
        processPots();

        if (potOverlayTimer > 0) {
            potOverlayTimer -= 2;
            if (potOverlayTimer <= 0) {
                potOverlayTimer = 0;
                renderDisplay();
            }
        }

        System::Delay(2);
    }
}
