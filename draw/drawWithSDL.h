#pragma once

#include <SDL2/SDL.h>

#include "./draw.h"
#include "helpers/getTicks.h"
#include "motion.h"
#include "plugins/components/EventInterface.h"

#ifndef MAX_SCREEN_MOTION
// The current display only support 5 touch point
#define MAX_SCREEN_MOTION 5
#endif

class DrawWithSDL : public Draw {
protected:
    SDL_Texture* texture = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Window* window = NULL;

    int windowX = SDL_WINDOWPOS_CENTERED;
    int windowY = SDL_WINDOWPOS_CENTERED;

    // bool needRendering = false;

    Motion motions[MAX_SCREEN_MOTION];

#if SDL_MINOR_VERSION <= 24
    uint8_t emulateEncoderId = 0;
#endif

    MotionInterface* getMotion(int id)
    {
        for (int i = 0; i < MAX_SCREEN_MOTION; ++i) {
            if (motions[i].id == id) {
                return &motions[i];
            }
        }
        return NULL;
    }

    MotionInterface* getOldestMotion()
    {
        MotionInterface* oldest = &motions[0];
        for (int i = 1; i < MAX_SCREEN_MOTION; ++i) {
            if (motions[i].id < oldest->id) {
                oldest = &motions[i];
            }
        }
        return oldest;
    }

    void handleMotion(EventInterface* view, int x, int y, int id)
    {
        if (id < 0) {
            return;
        }

#ifndef IS_RPI
#if SDL_MINOR_VERSION <= 24
        emulateEncoderId = getEmulatedEncoderId(x, y);
#endif
#endif

        MotionInterface* motion = getMotion(id);
        if (motion) {
            motion->move(x, y);
            view->onMotion(*motion);
        }
    }

    void handleMotionUp(EventInterface* view, int x, int y, int id)
    {
        if (id < 0) {
            return;
        }

        MotionInterface* motion = getMotion(id);
        if (motion) {
            motion->move(x, y);
            view->onMotionRelease(*motion);
            motion->setId(-1);
        }
    }

    void handleMotionDown(EventInterface* view, int x, int y, int id)
    {
        if (id < 0) {
            return;
        }

        MotionInterface* motion = getOldestMotion();
        motion->init(id, x, y);
        view->onMotion(*motion);
    }

    std::vector<Rect> zoneEncoders = {};
    uint8_t getEmulatedEncoderId(int32_t x, int32_t y)
    {
        for (uint8_t i = 0; i < zoneEncoders.size(); i++) {
            // if (x >= zoneEncoders[i].position.x && x < zoneEncoders[i].position.x + zoneEncoders[i].size.w &&
            //     y >= zoneEncoders[i].position.y && y < zoneEncoders[i].position.y + zoneEncoders[i].size.h) {
            if (inRect(zoneEncoders[i], { x, y })) {
                return i;
            }
        }

        return 255;
    }

    void emulateEncoder(EventInterface* view, SDL_MouseWheelEvent wheel)
    {
#if SDL_MINOR_VERSION > 24
        uint8_t emulateEncoderId = getEmulatedEncoderId(wheel.mouseX, wheel.mouseY);
#endif
        view->onEncoder(emulateEncoderId, wheel.y, getTicks());
    }

public:
    DrawWithSDL(Styles& styles)
        : Draw(styles)
    {
    }

    void quit() override
    {
        int x, y;
        SDL_GetWindowPosition(window, &x, &y);
        logInfo("Exit on position x: %d, y: %d", x, y);

        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyTexture(texture);
        SDL_Quit();
    }

    int flags = SDL_WINDOW_SHOWN
        | SDL_WINDOW_ALWAYS_ON_TOP
#ifdef IS_RPI
    // | SDL_WINDOW_FULLSCREEN_DESKTOP
#endif
        // | SDL_WINDOW_RESIZABLE
        ;

    void init() override
    {
        logDebug("Initializing SDL");
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize SDL: %s", SDL_GetError());
            throw std::runtime_error("Could not initialize SDL.");
        }

        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL video driver: %s", SDL_GetCurrentVideoDriver());

#ifdef IS_RPI
        SDL_ShowCursor(SDL_DISABLE);
#endif

        window = SDL_CreateWindow(
            "Zic",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            styles.screen.w, styles.screen.h,
            flags);

        if (window == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not create window: %s", SDL_GetError());
            throw std::runtime_error("Could not create window");
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        texture = (SDL_Texture*)setTextureRenderer(styles.screen);

        SDL_SetWindowPosition(window, windowX, windowY);
    }

    void render() override
    {
        // first clear display
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // draw pixels
        for (uint16_t i = 0; i < styles.screen.h; i++) {
            for (uint16_t j = 0; j < styles.screen.w; j++) {
                Color color = screenBuffer[i][j];
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
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

    void config(nlohmann::json& config) override
    {
        if (config.contains("windowPosition")) {
            nlohmann::json windowPosition = config["windowPosition"];
            if (windowPosition.contains("x") && windowPosition.contains("y")) {
                windowX = windowPosition["x"];
                windowY = windowPosition["y"];
            }
        }

        if (config.contains("zonesEncoders")) {
            for (nlohmann::json zoneEncoder : config["zonesEncoders"]) {
                int x = zoneEncoder[0];
                int y = zoneEncoder[1];
                int w = zoneEncoder[2];
                int h = zoneEncoder[3];
                zoneEncoders.push_back({ { x, y }, { w, h } });
            }
        }

        Draw::config(config);
    }

    bool handleEvent(EventInterface* view) override
    {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Quit");
                return false;

            case SDL_MOUSEMOTION:
                handleMotion(view, event.motion.x, event.motion.y, event.motion.which);
                return true;

            case SDL_MOUSEBUTTONDOWN:
                handleMotionDown(view, event.motion.x, event.motion.y, event.motion.which);
                return true;

            case SDL_MOUSEBUTTONUP:
                handleMotionUp(view, event.motion.x, event.motion.y, event.motion.which);
                return true;

            case SDL_MOUSEWHEEL:
                emulateEncoder(view, event.wheel);
                return true;

            case SDL_FINGERMOTION:
                handleMotion(view, event.tfinger.x * styles.screen.w, event.tfinger.y * styles.screen.h, event.tfinger.fingerId);
                return true;

            case SDL_FINGERDOWN:
                handleMotionDown(view, event.tfinger.x * styles.screen.w, event.tfinger.y * styles.screen.h, event.tfinger.fingerId);
                return true;

            case SDL_FINGERUP:
                handleMotionUp(view, event.tfinger.x * styles.screen.w, event.tfinger.y * styles.screen.h, event.tfinger.fingerId);
                return true;

            case SDL_KEYDOWN: {
                if (event.key.repeat) {
                    return true;
                }
                // printf("key %d\n", event.key.keysym.scancode);
                view->onKey(0, event.key.keysym.scancode, 1);
                return true;
            }

            case SDL_KEYUP: {
                if (event.key.repeat) {
                    return true;
                }
                view->onKey(0, event.key.keysym.scancode, 0);
                return true;
            }
            }
        }
        return true;
    }
};
