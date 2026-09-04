#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "daisy_seed.h"
#include "libDaisy/src/dev/oled_ssd130x.h"

#include "audio/engines/DriftKick.h"
#include "fonts/drawText.h"
#include "zicPot/sequenceBrain.h"

using namespace daisy;

DaisySeed hw;
Encoder encoder;
UartHandler uart;

SSD130xI2c64x32Driver display;
SSD130xI2c64x32Driver::Config displayCfg;

DriftKick driftKick(44100.0f);
SequenceBrain brain(44100.0f);

// Hardware Pin Mapping (from PCB layout)
constexpr Pin KNOB_1 = seed::A8;  // Top row 1: Sub Freq
constexpr Pin KNOB_2 = seed::A11; // Top row 2: Click Amt
constexpr Pin KNOB_3 = seed::A10; // Top row 3: Duration
constexpr Pin KNOB_4 = seed::A4;  // Middle row 1: VCO Morph
constexpr Pin KNOB_5 = seed::A5;  // Middle row 2: FM Depth
constexpr Pin KNOB_6 = seed::A6;  // Middle row 3: Drive
constexpr Pin KNOB_7 = seed::A3;  // Bottom row 1: Rumble Amt
constexpr Pin KNOB_8 = seed::A2;  // Bottom row 2: Rumble Gap
constexpr Pin KNOB_9 = seed::A0;  // Bottom row 3: BPM
constexpr Pin KNOB_10 = seed::A1; // Bottom row 4: Master Volume

enum PotKnob {
    POT_SUB_FREQ = 0,
    POT_CLICK_AMT,
    POT_DURATION,
    POT_VCO_MORPH,
    POT_FM_DEPTH,
    POT_DRIVE,
    POT_RUMBLE_AMT,
    POT_RUMBLE_GAP,
    POT_BPM,
    POT_MASTER_VOL,
    NUM_POTS
};

AdcChannelConfig knobCfgs[NUM_POTS];
float smoothedPot[NUM_POTS] = { 0.0f };
float activePotVal[NUM_POTS] = { -1.0f };
float masterVolume = 0.8f;

// Encoder hardware pins (U34): Channel A: D11, Channel B: D9, Switch: D10
constexpr Pin ENC_A_PIN = seed::D11;
constexpr Pin ENC_B_PIN = seed::D9;
constexpr Pin ENC_CLICK_PIN = seed::D10;

// Pot overlay state
int lastMovedPotIndex = -1;
int32_t potOverlayTimer = 0;

typedef void (*UpdateCb)();

void cbRegenKick()
{
    brain.regenerateKick();
}

struct MenuItem {
    const char* name;
    Param* param;  // If non-null, points to Engine Param
    float* varPtr; // If non-null, points to float variable
    float minVal;
    float maxVal;
    float stepVal;
    const char* unitStr;
    bool isInteger;
    UpdateCb onUpdate;
};

MenuItem menuItems[] = {
    { "PLAY / STOP", nullptr, nullptr, 0.0f, 1.0f, 1.0f, "", true, nullptr },
    { "BPM", nullptr, &brain.bpm, 60.0f, 240.0f, 1.0f, " BPM", true, nullptr },
    { "Gen Velocity", nullptr, &brain.genP1, 0.0f, 1.0f, 0.05f, "%", false, cbRegenKick },
    { "Gen Ghosts", nullptr, &brain.genP2, 0.0f, 1.0f, 0.05f, "%", false, cbRegenKick },
    { "Gen Rumble", nullptr, &brain.genP3, 0.0f, 1.0f, 0.05f, "%", false, cbRegenKick },
    { "Sub Freq", &driftKick.baseFreq, nullptr, 30.0f, 100.0f, 1.0f, " Hz", true, nullptr },
    { "Click Amt", &driftKick.clickAmt, nullptr, 0.0f, 100.0f, 1.0f, "%", true, nullptr },
    { "Duration", &driftKick.duration, nullptr, 50.0f, 1500.0f, 10.0f, " ms", true, nullptr },
    { "VCO Morph", &driftKick.vcoMorph, nullptr, 0.0f, 100.0f, 1.0f, "%", true, nullptr },
    { "FM Depth", &driftKick.fmDepth, nullptr, 0.0f, 100.0f, 1.0f, "%", true, nullptr },
    { "Drive", &driftKick.drive, nullptr, 0.0f, 100.0f, 1.0f, "%", true, nullptr },
    { "Rumble", &driftKick.rumbleAmt, nullptr, 0.0f, 100.0f, 1.0f, "%", true, nullptr },
    { "Rum Gap", &driftKick.rumbleGap, nullptr, 10.0f, 400.0f, 5.0f, " ms", true, nullptr },
    { "Master Vol", nullptr, &masterVolume, 0.0f, 1.0f, 0.05f, "%", false, nullptr }
};

