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

#include "zicGridImpact/audioWorker.h"
#include "zicGridImpact/gridState.h"
#include "zicGridImpact/studio.h"
#include "zicGridImpact/ui.h"

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

    handleEncoderInput(encoderId - 1, scaled);
    needFullRedraw = true;
}

inline void dispatchHardwareKeyEvent(int col, int row, bool pressed, bool& needFullRedraw)
{
    handlePadPress(col, row, pressed);
    needFullRedraw = true;
}
}

inline void runHardware(Draw& d, const Styles& appStyles, bool& needFullRedraw)
{
    std::mutex hwKeysEventMtx;
    std::deque<HwKeyEvent> hwKeysEvents;
    std::mutex hwEncodersEventMtx;
    std::deque<HwEncoderEvent> hwEncoderEvents;

    std::vector<GpioEncoder::Encoder> encoderConfigs = {
        { 1, 20, 19 }, { 2, 13, 6 }, { 3, 5, 7 }, { 4, 8, 11 },
        { 5, 21, 26 }, { 6, 9, 10 }, { 7, 27, 17 }, { 8, 22, 4 },
        { 9, 16, 12 }, { 10, 25, 24 }, { 11, 23, 18 }, { 12, 15, 14 }
    };

    std::string i2cDevice = "/dev/i2c-1";
    uint8_t addrTrellis1 = 0x2E;
    uint8_t addrTrellis2 = 0x30;
    uint8_t addrTrellis3 = 0x2F;

    NeoTrellis trellis1(i2cDevice, addrTrellis1);
    NeoTrellis trellis2(i2cDevice, addrTrellis2);
    NeoTrellis trellis3(i2cDevice, addrTrellis3);

    auto pushPadEvent = [&](int col, int row, bool pressed) {
        std::lock_guard<std::mutex> lock(hwKeysEventMtx);
        hwKeysEvents.push_back({ col, row, pressed });
    };

    if (trellis1.init()) {
        trellis1.setOnKeyPadEvent([&](int key, bool pressed) {
            int col = key % 4;
            int row = key / 4;
            pushPadEvent(col, row, pressed);
        });
    }

    if (trellis2.init()) {
        trellis2.setOnKeyPadEvent([&](int key, bool pressed) {
            int col = 4 + (key % 4);
            int row = key / 4;
            pushPadEvent(col, row, pressed);
        });
    }

    if (trellis3.init()) {
        trellis3.setOnKeyPadEvent([&](int key, bool pressed) {
            int col = 8 + (key % 4);
            int row = key / 4;
            pushPadEvent(col, row, pressed);
        });
    }

    std::unique_ptr<DrawToFB> fbDriver;
    try {
        fbDriver = std::make_unique<DrawToFB>(d, appStyles);
    } catch (...) {
        std::cerr << "[Hardware] Warning: Framebuffer open failed, continuing without FB driver." << std::endl;
    }

    GpioEncoder encoders(encoderConfigs, [&](int id, int8_t direction) {
        std::lock_guard<std::mutex> lock(hwEncodersEventMtx);
        hwEncoderEvents.push_back({ id, direction });
    });

    while (keep_running) {
        trellis1.update();
        trellis2.update();
        trellis3.update();

        {
            std::lock_guard<std::mutex> lock(hwKeysEventMtx);
            while (!hwKeysEvents.empty()) {
                auto ev = hwKeysEvents.front();
                hwKeysEvents.pop_front();
                dispatchHardwareKeyEvent(ev.col, ev.row, ev.pressed, needFullRedraw);
            }
        }

        {
            std::lock_guard<std::mutex> lock(hwEncodersEventMtx);
            while (!hwEncoderEvents.empty()) {
                auto ev = hwEncoderEvents.front();
                hwEncoderEvents.pop_front();
                dispatchHardwareEncoderEvent(ev.id, ev.direction, needFullRedraw);
            }
        }

        d.setScreenSize({ SCREEN_W, SCREEN_H });
        if (drawUI(d, SCREEN_W, SCREEN_H, needFullRedraw)) {
            if (fbDriver) {
                fbDriver->draw();
            }
            // Update NeoTrellis LED colors
            for (int r = 0; r < PAD_ROWS; ++r) {
                for (int c = 0; c < PAD_COLS; ++c) {
                    auto& pad = gridState.pads[c][r];
                    NeoTrellis::Color nc = { pad.color.r, pad.color.g, pad.color.b };
                    if (c < 4) trellis1.setPixelColor(r * 4 + c, nc);
                    else if (c < 8) trellis2.setPixelColor(r * 4 + (c - 4), nc);
                    else trellis3.setPixelColor(r * 4 + (c - 8), nc);
                }
            }
            trellis1.show();
            trellis2.show();
            trellis3.show();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}
