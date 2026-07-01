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
#include "helpers/GpioKey.h"
#include "zicXYv2/audioWorker.h"
#include "zicXYv2/ui.h"

namespace {
struct HwKeyEvent {
    int key;
    bool pressed;
};

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
            hwKeysEvents.push_back({ key.key, state == 0 });
        });

    if (gpioKey.init() == 0) {
        gpioKey.startThread();
        logInfo("Hardware keys initialized (GPIO)");
    } else {
        logWarn("Unable to initialize hardware keys (GPIO)");
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

        drawUI(*drawer, appStyles.screen.w, appStyles.screen.h, needFullRedraw);
        drawToST7789->render();
    }
#endif

    keep_running = false;
    aThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}