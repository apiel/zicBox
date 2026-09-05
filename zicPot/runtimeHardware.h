#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "audio/engines/PotKick.h"
#include "sequenceBrain.h"
#include "zicApp.h"

#if defined(DAISY_SEED) || __has_include("daisy_seed.h")
#include "daisy_seed.h"
#include "libDaisy/src/dev/oled_ssd130x.h"

using namespace daisy;

struct HardwareDaisy {
    DaisySeed hw;
    Encoder encoder;
    UartHandler uart;
    SSD130xI2c64x32Driver display;
    SSD130xI2c64x32Driver::Config displayCfg;

    AdcChannelConfig knobCfgs[NUM_POTS];
    float smoothedPot[NUM_POTS] = { 0.0f };
    float activePotVal[NUM_POTS] = { -1.0f };

    void init()
    {
        hw.Init();
        hw.SetAudioBlockSize(4);

        // Initialize UART1 TX on D13 for 31250 baud MIDI Master Clock
        UartHandler::Config uartCfg;
        uartCfg.periph = UartHandler::Config::Peripheral::USART_1;
        uartCfg.mode = UartHandler::Config::Mode::TX;
        uartCfg.pin_config.tx = seed::D13;
        uartCfg.baudrate = 31250;
        uart.Init(uartCfg);

        // Encoder
        constexpr Pin ENC_A_PIN = seed::D10;
        constexpr Pin ENC_B_PIN = seed::D8;
        constexpr Pin ENC_CLICK_PIN = seed::D9;

        encoder.Init(ENC_A_PIN, ENC_B_PIN, ENC_CLICK_PIN);

        // Initialize I2C Display 64x32 (D12 SDA, D11 SCL)
        displayCfg.transport_config.i2c_config.pin_config.sda = seed::D12;
        displayCfg.transport_config.i2c_config.pin_config.scl = seed::D11;
        display.Init(displayCfg);

        // Configure 10 ADC Potentiometer pins
        knobCfgs[POT_DURATION].InitSingle(seed::A10);
        knobCfgs[POT_VCO_MORPH].InitSingle(seed::A6);
        knobCfgs[POT_FM_DEPTH].InitSingle(seed::A5);
        knobCfgs[POT_FM_SNAP].InitSingle(seed::A4);
        knobCfgs[POT_SWEEP_DEPTH].InitSingle(seed::A11);
        knobCfgs[POT_SWEEP_SHP].InitSingle(seed::A8);
        knobCfgs[POT_DRIVE].InitSingle(seed::A1);
        knobCfgs[POT_WAVEFOLD].InitSingle(seed::A0);
        knobCfgs[POT_CRUSH].InitSingle(seed::A3);
        knobCfgs[POT_RESONATOR].InitSingle(seed::A2);

        hw.adc.Init(knobCfgs, NUM_POTS);
        hw.adc.Start();

        for (int cycle = 0; cycle < 10; cycle++) {
            for (int i = 0; i < NUM_POTS; i++) {
                smoothedPot[i] = getADCValue((PotIndex)i);
            }
        }
    }

    float getADCValue(PotIndex pot)
    {
        return std::clamp(hw.adc.GetFloat((int)pot), 0.0f, 0.99f);
    }

    void renderDisplay(ZicApp& app)
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

    void processPots(ZicApp& app)
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
                renderDisplay(app);
            }
        }
    }

    void sendMidiByte(uint8_t byte)
    {
        uart.PollTx(&byte, 1);
    }
};

#else

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

extern std::atomic<bool> keep_running;

inline void runHardware(ZicApp& app, SequenceBrain& brain, PotKick& kick)
{
    std::cout << "Starting embedded hardware runtime stub for zicPot...\n";
    while (keep_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

#endif
