#pragma once

#include <chrono>
#include <deque>
#include <fstream>
#include <mutex>
#include <thread>
#include <iostream>

#include "draw/drawToST7789.h"
#include "helpers/GpioEncoder.h"
#include "helpers/GpioKey.h"
#include "libs/nlohmann/json.hpp"
#include "uiDrop.h"
#include "sequenceBrain.h"
#include "audio/engines/drop.h"

extern std::atomic<bool> keep_running;
extern std::mutex audioMutex;

namespace {
struct HwKeyEvent {
    int key;
    bool pressed;
};

struct HwEncoderEvent {
    int id;
    int8_t direction;
};

void dispatchHardwareEncoderEvent(UiDrop& ui, int encoderId, int8_t direction, bool& needFullRedraw)
{
    if (direction == 0) return;
    std::lock_guard<std::mutex> lock(audioMutex);
    ui.handleEncoder(encoderId, direction);
    needFullRedraw = true;
}

void dispatchHardwareKeyEvent(UiDrop& ui, SequenceBrain& brain, Drop& audio, int key, bool pressed, bool& needFullRedraw)
{
    std::lock_guard<std::mutex> lock(audioMutex);
    if (pressed) {
        if (key == KEY_F1) {
            ui.activeSection = SECTION_BRAIN;
            needFullRedraw = true;
        } else if (key == KEY_F2) {
            ui.activeSection = SECTION_KICK;
            needFullRedraw = true;
        } else if (key == KEY_F4) {
            ui.activeSection = SECTION_ACID;
            needFullRedraw = true;
        } else if (key == KEY_F5) {
            ui.activeSection = SECTION_MASTER;
            needFullRedraw = true;
        } else if (key == KEY_8) { // Use KEY_8 as the Spacebar drop trigger on hardware
            brain.spacebarHeld = true;
            audio.performanceMode = true;
            needFullRedraw = true;
        }
    } else {
        if (key == KEY_8) {
            brain.spacebarHeld = false;
            audio.performanceMode = false;
            needFullRedraw = true;
        }
    }
}
}

void runHardware(Draw& d, const Styles& appStyles, bool& needFullRedraw, UiDrop& ui, SequenceBrain& brain, Drop& audio)
{
    std::mutex hwKeysEventMtx;
    std::deque<HwKeyEvent> hwKeysEvents;
    std::mutex hwEncodersEventMtx;
    std::deque<HwEncoderEvent> hwEncoderEvents;

    // Default key definitions matching zicXY
    std::vector<GpioKey::Key> keyConfigs = {
        { 20, KEY_F1 }, { 16, KEY_F2 }, { 25, KEY_F3 }, { 14, KEY_F4 }, { 2, KEY_F5 },
        { 12, KEY_1 }, { 1, KEY_2 }, { 24, KEY_3 }, { 15, KEY_4 }, { 7, KEY_5 }, { 8, KEY_6 }, { 23, KEY_7 }, { 22, KEY_8 }
    };

    // Default encoder definitions matching zicXY
    std::vector<GpioEncoder::Encoder> encoderConfigs = {
        { 1, 26, 13 },
        { 2, 6, 5 },
        { 3, 0, 9 },
        { 4, 27, 4 }
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
            std::cerr << "Failed to parse config.json, using defaults.\n";
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
    }

    GpioEncoder gpioEncoder(
        encoderConfigs,
        [&hwEncodersEventMtx, &hwEncoderEvents](GpioEncoder::Encoder encoder, int8_t direction) {
            std::lock_guard<std::mutex> lock(hwEncodersEventMtx);
            hwEncoderEvents.push_back({ encoder.id, direction });
        });

    if (gpioEncoder.init() == 0) {
        gpioEncoder.startThread();
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
            dispatchHardwareKeyEvent(ui, brain, audio, event.key, event.pressed, needFullRedraw);
        }

        std::deque<HwEncoderEvent> encoderEvents;
        {
            std::lock_guard<std::mutex> lock(hwEncodersEventMtx);
            encoderEvents.swap(hwEncoderEvents);
        }
        for (const auto& event : encoderEvents) {
            dispatchHardwareEncoderEvent(ui, event.id, event.direction, needFullRedraw);
        }

        bool rendered = false;
        {
            std::lock_guard<std::mutex> lock(audioMutex);
            rendered = ui.draw(d, appStyles.screen.w, appStyles.screen.h, needFullRedraw, brain, audio);
        }

        if (rendered) {
            drawToST7789->render();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