constexpr int TOTAL_MENU_ITEMS = sizeof(menuItems) / sizeof(menuItems[0]);
int currentMenuItem = 0;
bool isEditing = false;

// UART TX helper function for sending MIDI bytes on D13
void sendMidiByte(uint8_t byte)
{
    uart.PollTx(&byte, 1);
}

// Audio Callback
static void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                           AudioHandle::InterleavingOutputBuffer out,
                           size_t size)
{
    for (size_t i = 0; i < size; i += 2) {
        brain.processSample(driftKick, sendMidiByte);

        float sampleVal = driftKick.sample() * masterVolume;
        out[i] = sampleVal;
        out[i + 1] = sampleVal;
    }
}

float getADCValue(PotKnob pot)
{
    return std::clamp(hw.adc.GetFloat((int)pot), 0.0f, 0.99f);
}

const char* getPotName(PotKnob pot)
{
    switch (pot) {
        case POT_SUB_FREQ: return "Sub Freq";
        case POT_CLICK_AMT: return "Click Amt";
        case POT_DURATION: return "Duration";
        case POT_VCO_MORPH: return "VCO Morph";
        case POT_FM_DEPTH: return "FM Depth";
        case POT_DRIVE: return "Drive";
        case POT_RUMBLE_AMT: return "Rumble Amt";
        case POT_RUMBLE_GAP: return "Rumble Gap";
        case POT_BPM: return "BPM";
        case POT_MASTER_VOL: return "Master Vol";
        default: return "";
    }
}

std::string getPotFormattedValue(PotKnob pot)
{
    char buf[32];
    switch (pot) {
        case POT_SUB_FREQ: snprintf(buf, sizeof(buf), "%.0f Hz", driftKick.baseFreq.value); break;
        case POT_CLICK_AMT: snprintf(buf, sizeof(buf), "%.0f %%", driftKick.clickAmt.value); break;
        case POT_DURATION: snprintf(buf, sizeof(buf), "%.0f ms", driftKick.duration.value); break;
        case POT_VCO_MORPH: snprintf(buf, sizeof(buf), "%.0f %%", driftKick.vcoMorph.value); break;
        case POT_FM_DEPTH: snprintf(buf, sizeof(buf), "%.0f %%", driftKick.fmDepth.value); break;
        case POT_DRIVE: snprintf(buf, sizeof(buf), "%.0f %%", driftKick.drive.value); break;
        case POT_RUMBLE_AMT: snprintf(buf, sizeof(buf), "%.0f %%", driftKick.rumbleAmt.value); break;
        case POT_RUMBLE_GAP: snprintf(buf, sizeof(buf), "%.0f ms", driftKick.rumbleGap.value); break;
        case POT_BPM: snprintf(buf, sizeof(buf), "%.0f BPM", brain.bpm); break;
        case POT_MASTER_VOL: snprintf(buf, sizeof(buf), "%d %%", (int)std::round(masterVolume * 100.0f)); break;
        default: return "";
    }
    return std::string(buf);
}

void applyPotValue(PotKnob pot, float normVal)
{
    normVal = std::clamp(normVal, 0.0f, 1.0f);
    switch (pot) {
        case POT_SUB_FREQ:
            driftKick.baseFreq.set(driftKick.baseFreq.min + normVal * (driftKick.baseFreq.max - driftKick.baseFreq.min));
            break;
        case POT_CLICK_AMT:
            driftKick.clickAmt.set(driftKick.clickAmt.min + normVal * (driftKick.clickAmt.max - driftKick.clickAmt.min));
            break;
        case POT_DURATION:
            driftKick.duration.set(driftKick.duration.min + normVal * (driftKick.duration.max - driftKick.duration.min));
            break;
        case POT_VCO_MORPH:
            driftKick.vcoMorph.set(driftKick.vcoMorph.min + normVal * (driftKick.vcoMorph.max - driftKick.vcoMorph.min));
            break;
        case POT_FM_DEPTH:
            driftKick.fmDepth.set(driftKick.fmDepth.min + normVal * (driftKick.fmDepth.max - driftKick.fmDepth.min));
            break;
        case POT_DRIVE:
            driftKick.drive.set(driftKick.drive.min + normVal * (driftKick.drive.max - driftKick.drive.min));
            break;
        case POT_RUMBLE_AMT:
            driftKick.rumbleAmt.set(driftKick.rumbleAmt.min + normVal * (driftKick.rumbleAmt.max - driftKick.rumbleAmt.min));
            break;
        case POT_RUMBLE_GAP:
            driftKick.rumbleGap.set(driftKick.rumbleGap.min + normVal * (driftKick.rumbleGap.max - driftKick.rumbleGap.min));
            break;
        case POT_BPM:
            brain.bpm = 60.0f + normVal * (240.0f - 60.0f);
            break;
        case POT_MASTER_VOL:
            masterVolume = normVal;
            break;
        default:
            break;
    }
}

