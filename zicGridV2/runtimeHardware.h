#pragma once

#include <chrono>
#include <deque>
#include <fstream>
#include <mutex>
#include <thread>

#include "draw/drawToST7789.h"
#include "helpers/GpioEncoder.h"
#include "helpers/GpioKey.h"
#include "helpers/enc.h"
#include "libs/nlohmann/json.hpp"

#include "zicGridV2/ViewManager.h"
#include "zicGridV2/audioWorker.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/ui.h"

namespace {
struct HwKeyEvent {
    int col;
    int row;
    bool pressed;
};

struct HwEncoderEvent {
    int id;
    int8_t direction;
};

inline static uint32_t encoderLastShiftTicks[TOTAL_ENCODERS] = { 0 };

inline void dispatchHardwareEncoderEvent(int encoderId, int8_t direction, bool& needFullRedraw)
{
    if (direction == 0 || encoderId < 1 || encoderId > TOTAL_ENCODERS) return;

    uint32_t now = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch())
                       .count();
    int scaled = encGetScaledDirection(direction, now, encoderLastShiftTicks[encoderId - 1]);
    encoderLastShiftTicks[encoderId - 1] = now;

    ViewManager::handleEncoder(encoderId, scaled);
}

inline void dispatchHardwareKeyEvent(int col, int row, bool pressed, bool& needFullRedraw)
{
    ViewManager::handlePadPress(col, row, pressed);
}
}

inline void runHardware(Draw& d, const Styles& appStyles, bool& needFullRedraw)
{
    std::mutex hwKeysEventMtx;
    std::deque<HwKeyEvent> hwKeysEvents;
    std::mutex hwEncodersEventMtx;
    std::deque<HwEncoderEvent> hwEncoderEvents;

    // Default 12 encoder GPIO pin assignments (4 cols x 3 rows matrix)
    std::vector<GpioEncoder::Encoder> encoderConfigs = {
        { 1, 26, 13 }, { 2, 6, 5 }, { 3, 0, 9 }, { 4, 27, 4 },
        { 5, 20, 16 }, { 6, 25, 14 }, { 7, 2, 12 }, { 8, 1, 24 },
        { 9, 15, 7 }, { 10, 8, 23 }, { 11, 22, 10 }, { 12, 11, 19 }
    };

    // Load custom hardware pin mappings if config.json exists
    std::string configPath = "config.json";
    if (const char* envPath = std::getenv("ZIC_CONFIG_PATH")) {
        configPath = envPath;
    }
    std::ifstream configFile(configPath);
    if (configFile.is_open()) {
        try {
            nlohmann::json configJson;
            configFile >> configJson;
            if (configJson.contains("encoders") && configJson["encoders"].is_object()) {
                for (size_t i = 0; i < encoderConfigs.size(); ++i) {
                    std::string keyName = "ENCODER_" + std::to_string(i + 1);
                    if (configJson["encoders"].contains(keyName)) {
                        auto encJson = configJson["encoders"][keyName];
                        if (encJson.is_array() && encJson.size() == 2) {
                            encoderConfigs[i].gpioA = encJson[0].get<int>();
                            encoderConfigs[i].gpioB = encJson[1].get<int>();
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            logWarn("Failed to parse config.json for Zic Grid V2 hardware, using defaults.");
        }
    }

    GpioEncoder gpioEncoder(
        encoderConfigs,
        [&hwEncodersEventMtx, &hwEncoderEvents](GpioEncoder::Encoder encoder, int8_t direction) {
            std::lock_guard<std::mutex> lock(hwEncodersEventMtx);
            hwEncoderEvents.push_back({ encoder.id, direction });
        });

    if (gpioEncoder.init() == 0) {
        gpioEncoder.startThread();
        logInfo("Hardware 12 encoders initialized (GPIO)");
    } else {
        logWarn("Unable to initialize hardware encoders (GPIO)");
    }

    auto drawToST7789 = std::make_unique<DrawToST7789>(d);
    drawToST7789->setResetPin(17);
    drawToST7789->setDcPin(3);
    drawToST7789->setYRamMargin(0);
    drawToST7789->init();

    while (keep_running) {
        std::deque<HwKeyEvent> keyEvents;
        {
            std::lock_guard<std::mutex> lock(hwKeysEventMtx);
            keyEvents.swap(hwKeysEvents);
        }
        for (const auto& ev : keyEvents) {
            dispatchHardwareKeyEvent(ev.col, ev.row, ev.pressed, needFullRedraw);
        }

        std::deque<HwEncoderEvent> encEvents;
        {
            std::lock_guard<std::mutex> lock(hwEncodersEventMtx);
            encEvents.swap(hwEncoderEvents);
        }
        for (const auto& ev : encEvents) {
            dispatchHardwareEncoderEvent(ev.id, ev.direction, needFullRedraw);
        }

        drawUI(d, SCREEN_W, SCREEN_H, needFullRedraw);
        drawToST7789->render();
    }
}
