#pragma once

#include <algorithm>
#include <alsa/asoundlib.h>
#include <atomic>
#include <chrono>
#include <deque>
#include <fcntl.h>
#include <linux/input.h>
#include <mutex>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>

#include "draw/drawToFB.h"
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

struct HwTouchEvent {
    enum Type { Down, Move, Up } type;
    int x;
    int y;
};

static uint32_t stepLastShiftTicks[4] = { 0, 0, 0, 0 };

void dispatchHardwareEncoderEvent(int encoderId, int8_t direction, bool& needFullRedraw)
{
    if (direction == 0 || encoderId < 1 || encoderId > 4) return;
    uint32_t now = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    int scaled = encGetScaledDirection(direction, now, stepLastShiftTicks[encoderId - 1]);
    stepLastShiftTicks[encoderId - 1] = now;

    UiTrack::onEncoder(encoderId, scaled, needFullRedraw);
    MasterFx::onEncoder(encoderId, scaled, needFullRedraw);
    UiSeq::onEncoder(encoderId, scaled);
    UiMenu::onEncoder(encoderId, scaled);
    UiClips::onEncoder(encoderId, scaled);
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
            ? studio.tracks[trkIdx]->sequence[studio.selStep].note : 60;
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        studio.tracks[trkIdx]->engine->noteOff(note);
    }
}

void touchWorker(std::mutex& mtx, std::deque<HwTouchEvent>& queue)
{
    int fd = open("/dev/input/event1", O_RDONLY | O_NONBLOCK);
    if (fd < 0) return;

    struct input_absinfo absX {}, absY {};
    int minX = 0, maxX = 4095, minY = 0, maxY = 4095;
    if (ioctl(fd, EVIOCGABS(ABS_X), &absX) == 0 && absX.maximum > absX.minimum) { minX = absX.minimum; maxX = absX.maximum; }
    if (ioctl(fd, EVIOCGABS(ABS_Y), &absY) == 0 && absY.maximum > absY.minimum) { minY = absY.minimum; maxY = absY.maximum; }

    constexpr bool SWAP_XY = true;
    constexpr bool INVERT_X = true;
    constexpr bool INVERT_Y = true;

    int rawX = (minX + maxX) / 2, rawY = (minY + maxY) / 2;
    bool touching = false, pendingDown = false, pendingMove = false, pendingUp = false;

    auto mapCoords = [&]() -> std::pair<int, int> {
        float nx = std::clamp(float(rawX - minX) / float(maxX - minX), 0.0f, 1.0f);
        float ny = std::clamp(float(rawY - minY) / float(maxY - minY), 0.0f, 1.0f);
        if (SWAP_XY) std::swap(nx, ny);
        if (INVERT_X) nx = 1.0f - nx;
        if (INVERT_Y) ny = 1.0f - ny;
        return { int(nx * (SCREEN_W - 1)), int(ny * (SCREEN_H - 1)) };
    };

    struct input_event ev {};
    while (keep_running) {
        if (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
            if (ev.type == EV_ABS) {
                if (ev.code == ABS_X) { rawX = ev.value; if (touching) pendingMove = true; }
                else if (ev.code == ABS_Y) { rawY = ev.value; if (touching) pendingMove = true; }
            } else if (ev.type == EV_KEY && ev.code == BTN_TOUCH) {
                if (ev.value == 1 && !touching) { touching = true; pendingDown = true; }
                else if (ev.value == 0 && touching) { touching = false; pendingUp = true; }
            } else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
                auto [sx, sy] = mapCoords();
                std::lock_guard<std::mutex> lock(mtx);
                if (pendingDown) { queue.push_back({ HwTouchEvent::Down, sx, sy }); pendingDown = false; }
                else if (pendingMove && touching) { queue.push_back({ HwTouchEvent::Move, sx, sy }); pendingMove = false; }
                if (pendingUp) { queue.push_back({ HwTouchEvent::Up, sx, sy }); pendingUp = false; }
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(4));
        }
    }
    close(fd);
}

