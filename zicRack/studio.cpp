#include <pthread.h>
#include <thread>

#include "zicRack/audioWorker.h"
#include "zicRack/ui.h"
#ifdef DRAW_SMFL
#include "zicRack/windowSFML.h"
#endif

int main()
{
    snd_pcm_t* pcm_h = audioInit();
    pthread_setname_np(pthread_self(), "zicBox_UI");

    std::thread aThread(audioWorker, pcm_h);
    pthread_setname_np(aThread.native_handle(), "zicBox_Audio");

    Styles appStyles = {
        .screen = { 800, 480 }, .margin = 2, .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } }
    };
    auto drawer = std::make_unique<Draw>(appStyles);
    bool needFullRedraw = true;

#ifdef DRAW_SMFL
    windowSFML(*drawer, needFullRedraw);
#else
    while (keep_running) {
        drawUI(*drawer, appStyles.screen.w, appStyles.screen.h, needFullRedraw);
        // TODO copy buffer to screen
    }
#endif

    keep_running = false;
    aThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}