std::string getFormattedMenuItemValue(const MenuItem& item, int index)
{
    if (index == 0) {
        return brain.isPlaying ? "RUNNING" : "STOPPED";
    }
    char buf[32];
    float val = 0.0f;
    if (item.param != nullptr) {
        val = item.param->value;
    } else if (item.varPtr != nullptr) {
        val = *item.varPtr;
    }

    if (item.isInteger) {
        snprintf(buf, sizeof(buf), "%d%s", (int)std::round(val), item.unitStr);
    } else if (item.maxVal <= 1.0f && item.minVal >= 0.0f) {
        snprintf(buf, sizeof(buf), "%d%%", (int)std::round(val * 100.0f));
    } else {
        snprintf(buf, sizeof(buf), "%.1f%s", val, item.unitStr);
    }
    return std::string(buf);
}

void updateItemValue(MenuItem& item, int dir)
{
    float step = item.stepVal > 0.0f ? item.stepVal : 1.0f;
    if (item.param != nullptr) {
        item.param->set(item.param->value + (dir * step));
    } else if (item.varPtr != nullptr) {
        float val = *item.varPtr + (dir * step);
        *item.varPtr = std::clamp(val, item.minVal, item.maxVal);
    }
    if (item.onUpdate != nullptr) {
        item.onUpdate();
    }
}

const char* getShortPotName(PotKnob pot)
{
    switch (pot) {
        case POT_SUB_FREQ: return "SUB";
        case POT_CLICK_AMT: return "CLCK";
        case POT_DURATION: return "DUR";
        case POT_VCO_MORPH: return "MRPH";
        case POT_FM_DEPTH: return "FM";
        case POT_DRIVE: return "DRV";
        case POT_RUMBLE_AMT: return "RUMB";
        case POT_RUMBLE_GAP: return "GAP";
        case POT_BPM: return "BPM";
        case POT_MASTER_VOL: return "VOL";
        default: return "";
    }
}

const char* getShortItemName(int index)
{
    switch (index) {
        case 0: return "PLAY";
        case 1: return "BPM";
        case 2: return "VEL";
        case 3: return "GHST";
        case 4: return "RMBL";
        case 5: return "SUB";
        case 6: return "CLCK";
        case 7: return "DUR";
        case 8: return "MRPH";
        case 9: return "FM";
        case 10: return "DRV";
        case 11: return "RUMB";
        case 12: return "GAP";
        case 13: return "VOL";
        default: return "";
    }
}

void renderDisplay()
{
    display.Fill(false);

    // Header (y = 0): Menu Item ratio & Play/Stop Status
    char headerBuf[16];
    snprintf(headerBuf, sizeof(headerBuf), "%d/%d", currentMenuItem + 1, TOTAL_MENU_ITEMS);
    text(display, 0, 0, std::string(headerBuf), PoppinsLight_8);
    text(display, 20, 0, brain.isPlaying ? ">" : "||", PoppinsLight_8);

    if (potOverlayTimer > 0) {
        // Render Pot takeover screen overlay for 32x64 OLED
        std::string potTitle = getShortPotName((PotKnob)lastMovedPotIndex);
        std::string potVal = getPotFormattedValue((PotKnob)lastMovedPotIndex);
        text(display, 0, 12, potTitle, PoppinsLight_8);
        text(display, 0, 24, potVal, PoppinsLight_12);
    } else {
        // Render Encoder Menu for 32x64 OLED
        MenuItem& item = menuItems[currentMenuItem];
        std::string titleStr = getShortItemName(currentMenuItem);
        text(display, 0, 12, titleStr, PoppinsLight_8);

        std::string valStr = getFormattedMenuItemValue(item, currentMenuItem);
        if (isEditing) {
            valStr = ">" + valStr;
        }
        text(display, 0, 24, valStr, PoppinsLight_12);
    }

    // Mini 16-step bar across bottom of 32x64 screen (y = 52..60)
    for (int i = 0; i < 16; i++) {
        bool active = (i < (int)brain.kickSequence.size()) && brain.kickSequence[i].active;
        bool isCurrent = brain.isPlaying && ((brain.currentStep % 16) == i);
        if (active || isCurrent) {
            for (int y = 54; y <= 62; y++) {
                display.DrawPixel(i * 2, y, true);
            }
        }
    }

    display.Update();
}

