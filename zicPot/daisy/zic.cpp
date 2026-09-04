#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "daisy_seed.h"
#include "libDaisy/src/dev/oled_ssd130x.h"

#include "audio/engines/DriftKick.h"
#include "zicPot/sequenceBrain.h"
#include "zicPot/zicApp.h"

using namespace daisy;

DaisySeed hw;
Encoder encoder;
UartHandler uart;

SSD130xI2c64x32Driver display;
SSD130xI2c64x32Driver::Config displayCfg;

DriftKick driftKick(44100.0f);
SequenceBrain brain(44100.0f);
ZicApp app(brain, driftKick);

// Hardware Pin Mapping (from PCB layout)
constexpr Pin KNOB_1 = seed::A8;  // Sub Freq (A8) -> POT_SUB_FREQ
constexpr Pin KNOB_2 = seed::A11; // Click Amt (A11) -> POT_CLICK_AMT
constexpr Pin KNOB_3 = seed::A10; // Duration (A10) -> POT_DURATION
constexpr Pin KNOB_4 = seed::A4;  // VCO Morph (A4) -> POT_VCO_MORPH
constexpr Pin KNOB_5 = seed::A5;  // FM Depth (A5) -> POT_FM_DEPTH
constexpr Pin KNOB_6 = seed::A6;  // Drive (A6) -> POT_DRIVE
constexpr Pin KNOB_7 = seed::A3;  // Rumble Amt (A3) -> POT_RUMBLE_AMT
constexpr Pin KNOB_8 = seed::A2;  // Rumble Gap (A2) -> POT_RUMBLE_GAP
constexpr Pin KNOB_9 = seed::A0;  // BPM (A0) -> POT_BPM
constexpr Pin KNOB_10 = seed::A1; // Master Volume (A1) -> POT_MASTER_VOL

AdcChannelConfig knobCfgs[NUM_POTS];
float smoothedPot[NUM_POTS] = { 0.0f };
float activePotVal[NUM_POTS] = { -1.0f };

// Encoder hardware pins (U34): Channel A: D11, Channel B: D9, Switch: D10
constexpr Pin ENC_A_PIN = seed::D11;
constexpr Pin ENC_B_PIN = seed::D9;
constexpr Pin ENC_CLICK_PIN = seed::D10;

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

        float sampleVal = driftKick.sample() * app.masterVolume;
        out[i] = sampleVal;
        out[i + 1] = sampleVal;
    }
}

float getADCValue(PotIndex pot)
{
    return std::clamp(hw.adc.GetFloat((int)pot), 0.0f, 0.99f);
}

void renderDisplay()
{
    app.renderDisplay();

    display.Fill(false);
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 32; x++) {
            if (app.displayView.canvas.getPixel({ x, y })) {
                display.DrawPixel(63 - y, x, true); // 270 deg hardware transform
            }
        }
    }
    display.Update();
}

void processPots()
{
    for (int i = 0; i < NUM_POTS; i++) {
        float raw = getADCValue((PotIndex)i);
        // Exponential Moving Average filter (alpha = 0.15)
        smoothedPot[i] += 0.15f * (raw - smoothedPot[i]);

        // Hysteresis threshold: trigger if changed by more than 1.5%
        if (activePotVal[i] < 0.0f) {
            activePotVal[i] = smoothedPot[i];
            app.applyPotValue((PotIndex)i, activePotVal[i]);
        } else if (std::abs(smoothedPot[i] - activePotVal[i]) > 0.015f) {
            activePotVal[i] = smoothedPot[i];
            app.applyPotValue((PotIndex)i, activePotVal[i]);
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
            smoothedPot[i] = getADCValue((PotIndex)i);
        }
    }
    processPots();
    renderDisplay();

    while (1) {
        encoder.Debounce();

        int32_t inc = encoder.Increment();
        if (inc != 0) {
            app.handleEncoderTurn(inc > 0 ? 1 : -1);
            renderDisplay();
        }

        if (encoder.RisingEdge()) {
            app.handleEncoderClick(sendMidiByte);
            renderDisplay();
        }

        processPots();

        if (app.potOverlayTimer > 0) {
            app.potOverlayTimer -= 2;
            if (app.potOverlayTimer <= 0) {
                app.potOverlayTimer = 0;
                renderDisplay();
            }
        }

        System::Delay(2);
    }
}