void midiWorker(std::mutex& keyMtx, std::deque<HwKeyEvent>& keyQ, std::mutex& encMtx, std::deque<HwEncoderEvent>& encQ)
{
    snd_rawmidi_t* midiIn = nullptr;
    if (snd_rawmidi_open(&midiIn, nullptr, "hw:3,0,0", SND_RAWMIDI_NONBLOCK) < 0) return;

    uint8_t status = 0, data[2] = { 0, 0 };
    int dataIdx = 0;
    int lastCCVal[128];
    for (int i = 0; i < 128; ++i) lastCCVal[i] = -1;

    auto handleMsg = [&](uint8_t st, uint8_t d1, uint8_t d2) {
        uint8_t cmd = st & 0xF0;

        if (cmd == 0x90 || cmd == 0x80) {
            bool pressed = (cmd == 0x90 && d2 > 0);
            int mappedKey = 0;
            // Top Mute buttons 1-5 -> KEY_F1..KEY_F5
            if (d1 == 1) mappedKey = KEY_F1;
            else if (d1 == 4) mappedKey = KEY_F2;
            else if (d1 == 7) mappedKey = KEY_F3;
            else if (d1 == 10) mappedKey = KEY_F4;
            else if (d1 == 13) mappedKey = KEY_F5;
            // Bottom Rec Arm buttons 1-8 -> KEY_1..KEY_8
            else if (d1 == 3) mappedKey = KEY_1;
            else if (d1 == 6) mappedKey = KEY_2;
            else if (d1 == 9) mappedKey = KEY_3;
            else if (d1 == 12) mappedKey = KEY_4;
            else if (d1 == 15) mappedKey = KEY_5;
            else if (d1 == 18) mappedKey = KEY_6;
            else if (d1 == 21) mappedKey = KEY_7;
            else if (d1 == 24) mappedKey = KEY_8;

            if (mappedKey > 0) {
                std::lock_guard<std::mutex> lock(keyMtx);
                keyQ.push_back({ mappedKey, pressed });
                return;
            }
            if (pressed) {
                if (d1 == 25) studio.isPlaying = !studio.isPlaying; // Bank Left -> Play/Stop
                else if (d1 == 26) studio.currentCombinationKey = (studio.currentCombinationKey == KeyShift) ? KeyNone : KeyShift; // Bank Right -> Shift
                else if (d1 == 27 && studio.selTrack < (int)studio.tracks.size()) studio.tracks[studio.selTrack]->isMuted = !studio.tracks[studio.selTrack]->isMuted; // Solo -> Mute active track
            }
            return;
        }

        if (cmd == 0xB0) {
            // Top Knobs 1-4 -> Encoders 1-4
            int encId = (d1 == 16) ? 1 : (d1 == 20) ? 2 : (d1 == 24) ? 3 : (d1 == 28) ? 4 : 0;
            if (encId > 0) {
                int8_t delta = 0;
                if (lastCCVal[d1] == -1) delta = (d2 > 64) ? 1 : (d2 < 64 ? -1 : 0);
                else if (d2 > lastCCVal[d1] || (d2 == 127 && lastCCVal[d1] == 127)) delta = 1;
                else if (d2 < lastCCVal[d1] || (d2 == 0 && lastCCVal[d1] == 0)) delta = -1;
                lastCCVal[d1] = d2;
                if (delta != 0) { std::lock_guard<std::mutex> lock(encMtx); encQ.push_back({ encId, delta }); }
                return;
            }
            // Faders 1-8 -> Track Volumes, Fader 9 (CC 62) -> Master Volume
            const uint8_t faderCC[8] = { 19, 23, 27, 31, 49, 53, 57, 61 };
            for (int t = 0; t < 8; ++t) {
                if (d1 == faderCC[t] && t < (int)studio.tracks.size() && studio.tracks[t]) {
                    studio.tracks[t]->volume = float(d2) / 127.0f;
                    return;
                }
            }
            if (d1 == 62) studio.volume = float(d2) / 127.0f;
        }
    };

    uint8_t byte = 0;
    while (keep_running) {
        if (snd_rawmidi_read(midiIn, &byte, 1) == 1) {
            if (byte >= 0xF8) continue;
            if (byte & 0x80) { status = byte; dataIdx = 0; }
            else if (status >= 0x80 && status < 0xF0) {
                data[dataIdx++] = byte;
                int expected = ((status & 0xF0) == 0xC0 || (status & 0xF0) == 0xD0) ? 1 : 2;
                if (dataIdx >= expected) { handleMsg(status, data[0], expected == 2 ? data[1] : 0); dataIdx = 0; }
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
    snd_rawmidi_close(midiIn);
}
}

void runHardware(Draw& d, const Styles& appStyles, bool& needFullRedraw)
{
    std::mutex hwKeysEventMtx, hwEncodersEventMtx, hwTouchEventMtx;
    std::deque<HwKeyEvent> hwKeysEvents;
    std::deque<HwEncoderEvent> hwEncoderEvents;
    std::deque<HwTouchEvent> hwTouchEvents;

    std::thread touchThread(touchWorker, std::ref(hwTouchEventMtx), std::ref(hwTouchEvents));
    std::thread midiThread(midiWorker, std::ref(hwKeysEventMtx), std::ref(hwKeysEvents), std::ref(hwEncodersEventMtx), std::ref(hwEncoderEvents));

    auto drawToFB = std::make_unique<DrawToFB>(d, "/dev/fb1");
    drawToFB->init();

    while (keep_running) {
        std::deque<HwKeyEvent> events;
        { std::lock_guard<std::mutex> lock(hwKeysEventMtx); events.swap(hwKeysEvents); }
        for (const auto& ev : events) dispatchHardwareKeyEvent(ev.key, ev.pressed, needFullRedraw);

        std::deque<HwEncoderEvent> encEvents;
        { std::lock_guard<std::mutex> lock(hwEncodersEventMtx); encEvents.swap(hwEncoderEvents); }
        for (const auto& ev : encEvents) dispatchHardwareEncoderEvent(ev.id, ev.direction, needFullRedraw);

        std::deque<HwTouchEvent> tEvents;
        { std::lock_guard<std::mutex> lock(hwTouchEventMtx); tEvents.swap(hwTouchEvents); }
        for (const auto& te : tEvents) {
            if (te.type == HwTouchEvent::Down) {
                if (te.y < 18) {
                    int col = te.x / (appStyles.screen.w / 8);
                    if (col >= 0 && col < 5) dispatchHardwareKeyEvent(KEY_F1 + col, true, needFullRedraw);
                }
                TopBar::mouseButtonPressed({ te.x, te.y }, needFullRedraw);
                UiTrack::mouseButtonPressed({ te.x, te.y });
                UiSeq::mouseButtonPressed({ te.x, te.y }, appStyles.screen.w, needFullRedraw);
                UiClips::mouseButtonPressed({ te.x, te.y }, appStyles.screen.w, needFullRedraw);
                UiMenu::mouseButtonPressed({ te.x, te.y }, needFullRedraw);
                needFullRedraw = true;
            } else if (te.type == HwTouchEvent::Move) {
                UiTrack::mouseMoved({ te.x, te.y }, appStyles.screen.w);
                needFullRedraw = true;
            } else if (te.type == HwTouchEvent::Up) {
                for (int k = KEY_F1; k <= KEY_F5; ++k) dispatchHardwareKeyEvent(k, false, needFullRedraw);
                UiTrack::mouseButtonReleased();
                needFullRedraw = true;
            }
        }

        if (drawUI(d, appStyles.screen.w, appStyles.screen.h, needFullRedraw)) {
            drawToFB->render();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    if (touchThread.joinable()) touchThread.join();
    if (midiThread.joinable()) midiThread.join();
}
