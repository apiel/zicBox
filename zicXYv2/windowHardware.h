#pragma once

#include <deque>
#include <mutex>

#include "draw/drawToST7789.h"
#include "helpers/GpioEncoder.h"
#include "helpers/GpioKey.h"
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

void dispatchHardwareEncoderEvent(int encoderId, int8_t direction, bool& needFullRedraw)
{
    if (direction == 0) return;

    UiTrack::onEncoder(encoderId, direction, needFullRedraw);
    MasterFx::onEncoder(encoderId, direction, needFullRedraw);
    UiSeq::onEncoder(encoderId, direction);
    UiProject::onEncoder(encoderId, direction);
}

void dispatchHardwareKeyEvent(int key, bool pressed, bool& needFullRedraw)
{
    if (pressed) {
        UiTrack::keyPressed(key, needFullRedraw);
        UiSeq::keyPressed(key, needFullRedraw);
        UiClips::keyPressed(key, needFullRedraw);
        MasterFx::keyPressed(key, needFullRedraw);
        UiProject::keyPressed(key, needFullRedraw);
        TopBar::keyPressed(key, needFullRedraw);
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

void windowHardware(Draw& d, const Styles& appStyles, bool& needFullRedraw)
{
    std::mutex hwKeysEventMtx;
    std::deque<HwKeyEvent> hwKeysEvents;
    std::mutex hwEncodersEventMtx;
    std::deque<HwEncoderEvent> hwEncoderEvents;
    GpioKey gpioKey(
        {
            { 20, KEY_F1 },
            { 16, KEY_F2 },
            { 25, KEY_F3 },
            { 14, KEY_F4 },
            { 2, KEY_F5 },

            { 12, KEY_1 },
            { 1, KEY_2 },
            { 24, KEY_3 },
            { 15, KEY_4 },
            { 7, KEY_5 },
            { 8, KEY_6 },
            { 23, KEY_7 },
            { 22, KEY_8 },
        },
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
        {
            { 1, 26, 13 },
            { 2, 6, 5 },
            { 3, 0, 9 },
            { 4, 27, 4 },
        },
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

        drawUI(d, appStyles.screen.w, appStyles.screen.h, needFullRedraw);
        drawToST7789->render();
    }
}