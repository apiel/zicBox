#pragma once

#include <SDL2/SDL.h>

#include "./draw.h"
#include "SDL_EventHandler.h"

class DrawWithSDL : public Draw {
protected:
    EventHandler eventHandler;

    SDL_Texture* texture = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Window* window = NULL;

    int windowX = SDL_WINDOWPOS_CENTERED;
    int windowY = SDL_WINDOWPOS_CENTERED;

    // bool needRendering = false;

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

        eventHandler.config(config);

        Draw::config(config);
    }

    bool handleEvent(EventInterface* view) override
    {
        return eventHandler.handle(view);
    }
};
