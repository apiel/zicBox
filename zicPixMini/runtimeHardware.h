#pragma once

#include <chrono>
#include <deque>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "draw/drawToST7789.h"
#include "helpers/GpioEncoder.h"
#include "helpers/GpioKey.h"
#include "helpers/enc.h"
#include "libs/nlohmann/json.hpp"
#include "studio.h"
#include "ui.h"

extern std::atomic<bool> keep_running;

namespace {
struct HwKeyEvent {
    int key;
    bool pressed;
};

struct HwEncoderEvent {
    int id;
    int8_t direction;
};

static uint32_t stepLastShiftTicks[4] = { 0, 0, 0, 0 };

void dispatchHardwareEncoderEvent(UiPixMini& ui, int encoderId, int8_t direction, bool& needFullRedraw)
{
    if (direction == 0) return;

    uint32_t now = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch())
                       .count();
    int scaled = encGetScaledDirection(direction, now, stepLastShiftTicks[encoderId - 1]);
    stepLastShiftTicks[encoderId - 1] = now;

    ui.handleEncoderTurn(encoderId - 1, scaled, needFullRedraw);
}

void dispatchHardwareKeyEvent(UiPixMini& ui, int key, bool pressed, bool& needFullRedraw)
{
    char k = (char)key;
    if (k == 'a' || k == 's' || k == 'd' || k == 'z' || k == 'x' || k == 'c' || k == 'p') {
        ui.handleButtonKey(k, pressed, needFullRedraw);
    } else if (k >= '1' && k <= '4') {
        int pushIdx = k - '1';
        ui.handleEncoderPush(pushIdx, pressed, needFullRedraw);
    }
}
}

void runHardware(Draw& d, UiPixMini& ui, bool& needFullRedraw)
{
    const int SCREEN_W = 240;
    const int SCREEN_H = 320;

    std::mutex hwKeysEventMtx;
    std::deque<HwKeyEvent> hwKeysEvents;
    std::mutex hwEncodersEventMtx;
    std::deque<HwEncoderEvent> hwEncoderEvents;

    // Default GPIO pins for 6 dedicated buttons (A, S, D, Z, X, C) + 4 encoder push buttons (1..4)
    std::vector<GpioKey::Key> keyConfigs = {
        { 20, 'a' }, { 16, 's' }, { 25, 'd' }, // Row 1: A, S, D
        { 14, 'z' }, { 12, 'x' }, { 1,  'c' }, // Row 2: Z, X, C
        { 24, '1' }, { 15, '2' }, { 7,  '3' }, { 8,  '4' } // Encoders P1..P4 push buttons
    };

    // Default GPIO pins for 4 rotary encoders
    std::vector<GpioEncoder::Encoder> encoderConfigs = {
        { 1, 26, 13 }, // Enc 1 (Top-Left)
        { 2, 6,  5  }, // Enc 2 (Top-Right)
        { 3, 0,  9  }, // Enc 3 (Bottom-Left)
        { 4, 27, 4  }  // Enc 4 (Bottom-Right)
    };

    std::string configPath = "config.json";
    if (const char* envPath = std::getenv("ZIC_CONFIG_PATH")) {
        configPath = envPath;
    }
    std::ifstream configFile(configPath);
    if (configFile.is_open()) {
        try {
            nlohmann::json configJson;
            configFile >> configJson;

            const std::unordered_map<std::string, int> keyMap = {
                { "BTN_A", 'a' }, { "BTN_S", 's' }, { "BTN_D", 'd' },
                { "BTN_Z", 'z' }, { "BTN_X", 'x' }, { "BTN_C", 'c' },
                { "PUSH_1", '1' }, { "PUSH_2", '2' }, { "PUSH_3", '3' }, { "PUSH_4", '4' }
            };

            if (configJson.contains("keys") && configJson["keys"].is_object()) {
                for (auto& keyConfig : keyConfigs) {
                    for (const auto& [name, code] : keyMap) {
                        if (keyConfig.key == code && configJson["keys"].contains(name)) {
                            keyConfig.gpio = configJson["keys"][name].get<int>();
                            break;
                        }
                    }
                }
            }

            if (configJson.contains("encoders") && configJson["encoders"].is_object()) {
                const std::vector<std::string> encoderNames = { "ENCODER_1", "ENCODER_2", "ENCODER_3", "ENCODER_4" };
                for (size_t i = 0; i < encoderConfigs.size() && i < encoderNames.size(); ++i) {
                    if (configJson["encoders"].contains(encoderNames[i])) {
                        auto encJson = configJson["encoders"][encoderNames[i]];
                        if (encJson.is_array() && encJson.size() == 2) {
                            encoderConfigs[i].gpioA = encJson[0].get<int>();
                            encoderConfigs[i].gpioB = encJson[1].get<int>();
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            logWarn("Failed to parse config.json, using default GPIO pinout.");
        }
    }

    GpioKey gpioKey(
        keyConfigs,
        [&hwKeysEventMtx, &hwKeysEvents](GpioKey::Key key, uint8_t state) {
            std::lock_guard<std::mutex> lock(hwKeysEventMtx);
            hwKeysEvents.push_back({ key.key, state == 1 });
        });

    if (gpioKey.init() == 0) {
        gpioKey.startThread();
        logInfo("Hardware keys initialized for zicPixMini");
    } else {
        logWarn("Unable to initialize hardware keys");
    }

    GpioEncoder gpioEncoder(
        encoderConfigs,
        [&hwEncodersEventMtx, &hwEncoderEvents](GpioEncoder::Encoder encoder, int8_t direction) {
            std::lock_guard<std::mutex> lock(hwEncodersEventMtx);
            hwEncoderEvents.push_back({ encoder.id, direction });
        });

    if (gpioEncoder.init() == 0) {
        gpioEncoder.startThread();
        logInfo("Hardware encoders initialized for zicPixMini");
    } else {
        logWarn("Unable to initialize hardware encoders");
    }

    d.setScreenSize({ SCREEN_W, SCREEN_H });
    auto drawToST7789 = std::make_unique<DrawToST7789>(d);
    drawToST7789->setResetPin(17);
    drawToST7789->setDcPin(3);
    drawToST7789->init();

    logInfo("Starting zicPixMini Hardware Loop");

    while (keep_running) {
        std::deque<HwKeyEvent> events;
        {
            std::lock_guard<std::mutex> lock(hwKeysEventMtx);
            events.swap(hwKeysEvents);
        }
        for (const auto& event : events) {
            dispatchHardwareKeyEvent(ui, event.key, event.pressed, needFullRedraw);
        }

        std::deque<HwEncoderEvent> encoderEvents;
        {
            std::lock_guard<std::mutex> lock(hwEncodersEventMtx);
            encoderEvents.swap(hwEncoderEvents);
        }
        for (const auto& event : encoderEvents) {
            dispatchHardwareEncoderEvent(ui, event.id, event.direction, needFullRedraw);
        }

        if (ui.drawUI(d, SCREEN_W, SCREEN_H, needFullRedraw)) {
            drawToST7789->render();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
