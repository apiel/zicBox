#pragma once

#include <chrono>
#include <deque>
#include <fstream>
#include <mutex>
#include <thread>

#include "draw/drawToST7789.h"
#include "helpers/GpioEncoder.h"
#include "helpers/GpioKey.h"
#include "libs/nlohmann/json.hpp"
#include "zicXYv2/ui.h"

namespace {
struct HwKeyEvent {
    int key;
    bool pressed;
};

struct HwEncoderEvent {
    int id;
    int8_t direction;
};

// last tick times for mouse-wheel scaling for each of the 4 params
static uint32_t stepLastShiftTicks[4] = { 0, 0, 0, 0 };

void dispatchHardwareEncoderEvent(int encoderId, int8_t direction, bool& needFullRedraw)
{
    if (direction == 0) return;

    uint32_t now = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch())
                       .count();
    int scaled = encGetScaledDirection(direction, now, stepLastShiftTicks[encoderId - 1]);
    stepLastShiftTicks[encoderId - 1] = now;

    UiTrack::onEncoder(encoderId, scaled, needFullRedraw);
    MasterFx::onEncoder(encoderId, scaled, needFullRedraw);
    UiSeq::onEncoder(encoderId, scaled);
    UiMenu::onEncoder(encoderId, scaled);
}

void dispatchHardwareKeyEvent(int key, bool pressed, bool& needFullRedraw)
{
    if (pressed) {
        TopBar::keyPressed(key, needFullRedraw);
        UiTrack::keyPressed(key, needFullRedraw);
        UiSeq::keyPressed(key, needFullRedraw);
        UiClips::keyPressed(key, needFullRedraw);
        MasterFx::keyPressed(key, needFullRedraw);
        UiMenu::keyPressed(key, needFullRedraw);
        return;
    }

    TopBar::keyReleased(key, needFullRedraw);
    UiSeq::keyReleased(key, needFullRedraw);
    UiClips::keyReleased(key, needFullRedraw);

    if (key >= KEY_1 && key <= KEY_6) {
        int trkIdx = key - KEY_1;
        int note = (studio.selTrack == trkIdx && studio.selStep != -1)
            ? studio.tracks[trkIdx]->sequence[studio.selStep].note
            : 60;
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        studio.tracks[trkIdx]->engine->noteOff(note);
    }
}
}

void runHardware(Draw& d, const Styles& appStyles, bool& needFullRedraw)
{
    std::mutex hwKeysEventMtx;
    std::deque<HwKeyEvent> hwKeysEvents;
    std::mutex hwEncodersEventMtx;
    std::deque<HwEncoderEvent> hwEncoderEvents;

    // Default key definitions
    std::vector<GpioKey::Key> keyConfigs = {
        { 20, KEY_F1 }, { 16, KEY_F2 }, { 25, KEY_F3 }, { 14, KEY_F4 }, { 2, KEY_F5 },
        { 12, KEY_1 }, { 1, KEY_2 }, { 24, KEY_3 }, { 15, KEY_4 }, { 7, KEY_5 }, { 8, KEY_6 }, { 23, KEY_7 }, { 22, KEY_8 }
    };

    // Default encoder definitions
    std::vector<GpioEncoder::Encoder> encoderConfigs = {
        { 1, 26, 13 },
        { 2, 6, 5 },
        { 3, 0, 9 },
        { 4, 27, 4 }
    };

    // Attempt to load and overwrite configurations from JSON
    // {
    //    "keys": {
    //        "KEY_F1": 21,
    //        "KEY_1": 10
    //    },
    //    "encoders": {
    //        "ENCODER_1": [26, 13],
    //        "ENCODER_2": [6, 5]
    //    }
    // }
    std::string configPath = "config.json";
    if (const char* envPath = std::getenv("ZIC_CONFIG_PATH")) {
        configPath = envPath;
    }
    std::ifstream configFile(configPath);
    if (configFile.is_open()) {
        try {
            nlohmann::json configJson;
            configFile >> configJson;

            // Map standard key names to internal integer values
            const std::unordered_map<std::string, int> keyMap = {
                { "KEY_F1", KEY_F1 }, { "KEY_F2", KEY_F2 }, { "KEY_F3", KEY_F3 }, { "KEY_F4", KEY_F4 }, { "KEY_F5", KEY_F5 },
                { "KEY_1", KEY_1 }, { "KEY_2", KEY_2 }, { "KEY_3", KEY_3 }, { "KEY_4", KEY_4 }, { "KEY_5", KEY_5 }, { "KEY_6", KEY_6 }, { "KEY_7", KEY_7 }, { "KEY_8", KEY_8 }
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
            logWarn("Failed to parse config.json, using defaults.");
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
        logInfo("Hardware keys initialized (GPIO)");
    } else {
        logWarn("Unable to initialize hardware keys (GPIO)");
    }

    GpioEncoder gpioEncoder(
        encoderConfigs,
        [&hwEncodersEventMtx, &hwEncoderEvents](GpioEncoder::Encoder encoder, int8_t direction) {
            std::lock_guard<std::mutex> lock(hwEncodersEventMtx);
            hwEncoderEvents.push_back({ encoder.id, direction });
        });

    if (gpioEncoder.init() == 0) {
        gpioEncoder.startThread();
        logInfo("Hardware encoders initialized (GPIO)");
    } else {
        logWarn("Unable to initialize hardware encoders (GPIO)");
    }

    auto drawToST7789 = std::make_unique<DrawToST7789>(d);
    drawToST7789->setResetPin(17);
    drawToST7789->setDcPin(3);
    drawToST7789->setYRamMargin(0);
    drawToST7789->init();

    while (keep_running) {
        std::deque<HwKeyEvent> events;
        {
            std::lock_guard<std::mutex> lock(hwKeysEventMtx);
            events.swap(hwKeysEvents);
        }
        for (const auto& event : events) {
            dispatchHardwareKeyEvent(event.key, event.pressed, needFullRedraw);
        }

        std::deque<HwEncoderEvent> encoderEvents;
        {
            std::lock_guard<std::mutex> lock(hwEncodersEventMtx);
            encoderEvents.swap(hwEncoderEvents);
        }
        for (const auto& event : encoderEvents) {
            dispatchHardwareEncoderEvent(event.id, event.direction, needFullRedraw);
        }

        if (drawUI(d, appStyles.screen.w, appStyles.screen.h, needFullRedraw)) {
            drawToST7789->render();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}