void processPots()
{
    for (int i = 0; i < NUM_POTS; i++) {
        float raw = getADCValue((PotKnob)i);
        // Exponential Moving Average filter (alpha = 0.15)
        smoothedPot[i] += 0.15f * (raw - smoothedPot[i]);

        // Hysteresis threshold: trigger if changed by more than 1.5%
        if (activePotVal[i] < 0.0f) {
            activePotVal[i] = smoothedPot[i];
            applyPotValue((PotKnob)i, activePotVal[i]);
        } else if (std::abs(smoothedPot[i] - activePotVal[i]) > 0.015f) {
            activePotVal[i] = smoothedPot[i];
            applyPotValue((PotKnob)i, activePotVal[i]);

            lastMovedPotIndex = i;
            potOverlayTimer = 1500; // 1.5 seconds takeover
            renderDisplay();
        }
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(4);
    hw.StartAudio(AudioCallback);

    // Initialize UART1 TX on D13 for 31250 baud MIDI Master Clock
    UartHandler::Config uartCfg;
    uartCfg.periph = UartHandler::Config::Peripheral::USART_1;
    uartCfg.mode = UartHandler::Config::Mode::TX;
    uartCfg.pin_config.tx = seed::D13;
    uartCfg.baudrate = 31250;
    uart.Init(uartCfg);

    // Initialize Rotary Encoder on D11 (Ch A), D9 (Ch B), D10 (Switch)
    encoder.Init(ENC_A_PIN, ENC_B_PIN, ENC_CLICK_PIN);

    // Initialize I2C Display 64x32 (D12 SDA, D11 SCL)
    displayCfg.transport_config.i2c_config.pin_config.sda = seed::D12;
    displayCfg.transport_config.i2c_config.pin_config.scl = seed::D11;
    display.Init(displayCfg);

    // Configure 10 ADC Potentiometer pins
    knobCfgs[POT_SUB_FREQ].InitSingle(KNOB_1);
    knobCfgs[POT_CLICK_AMT].InitSingle(KNOB_2);
    knobCfgs[POT_DURATION].InitSingle(KNOB_3);
    knobCfgs[POT_VCO_MORPH].InitSingle(KNOB_4);
    knobCfgs[POT_FM_DEPTH].InitSingle(KNOB_5);
    knobCfgs[POT_DRIVE].InitSingle(KNOB_6);
    knobCfgs[POT_RUMBLE_AMT].InitSingle(KNOB_7);
    knobCfgs[POT_RUMBLE_GAP].InitSingle(KNOB_8);
    knobCfgs[POT_BPM].InitSingle(KNOB_9);
    knobCfgs[POT_MASTER_VOL].InitSingle(KNOB_10);

    hw.adc.Init(knobCfgs, NUM_POTS);
    hw.adc.Start();

    // Initial ADC warm-up
    for (int cycle = 0; cycle < 10; cycle++) {
        for (int i = 0; i < NUM_POTS; i++) {
            smoothedPot[i] = getADCValue((PotKnob)i);
        }
    }
    processPots();
    renderDisplay();

    while (1) {
        encoder.Debounce();

        int32_t inc = encoder.Increment();
        if (inc != 0) {
            potOverlayTimer = 0;
            if (isEditing) {
                updateItemValue(menuItems[currentMenuItem], inc > 0 ? 1 : -1);
            } else {
                currentMenuItem += (inc > 0 ? 1 : -1);
                if (currentMenuItem < 0) currentMenuItem = TOTAL_MENU_ITEMS - 1;
                if (currentMenuItem >= TOTAL_MENU_ITEMS) currentMenuItem = 0;
            }
            renderDisplay();
        }

        if (encoder.RisingEdge()) {
            potOverlayTimer = 0;
            if (currentMenuItem == 0) {
                brain.togglePlayStop(sendMidiByte);
            } else {
                isEditing = !isEditing;
            }
            renderDisplay();
        }

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
