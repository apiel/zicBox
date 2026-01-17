/** Description:
This C++ header defines a specialized class, `DrawWithSDL`, which serves as the core graphics and interaction manager for a desktop application. It uses the widely known SDL (Simple DirectMedia Layer) library to handle all visual output and user input.

The class is responsible for the fundamental lifecycle of the graphical interface. During its initialization process, it sets up the main application window and the necessary tools, such as a dedicated renderer and a temporary drawing surface (called a texture), to handle graphics efficiently. It ensures a safe closure by accurately recording the window's final position and cleaning up all associated resources when the application exits.

For drawing, it employs a technique where all visual data is first rendered onto a hidden texture, which is then quickly copied to the visible screen. This method ensures smooth, flicker-free updates.

Crucially, `DrawWithSDL` manages user interaction. It continuously listens for low-level desktop events like mouse clicks, keyboard strokes, and touch inputs, and translates these into structured signals for the main program. A key feature is the simulation of physical rotary encoders (knobs). When a user scrolls the mouse wheel, the code checks if the cursor is positioned over specific rectangular zones defined by the user. If it is, the mouse scroll is translated into a turn of a virtual knob, allowing complex program control using standard desktop hardware.

Finally, the class reads configuration data, loading settings such as the initial window position and the specific screen coordinates for these simulated knob zones during startup.

sha: d4f938bfedb89f8191abab3f3850d295ffa51eaf594f37aab6a504464202ec07 
*/
#pragma once

#include <SDL2/SDL.h>
#include <vector>

#include "log.h"
#include "draw/drawDesktop.h"
#include "helpers/getTicks.h"

class DrawWithSDL : public DrawDesktop {
protected:
    SDL_Texture* texture = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Window* window = NULL;

    void emulateEncoder(EventInterface* view, SDL_MouseWheelEvent wheel)
    {
#if SDL_MINOR_VERSION > 24
        uint8_t emulateEncoderId = getEmulatedEncoderId(wheel.mouseX, wheel.mouseY);
#else
        uint8_t emulateEncoderId = 0;
#endif
        view->onEncoder(emulateEncoderId, wheel.y, getTicks());
    }

    std::vector<Rect> zoneEncoders = {};
    uint8_t getEmulatedEncoderId(int32_t x, int32_t y)
    {
        for (uint8_t i = 0; i < zoneEncoders.size(); i++) {
            if (inRect(zoneEncoders[i], { x, y })) {
                return i;
            }
        }
        return 255;
    }

public:
    DrawWithSDL(Draw& draw)
        : DrawDesktop(draw)
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
            draw.styles.screen.w, draw.styles.screen.h,
            flags);

        if (window == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not create window: %s", SDL_GetError());
            throw std::runtime_error("Could not create window");
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        texture = (SDL_Texture*)draw.setTextureRenderer(draw.styles.screen);

        SDL_SetWindowPosition(window, windowX, windowY);
    }

    void render() override
    {
        // first clear display
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // draw pixels
        for (uint16_t i = 0; i < draw.styles.screen.h; i++) {
            for (uint16_t j = 0; j < draw.styles.screen.w; j++) {
                Color color = draw.screenBuffer[i][j];
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
                handleMotion(view, event.tfinger.x * draw.styles.screen.w, event.tfinger.y * draw.styles.screen.h, event.tfinger.fingerId);
                return true;

            case SDL_FINGERDOWN:
                handleMotionDown(view, event.tfinger.x * draw.styles.screen.w, event.tfinger.y * draw.styles.screen.h, event.tfinger.fingerId);
                return true;

            case SDL_FINGERUP:
                handleMotionUp(view, event.tfinger.x * draw.styles.screen.w, event.tfinger.y * draw.styles.screen.h, event.tfinger.fingerId);
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

    void config(nlohmann::json& config) override
    {
        if (config.contains("zonesEncoders")) {
            for (auto zoneEncoder : config["zonesEncoders"]) {
                int x = zoneEncoder[0];
                int y = zoneEncoder[1];
                int w = zoneEncoder[2];
                int h = zoneEncoder[3];
                zoneEncoders.push_back({ { x, y }, { w, h } });
            }
        }

        DrawDesktop::config(config);
    }
};
