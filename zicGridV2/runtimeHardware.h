#pragma once

#include <chrono>
#include <deque>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "draw/drawToFB.h"
#include "helpers/GpioEncoder.h"
#include "helpers/GpioKey.h"
#include "helpers/NeoTrellis.h"
#include "helpers/enc.h"
#include "libs/nlohmann/json.hpp"

#include "zicGridV2/ViewManager.h"
#include "zicGridV2/audioWorker.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/ui.h"

struct HwKeyEvent {
    int col;
    int row;
    bool pressed;
};

struct HwEncoderEvent {
    int id;
    int8_t direction;
};

namespace {
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
    needFullRedraw = true;
}

inline void dispatchHardwareKeyEvent(int col, int row, bool pressed, bool& needFullRedraw)
{
    ViewManager::handlePadPress(col, row, pressed);
    needFullRedraw = true;
}
}

inline void runHardware(Draw& d, const Styles& appStyles, bool& needFullRedraw)
{
    std::mutex hwKeysEventMtx;
    std::deque<HwKeyEvent> hwKeysEvents;
    std::mutex hwEncodersEventMtx;
    std::deque<HwEncoderEvent> hwEncoderEvents;

    // Default 12 encoder GPIO pin assignments (PixelController layout == "grid")
    std::vector<GpioEncoder::Encoder> encoderConfigs = {
        { 1, 20, 19 }, { 2, 13, 6 }, { 3, 5, 7 }, { 4, 8, 11 },
        { 5, 21, 26 }, { 6, 9, 10 }, { 7, 27, 17 }, { 8, 22, 4 },
        { 9, 16, 12 }, { 10, 25, 24 }, { 11, 23, 18 }, { 12, 15, 14 }
    };

    // Default NeoTrellis I2C bus and addresses
    std::string i2cDevice = "/dev/i2c-1";
    uint8_t addrTrellis1 = 0x2E; // Tile 1 (Left: Cols 0..3)
    uint8_t addrTrellis2 = 0x30; // Tile 2 (Middle: Cols 4..7)
    uint8_t addrTrellis3 = 0x2F; // Tile 3 (Right: Cols 8..11)

    // Load custom hardware pin mappings and NeoTrellis configs if config.json exists
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

            if (configJson.contains("neotrellis") && configJson["neotrellis"].is_object()) {
                auto neoJson = configJson["neotrellis"];
                if (neoJson.contains("device") && neoJson["device"].is_string()) {
                    i2cDevice = neoJson["device"].get<std::string>();
                }
                if (neoJson.contains("tile1")) {
                    if (neoJson["tile1"].is_number()) addrTrellis1 = neoJson["tile1"].get<uint8_t>();
                    else if (neoJson["tile1"].is_string()) addrTrellis1 = (uint8_t)std::stoi(neoJson["tile1"].get<std::string>(), nullptr, 0);
                }
                if (neoJson.contains("tile2")) {
                    if (neoJson["tile2"].is_number()) addrTrellis2 = neoJson["tile2"].get<uint8_t>();
                    else if (neoJson["tile2"].is_string()) addrTrellis2 = (uint8_t)std::stoi(neoJson["tile2"].get<std::string>(), nullptr, 0);
                }
                if (neoJson.contains("tile3")) {
                    if (neoJson["tile3"].is_number()) addrTrellis3 = neoJson["tile3"].get<uint8_t>();
                    else if (neoJson["tile3"].is_string()) addrTrellis3 = (uint8_t)std::stoi(neoJson["tile3"].get<std::string>(), nullptr, 0);
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

    // Initialize 3 NeoTrellis boards
    std::unique_ptr<NeoTrellis> trellis1;
    std::unique_ptr<NeoTrellis> trellis2;
    std::unique_ptr<NeoTrellis> trellis3;

    auto makeTrellisCallback = [&](int tileColOffset) {
        return [&hwKeysEventMtx, &hwKeysEvents, tileColOffset](uint8_t num, bool pressed) {
            if (num >= NEO_TRELLIS_NUM_KEYS) return;
            int localCol = num % 4;
            int localRow = num / 4;
            int col = tileColOffset + localCol;
            int row = localRow;
            if (col >= 0 && col < PAD_COLS && row >= 0 && row < PAD_ROWS) {
                gridState.pads[col][row].pressed = pressed;
                std::lock_guard<std::mutex> lock(hwKeysEventMtx);
                hwKeysEvents.push_back({ col, row, pressed });
            }
        };
    };

    try {
        trellis1 = std::make_unique<NeoTrellis>(makeTrellisCallback(0));
        trellis1->begin(addrTrellis1, i2cDevice.c_str());
        trellis1->startThread("neotrellis1");
        logInfo("NeoTrellis Tile 1 initialized (0x%02X)", addrTrellis1);
    } catch (const std::exception& e) {
        logWarn("NeoTrellis Tile 1 (0x%02X) init failed: %s", addrTrellis1, e.what());
        trellis1.reset();
    }

    try {
        trellis2 = std::make_unique<NeoTrellis>(makeTrellisCallback(4));
        trellis2->begin(addrTrellis2, i2cDevice.c_str());
        trellis2->startThread("neotrellis2");
        logInfo("NeoTrellis Tile 2 initialized (0x%02X)", addrTrellis2);
    } catch (const std::exception& e) {
        logWarn("NeoTrellis Tile 2 (0x%02X) init failed: %s", addrTrellis2, e.what());
        trellis2.reset();
    }

    try {
        trellis3 = std::make_unique<NeoTrellis>(makeTrellisCallback(8));
        trellis3->begin(addrTrellis3, i2cDevice.c_str());
        trellis3->startThread("neotrellis3");
        logInfo("NeoTrellis Tile 3 initialized (0x%02X)", addrTrellis3);
    } catch (const std::exception& e) {
        logWarn("NeoTrellis Tile 3 (0x%02X) init failed: %s", addrTrellis3, e.what());
        trellis3.reset();
    }

    auto drawToFB = std::make_unique<DrawToFB>(d);
    drawToFB->init();

    static Color lastPadColors[PAD_COLS][PAD_ROWS] = {};

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

        // Sync pad LED colors with physical NeoTrellis hardware
        for (int r = 0; r < PAD_ROWS; ++r) {
            for (int c = 0; c < PAD_COLS; ++c) {
                Color effColor = getPadEffectiveColor(c, r);
                if (effColor.r != lastPadColors[c][r].r ||
                    effColor.g != lastPadColors[c][r].g ||
                    effColor.b != lastPadColors[c][r].b) {

                    lastPadColors[c][r] = effColor;
                    int localCol = c % 4;
                    int num = r * 4 + localCol;
                    NeoTrellis::Color neoColor(effColor.r, effColor.g, effColor.b);

                    if (c < 4 && trellis1) {
                        trellis1->updateColorArray(num, neoColor);
                    } else if (c >= 4 && c < 8 && trellis2) {
                        trellis2->updateColorArray(num, neoColor);
                    } else if (c >= 8 && c < 12 && trellis3) {
                        trellis3->updateColorArray(num, neoColor);
                    }
                }
            }
        }

        drawUI(d, SCREEN_W, SCREEN_H, needFullRedraw);
        drawToFB->render();
    }
}

