/** Description:
This C++ structure defines a specialized tool for creating and managing a graphical display using a popular multimedia library called SDL (Simple DirectMedia Layer). Think of this as the engine that powers a simple graphical output application.

**Basic Idea**

The primary function of this component is to act as a translator. It takes raw, low-level data representing individual black-and-white pixels from an internal buffer and converts them into a visible image on a computer screen.

**How It Works**

1.  **Initialization:** When started, the engine sets up the core display environment. It initializes the SDL library, creates the main viewing area (the window, titled "Zic"), and prepares the specialized drawing mechanisms (the renderer). It also sets up a hidden temporary canvas (a texture) where all graphics are drawn first. This setup ensures that the window is positioned correctly, sometimes forced to stay on top, or customized for specific hardware like a Raspberry Pi.
2.  **Drawing (Rendering):** The core process involves updating the screen rapidly. First, the entire display is cleared (usually to black). Then, the system iterates through all the stored pixel data. If the data indicates a pixel should be "on," the system draws a white dot on the screen. To prevent the user from seeing partial updates or flickering, it employs a technique called double-buffering: it draws everything onto the hidden canvas first, and only when the drawing is complete, it instantly swaps or copies that finished image onto the visible window.
3.  **Cleanup:** When the application is finished, this tool carefully shuts down every component it created—the window, the renderer, and the temporary canvas—to ensure system resources are properly released.

sha: 0fa79c3032859903647052a39a0b4f5a6ba7ca15f1d4a9c9770193d5f3f04723 
*/
#pragma once

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
        // draw pixels
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
