#include <pthread.h>
#include <sched.h>
#include <thread>

#ifdef DRAW_SMFL
#include "zicXYv2/runtimeDesktopSFML.h" // Stay in first position for key definition
#else
#include "zicXYv2/runtimeHardware.h"
#endif

#include "log.h"
#include "zicXYv2/audioWorker.h"
#include "zicXYv2/ui.h"

namespace {
void setThreadRealtime(pthread_t thread, int priority, const char* name)
{
    sched_param sch {};
    sch.sched_priority = priority;
    int rc = pthread_setschedparam(thread, SCHED_FIFO, &sch);
    if (rc != 0) {
        logWarn("Unable to set realtime priority for %s (need CAP_SYS_NICE/root)", name);
    } else {
        logInfo("Realtime priority set for %s", name);
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
    setThreadRealtime(aThread.native_handle(), 30, "audio thread");

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
    runDesktopSFML(*drawer, needFullRedraw);
#else
    runHardware(*drawer, appStyles, needFullRedraw);
#endif

    keep_running = false;
    aThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}