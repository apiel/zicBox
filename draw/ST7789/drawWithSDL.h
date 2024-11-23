#ifndef _UI_DRAW_WITH_SDL_H_
#define _UI_DRAW_WITH_SDL_H_

#include <SDL2/SDL.h>

#include "./draw.h"

class DrawWithSDL : public Draw {
protected:
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

    int flags = SDL_WINDOW_SHOWN
        | SDL_WINDOW_ALWAYS_ON_TOP
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

        SDL_SetWindowPosition(window, windowX, windowY);
    }

    void render() override
    {
        // first clear display
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // // draw pixels
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        // for (uint16_t i = 0; i < ST7789_BUFFER_SIZE; i++) {
        //     if (oledBuffer[i] != 0x00) {
        //         Point position = { i % styles.screen.w, (i / styles.screen.w) * 8 };
        //         // SDL_RenderDrawPoint(renderer, position.x, position.y);
        //         for (int j = 0; j < 8; j++) {
        //             if (oledBuffer[i] & (1 << j)) {
        //                 SDL_RenderDrawPoint(renderer, position.x, position.y + j);
        //             }
        //         }
        //     }
        // }

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

    bool config(char* key, char* value)
    {
        if (strcmp(key, "WINDOW_POSITION") == 0) {
            windowX = atoi(value);
            windowY = atoi(value);
            return true;
        }
        return Draw::config(key, value);
    }
};

#endif