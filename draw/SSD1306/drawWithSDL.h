#ifndef _UI_DRAW_WITH_SDL_H_
#define _UI_DRAW_WITH_SDL_H_

#include <SDL2/SDL.h>

#include "./draw.h"

class DrawWithSDL : public Draw {
protected:
    SDL_Texture* texture = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Window* window = NULL;

    // bool needRendering = false;

public:
    DrawWithSDL(Styles& styles)
        : Draw(styles)
    {
    }

    int flags = SDL_WINDOW_SHOWN
#ifdef IS_RPI
    // | SDL_WINDOW_FULLSCREEN_DESKTOP
#endif
        ;

    void init() override
    {
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

        // for testing
        line({ 0, 0 }, { 127, 64 });
        line({ 0, 64 }, { 127, 0 });
        rect({ 10, 10 }, { 30, 30 });
        arc({ 100, 32 }, 10, 0, 180);

        render();
    }

    void render() override
    {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (uint16_t i = 0; i < SSD1306_BUFFER_SIZE; i++) {
            if (oledBuffer[i] != 0x00) {
                Point position = { i % styles.screen.w, (i / styles.screen.w) * 8 };
                // SDL_RenderDrawPoint(renderer, position.x, position.y);
                for (int j = 0; j < 8; j++) {
                    if (oledBuffer[i] & (1 << j)) {
                        SDL_RenderDrawPoint(renderer, position.x, position.y + j);
                    }
                }
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
};

#endif