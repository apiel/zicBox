#include <pthread.h>
#include <thread>

#ifdef DRAW_SMFL
#include "zicXYv2/windowSFML.h" // Stay in first position for key definition
#else
#include "draw/drawToST7789.h"
#endif

#include "log.h"
#include "zicXYv2/audioWorker.h"
#include "zicXYv2/ui.h"

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
    // DrawToST7789 drawToST7789(*drawer);
    auto drawToST7789 = std::make_unique<DrawToST7789>(*drawer);
    drawToST7789->setResetPin(17);
    drawToST7789->setDcPin(3);
    drawToST7789->setYRamMargin(0);
    drawToST7789->init();
    while (keep_running) {
        drawUI(*drawer, appStyles.screen.w, appStyles.screen.h, needFullRedraw);
        drawToST7789->render();
    }
#endif

    keep_running = false;
    aThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}