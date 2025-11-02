#define FS_ROOT_FOLDER "./esp/zicBlock/flash_data"

#include "helpers/getTicks.h"
#include "log.h"
#include "main/uiManager.h"

#include <SDL2/SDL.h>
#include <cstdlib>
#include <stdexcept>
#include <thread>
#include <unistd.h>

SDL_Texture* texture = NULL;
SDL_Renderer* renderer = NULL;
SDL_Window* window = NULL;

// int windowX = SDL_WINDOWPOS_CENTERED;
// int windowY = SDL_WINDOWPOS_CENTERED;
int windowX = 200;
int windowY = 300;
int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALWAYS_ON_TOP;

UIManager ui;

bool appRunning = true;

void quit()
{
    int x, y;
    SDL_GetWindowPosition(window, &x, &y);
    logInfo("Exit on position x: %d, y: %d", x, y);

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);
    SDL_Quit();
}

// void* setTextureRenderer(Size size) { return NULL; }

void init()
{
    logDebug("Initializing SDL");
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize SDL: %s", SDL_GetError());
        throw std::runtime_error("Could not initialize SDL.");
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL video driver: %s", SDL_GetCurrentVideoDriver());

    window = SDL_CreateWindow(
        "Zic",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        ui.width, ui.height,
        flags);

    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not create window: %s", SDL_GetError());
        throw std::runtime_error("Could not create window");
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // texture = (SDL_Texture*)setTextureRenderer({ screenW, screenH });

    SDL_SetWindowPosition(window, windowX, windowY);
}

void render()
{
    // first clear display
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // // draw pixels
    for (uint16_t i = 0; i < ui.height; i++) {
        for (uint16_t j = 0; j < ui.width; j++) {
            // Color color = screenBuffer[i][j];
            bool pixel = ui.draw.getPixel({ j, i });
            uint8_t color = pixel ? 255 : 0;
            SDL_SetRenderDrawColor(renderer, color, color, color, 255);
            SDL_RenderDrawPoint(renderer, j, i);
        }
    }

    // During the whole rendering process, we render into a texture
    // Only at the end, we push the texture to the screen
    //
    // Set renderer pointing to screen
    SDL_SetRenderTarget(renderer, NULL);
    // Copy texture to renderer pointing on the screen
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    // Present renderer
    SDL_RenderPresent(renderer);
    // Set renderer pointinng to texture
    SDL_SetRenderTarget(renderer, texture);
}

void emulateEncoder(SDL_MouseWheelEvent wheel)
{
    int encoderId = 1;
    for (int i = 0; i < 9; i++) {
        Point pos = ui.toneView.valuePos[i];
        if (wheel.mouseX > pos.x && wheel.mouseX < pos.x + ui.toneView.valueSize.w && wheel.mouseY > pos.y && wheel.mouseY < pos.y + ui.toneView.valueSize.h) {
            encoderId = i + 1;
            break;    
        }
    }
    ui.onEncoder(encoderId, wheel.y, getTicks());
}

bool handleEvent()
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Quit");
            return false;

        case SDL_MOUSEWHEEL:
            emulateEncoder(event.wheel);
            return true;

        case SDL_KEYDOWN: {
            if (event.key.repeat) {
                return true;
            }
            // printf("key %d\n", event.key.keysym.scancode);
            // view->onKey(0, event.key.keysym.scancode, 1);
            // logDebug("keydown %d", event.key.keysym.scancode);
            ui.onKey(0, event.key.keysym.scancode, 1);
            return true;
        }

        case SDL_KEYUP: {
            if (event.key.repeat) {
                return true;
            }
            // view->onKey(0, event.key.keysym.scancode, 0);
            // logDebug("keyup %d", event.key.keysym.scancode);
            ui.onKey(0, event.key.keysym.scancode, 0);
            return true;
        }
        }
    }
    return true;
}

void* uiThread(void* = NULL)
{
    init();
    ui.init();
    // int ms = 50;
    int ms = 80;
    logInfo("Rendering with SDL.");
    unsigned long lastUpdate = getTicks();
    while (handleEvent() && appRunning) {
        unsigned long now = getTicks();
        if (now - lastUpdate > ms) {
            lastUpdate = now;
            if (ui.render()) {
                render();
            }
        }
        usleep(1);
    }
    appRunning = false;

    return NULL;
}

void initAudio()
{
    // Ensure audio subsystem is ready
    if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO)) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not init audio subsystem: %s", SDL_GetError());
            throw std::runtime_error("Audio subsystem init failed");
        }
    }

    SDL_AudioSpec want = {};
    want.freq = ui.audio.sampleRate;
    want.format = AUDIO_F32SYS;
    want.channels = ui.audio.channels;
    want.samples = 512;
    want.callback = +[](void* userdata, Uint8* stream, int len) {
        float* fstream = reinterpret_cast<float*>(stream);
        int samples = len / sizeof(float);
        // for (int i = 0; i < samples; i += 2) {
        //     fstream[i] = ui.audio.sample();
        //     fstream[i+1] = fstream[i];
        // }
        for (int i = 0; i < samples; i++) {
            fstream[i] = ui.audio.sample();
        }
    };

    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
    if (!dev) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open audio: %s", SDL_GetError());
        throw std::runtime_error("Audio init failed");
    }

    SDL_PauseAudioDevice(dev, 0); // start playback
}

int main(int argc, char* argv[])
{
    pthread_t ptid;
    pthread_create(&ptid, NULL, &uiThread, NULL);
    pthread_setname_np(ptid, "ui");

    initAudio();
    // wait for uiThread to finish
    pthread_join(ptid, NULL);

    return 0;
}
