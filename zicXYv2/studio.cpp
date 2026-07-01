#include <pthread.h>
#include <deque>
#include <mutex>
#include <thread>

#ifdef DRAW_SMFL
#include "zicXYv2/windowSFML.h" // Stay in first position for key definition
#else
#include "draw/drawToST7789.h"
#endif

#include "log.h"
#include "helpers/GpioEncoder.h"
#include "helpers/GpioKey.h"
#include "zicXYv2/audioWorker.h"
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

uint32_t getNowMs()
{
    return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

bool dispatchTrackEncoder(int col, int delta, uint32_t now, bool& needFullRedraw)
{
    if (studio.currentView != ViewTrack || studio.tracks[studio.selTrack] == nullptr) return false;

    Track& trk = *studio.tracks[studio.selTrack];
    const int paramsPerRow = 4;
    const int maxVisibleRows = trk.showWaveform ? 4 : 5;
    const int sbWidth = 4;
    const int sbGap = 3;
    const int usableWidth = SCREEN_W - (MARGIN * 2) - (sbWidth + sbGap);
    const int adjustedColW = usableWidth / paramsPerRow;

    size_t totalParamCount = 4 + trk.engine->getParamCount();
    int totalParamRows = ((int)totalParamCount + paramsPerRow - 1) / paramsPerRow;
    int startRow = 0;
    int activeRow = trk.encodersSelection;
    if (activeRow < startRow) {
        startRow = activeRow;
    } else if (activeRow >= startRow + maxVisibleRows) {
        startRow = activeRow - maxVisibleRows + 1;
    }
    if (startRow > totalParamRows - maxVisibleRows) {
        startRow = std::max(0, totalParamRows - maxVisibleRows);
    }

    int visualRow = trk.encodersSelection - startRow;
    visualRow = std::clamp(visualRow, 0, maxVisibleRows - 1);

    Point position = {
        MARGIN + col * adjustedColW + (adjustedColW / 2),
        UiTrack::paramsTopY + visualRow * UiDraw::ROW_H + (UiDraw::ROW_H / 2),
    };
    return UiTrack::mouseWheelScrolled(position, delta, SCREEN_W, now, false, needFullRedraw);
}

bool dispatchMasterEncoder(int col, int delta, uint32_t now)
{
    if (studio.currentView != ViewMaster) return false;

    const int paramsPerRow = 4;
    const int maxVisibleRows = 5;
    const int sbWidth = 3;
    const int sbGap = 1;
    const int usableWidth = SCREEN_W - (MARGIN * 2) - (sbWidth + sbGap);
    const int adjustedColW = usableWidth / paramsPerRow;

    size_t totalParamRows = ((int)MasterFx::paramCount + paramsPerRow - 1) / paramsPerRow;
    int startRow = 0;
    int activeRow = MasterFx::encodersSelection;
    if (activeRow < startRow) {
        startRow = activeRow;
    } else if (activeRow >= startRow + maxVisibleRows) {
        startRow = activeRow - maxVisibleRows + 1;
    }
    if (startRow > (int)totalParamRows - maxVisibleRows) {
        startRow = std::max(0, (int)totalParamRows - maxVisibleRows);
    }

    int visualRow = MasterFx::encodersSelection - startRow;
    visualRow = std::clamp(visualRow, 0, maxVisibleRows - 1);

    Point position = {
        MARGIN + col * adjustedColW + (adjustedColW / 2),
        MasterFx::paramsTopY + visualRow * UiDraw::ROW_H + (UiDraw::ROW_H / 2),
    };
    return MasterFx::mouseWheelScrolled(position, delta, SCREEN_W, now, false);
}

bool dispatchSeqEncoder(int col, int delta, uint32_t now)
{
    if (studio.currentView != ViewSeq) return false;

    const int paramsPerRow = 4;
    const int sbWidth = 4;
    const int sbGap = 3;
    const int usableWidth = SCREEN_W - (MARGIN * 2) - (sbWidth + sbGap);
    const int adjustedColW = usableWidth / paramsPerRow;

    Point position = {
        MARGIN + col * adjustedColW + (adjustedColW / 2),
        UiSeq::paramsTopY + (UiDraw::ROW_H / 2),
    };
    return UiSeq::mouseWheelScrolled(position, delta, SCREEN_W, now, false);
}

bool dispatchClipsEncoder(int col, int delta, uint32_t now)
{
    if (studio.currentView != ViewClips) return false;

    const int paramsPerRow = 4;
    const int usableWidth = SCREEN_W - (MARGIN * 2);
    const int adjustedColW = usableWidth / paramsPerRow;

    int rowH = UiClips::gridRect.size.h > 0 ? UiClips::gridRect.size.h / MAX_TRACKS : UiDraw::ROW_H;
    int paramsTopY = UiClips::top + rowH * MAX_TRACKS + 4;

    Point position = {
        MARGIN + col * adjustedColW + (adjustedColW / 2),
        paramsTopY + (UiDraw::ROW_H / 2),
    };
    return UiClips::mouseWheelScrolled(position, delta, SCREEN_W, now, false);
}

void dispatchHardwareEncoderEvent(int encoderId, int8_t direction, bool& needFullRedraw)
{
    if (direction == 0) return;

    int col = std::clamp(encoderId - 1, 0, 3);
    int delta = (int)direction;
    uint32_t now = getNowMs();

    if (dispatchTrackEncoder(col, delta, now, needFullRedraw)) return;
    if (dispatchMasterEncoder(col, delta, now)) return;
    if (dispatchSeqEncoder(col, delta, now)) return;
    if (dispatchClipsEncoder(col, delta, now)) return;
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

int main()
{
    logInfo("Starting zicXYv2");

    snd_pcm_t* pcm_h = audioInit();
    pthread_setname_np(pthread_self(), "zicBox_UI");

    std::thread aThread(audioWorker, pcm_h);
    pthread_setname_np(aThread.native_handle(), "zicBox_Audio");

    Styles appStyles = {
        .screen = { SCREEN_W, SCREEN_H }, .margin = 2, .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } }
    };
    auto drawer = std::make_unique<Draw>(appStyles);
    bool needFullRedraw = true;

    std::string currentProject = getCurrentLoadedProject();
    if (!currentProject.empty()) {
        loadProject(PROJECT_FOLDER + "/" + currentProject);
    }

#ifdef DRAW_SMFL
    windowSFML(*drawer, needFullRedraw);
#else
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

    // DrawToST7789 drawToST7789(*drawer);
    auto drawToST7789 = std::make_unique<DrawToST7789>(*drawer);
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

        drawUI(*drawer, appStyles.screen.w, appStyles.screen.h, needFullRedraw);
        drawToST7789->render();
    }
#endif

    keep_running = false;
    aThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}