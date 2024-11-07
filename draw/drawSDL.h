#ifndef _UI_DRAW_H_
#define _UI_DRAW_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdexcept>

#include "../plugins/components/drawInterface.h"
#include "../plugins/components/utils/color.h"
#include "drawSDLGfx.h"

#define PIXEL_FORMAT SDL_PIXELFORMAT_RGBA8888
// #define PIXEL_FORMAT SDL_PIXELFORMAT_RGB565

class Draw : public DrawInterface {
protected:
    SDL_Texture* texture = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Window* window = NULL;

    bool needRendering = false;

    SDL_Surface* getTextSurface(const char* text, uint32_t size, DrawTextOptions options = {})
    {
        TTF_Font* font = TTF_OpenFont(options.fontPath, size);
        if (font == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open font\n");
            return 0;
        }
        Color color = options.color;
        SDL_Surface* surface = TTF_RenderText_Solid(font, text, { color.r, color.g, color.b, color.a });
        if (surface == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to render text\n");
            return 0;
        }
        TTF_CloseFont(font);

        return surface;
    }

    void textToRenderer(Point position, SDL_Surface* surface, int maxWidth)
    {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create texture\n");
            return;
        }
        // SDL_Rect rect = {position.x, position.y, surface->w, surface->h};
        // SDL_RenderCopy(renderer, texture, NULL, &rect);

        int w = maxWidth < surface->w ? maxWidth : surface->w;
        SDL_Rect rect1 = { 0, 0, w, surface->h };
        SDL_Rect rect2 = { position.x, position.y, w, surface->h };
        SDL_RenderCopy(renderer, texture, &rect1, &rect2);
        SDL_DestroyTexture(texture);
    }

    Color* getStyleColor(char* color)
    {
        if (strcmp(color, "background") == 0) {
            return &styles.colors.background;
        }

        if (strcmp(color, "overlay") == 0) {
            return &styles.colors.overlay;
        }

        if (strcmp(color, "on") == 0) {
            return &styles.colors.on;
        }

        if (strcmp(color, "white") == 0) {
            return &styles.colors.white;
        }

        if (strcmp(color, "blue") == 0) {
            return &styles.colors.blue;
        }

        if (strcmp(color, "red") == 0) {
            return &styles.colors.red;
        }

        if (strcmp(color, "turquoise") == 0) {
            return &styles.colors.turquoise;
        }

        if (strcmp(color, "grey") == 0) {
            return &styles.colors.grey;
        }

        return NULL;
    }

public:
    Draw(Styles& styles)
        : DrawInterface(styles)
    {
    }

    ~Draw()
    {
        if (texture != NULL) {
            SDL_DestroyTexture(texture);
        }
        if (renderer != NULL) {
            SDL_DestroyRenderer(renderer);
        }
        if (window != NULL) {
            SDL_DestroyWindow(window);
        }
    }

    int flags = SDL_WINDOW_SHOWN
#ifdef IS_RPI
    // | SDL_WINDOW_FULLSCREEN_DESKTOP
#endif
        ;

    void init()
    {
        window = SDL_CreateWindow(
            "Zic",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            styles.screen.w, styles.screen.h,
            flags);

        if (window == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not create window: %s", SDL_GetError());
            throw std::runtime_error("Could not create window");
        }

        TTF_Init();

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        texture = (SDL_Texture*)setTextureRenderer(styles.screen);
    }

    void renderNext()
    {
        needRendering = true;
    }

    void triggerRendering()
    {
        if (needRendering) {
            render();
            needRendering = false;
        }
    }

    void render()
    {
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

    int textCentered(Point position, std::string text, uint32_t size, DrawTextOptions options = {})
    {
        options = getDefaultTextOptions(options);
        SDL_Surface* surface = getTextSurface(text.c_str(), size, options);
        int w = surface->w > options.maxWidth ? options.maxWidth : surface->w;
        int x = position.x - (w * 0.5);
        textToRenderer({ x, position.y }, surface, options.maxWidth);
        int xEnd = x + w;
        SDL_FreeSurface(surface);

        return xEnd;
    }

    int text(Point position, std::string text, uint32_t size, DrawTextOptions options = {})
    {
        options = getDefaultTextOptions(options);
        SDL_Surface* surface = getTextSurface(text.c_str(), size, options);
        textToRenderer(position, surface, options.maxWidth);
        int xEnd = position.x + surface->w;
        SDL_FreeSurface(surface);

        return xEnd;
    }

    int textRight(Point position, std::string text, uint32_t size, DrawTextOptions options = {})
    {
        options = getDefaultTextOptions(options);
        SDL_Surface* surface = getTextSurface(text.c_str(), size, options);
        int w = surface->w > options.maxWidth ? options.maxWidth : surface->w;
        int x = position.x - w;
        textToRenderer({ x, position.y }, surface, options.maxWidth);
        SDL_FreeSurface(surface);

        return x;
    }

    void clear()
    {
        Color color = styles.colors.background;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderClear(renderer);
    }

    void filledRect(Point position, Size size, DrawOptions options = {})
    {
        Color color = options.color;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect rect = { position.x, position.y, size.w, size.h };
        SDL_RenderFillRect(renderer, &rect);
    }

    void rect(Point position, Size size, DrawOptions options = {})
    {
        Color color = options.color;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect rect = { position.x, position.y, size.w, size.h };
        SDL_RenderDrawRect(renderer, &rect);
    }

    void filledRect(Point position, Size size, uint8_t radius, DrawOptions options = {})
    {
        filledRect({ position.x + radius, position.y }, { size.w - 2 * radius, size.h }, options);
        filledRect({ position.x, position.y + radius }, { size.w, size.h - 2 * radius }, options);
        filledPie({ position.x + radius, position.y + radius }, radius, 180, 270, options);
        filledPie({ position.x + size.w - radius, position.y + radius }, radius, 270, 360, options);
        filledPie({ position.x + radius, position.y + size.h - radius }, radius, 90, 180, options);
        filledPie({ position.x + size.w - radius, position.y + size.h - radius }, radius, 0, 90, options);
    }

    void rect(Point position, Size size, uint8_t radius, DrawOptions options = {})
    {
        arc({ position.x + radius, position.y + radius }, radius, 180, 270, options);
        line({ position.x, position.y + radius }, { position.x, position.y + size.h - radius }, options);
        arc({ position.x + size.w - radius, position.y + radius }, radius, 270, 360, options);
        line({ position.x + radius, position.y }, { position.x + size.w - radius, position.y }, options);
        arc({ position.x + radius, position.y + size.h - radius }, radius, 90, 180, options);
        line({ position.x + radius, position.y + size.h }, { position.x + size.w - radius, position.y + size.h }, options);
        arc({ position.x + size.w - radius, position.y + size.h - radius }, radius, 0, 90, options);
        line({ position.x + size.w, position.y + size.h - radius }, { position.x + size.w, position.y + radius }, options);
    }

    void arc(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {})
    {
        Color color = options.color;
        // arcRGBA(renderer, position.x, position.y, radius, startAngle, endAngle, color.r, color.g, color.b, color.a);
        aaArcRGBA(renderer, position.x, position.y, radius, radius, startAngle, endAngle, 1, color.r, color.g, color.b, color.a);
    }

    void filledPie(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {})
    {
        Color color = options.color;
        // https://github.com/rtrussell/BBCSDL/blob/master/include/SDL2_gfxPrimitives.h
        aaFilledPieRGBA(renderer, position.x, position.y, radius, radius, startAngle, endAngle, 0, color.r, color.g, color.b, color.a);
    }

    void filledEllipse(Point position, int radiusX, int radiusY, DrawOptions options = {})
    {
        Color color = options.color;
        // https://github.com/rtrussell/BBCSDL/blob/master/include/SDL2_gfxPrimitives.h
        aaFilledEllipseRGBA(renderer, position.x, position.y, radiusX, radiusY, color.r, color.g, color.b, color.a);
    }

    void ellipse(Point position, int radiusX, int radiusY, DrawOptions options = {})
    {
        Color color = options.color;
        // https://github.com/rtrussell/BBCSDL/blob/master/include/SDL2_gfxPrimitives.h
        aaellipseRGBA(renderer, position.x, position.y, radiusX, radiusY, color.r, color.g, color.b, color.a);
    }

    void filledPolygon(std::vector<Point> points, DrawOptions options = {})
    {
        double x[points.size()];
        double y[points.size()];
        for (int i = 0; i < points.size(); i++) {
            x[i] = points[i].x;
            y[i] = points[i].y;
        }

        Color color = options.color;
        // https://github.com/rtrussell/BBCSDL/blob/master/include/SDL2_gfxPrimitives.h
        aaFilledPolygonRGBA(renderer, x, y, points.size(), color.r, color.g, color.b, color.a);
    }

    void polygon(std::vector<Point> points, DrawOptions options = {})
    {
        Sint16 x[points.size()];
        Sint16 y[points.size()];
        for (int i = 0; i < points.size(); i++) {
            x[i] = points[i].x;
            y[i] = points[i].y;
        }

        Color color = options.color;
        // https://github.com/rtrussell/BBCSDL/blob/master/include/SDL2_gfxPrimitives.h
        aapolygonRGBA(renderer, x, y, points.size(), color.r, color.g, color.b, color.a);
    }

    void aaline(Point start, Point end, DrawOptions options = {})
    {
        Color color = options.color;
        // https://github.com/rtrussell/BBCSDL/blob/master/include/SDL2_gfxPrimitives.h
        aalineRGBA(renderer, start.x, start.y, end.x, end.y, color.r, color.g, color.b, color.a);
    }

    void aalines(std::vector<Point> points, DrawOptions options = {})
    {
        for (int i = 0; i < points.size() - 1; i++) {
            aaline(points[i], points[i + 1], options);
        }
    }

    void line(Point start, Point end, DrawOptions options = {})
    {
        Color color = options.color;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawLine(renderer, start.x, start.y, end.x, end.y);
    }

    void lines(std::vector<Point> points, DrawOptions options = {})
    {
        Color color = options.color;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawLines(renderer, (SDL_Point*)points.data(), points.size());
    }

    void pixel(Point position, DrawOptions options = {})
    {
        Color color = options.color;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawPoint(renderer, position.x, position.y);
    }

    void* setTextureRenderer(Size size)
    {
        SDL_Texture* texture = SDL_CreateTexture(renderer, PIXEL_FORMAT, SDL_TEXTUREACCESS_TARGET, size.w, size.h);
        SDL_SetRenderTarget(renderer, texture);
        return texture;
    }

    void setMainRenderer()
    {
        SDL_SetRenderTarget(renderer, texture);
    }

    void destroyTexture(void* texture)
    {
        SDL_DestroyTexture((SDL_Texture*)texture);
    }

    void applyTexture(void* texture, Rect dest)
    {
        SDL_Rect rect = { dest.position.x, dest.position.y, dest.size.w, dest.size.h };
        SDL_RenderCopy(renderer, (SDL_Texture*)texture, NULL, &rect);
        SDL_RenderPresent(renderer);
    }

    Color getColor(char* color)
    {
        // if first char is # then call hex2rgb
        if (color[0] == '#') {
            return hex2rgb(color);
        }

        Color* styleColor = getStyleColor(color);
        if (styleColor != NULL) {
            return *styleColor;
        }

        return styles.colors.white;
    }

    bool config(char* key, char* value)
    {
        /*//md
### SET_COLOR

`SET_COLOR` give the possibility to customize the pre-defined color for the UI. To change a color, use `SET_COLOR: name_of_color #xxxxxx`.

```coffee
SET_COLOR: overlay #00FFFF
```

In this example, we change the `overlay` color to `#00FFFF`.




- `#21252b` ![#21252b](https://via.placeholder.com/15/21252b/000000?text=+) background
- `#00FFFF` ![#00FFFF](https://via.placeholder.com/15/00FFFF/000000?text=+) overlay
- `#00b300` ![#00b300](https://via.placeholder.com/15/00b300/000000?text=+) on
- `#ffffff` ![#ffffff](https://via.placeholder.com/15/ffffff/000000?text=+) white
- `#adcdff` ![#adcdff](https://via.placeholder.com/15/adcdff/000000?text=+) blue
- `#ff8d99` ![#ff8d99](https://via.placeholder.com/15/ff8d99/000000?text=+) red

> This list might be outdated, to get the list of existing colors, look at `./styles.h`
        */
        if (strcmp(key, "SET_COLOR") == 0) {
            char* name = strtok(value, " ");
            char* color = strtok(NULL, " ");
            Color* styleColor = getStyleColor(name);
            if (styleColor != NULL) {
                Color newColor = hex2rgb(color);
                //    *styleColor = newColor; // Dont do like this, to keep transparency
                styleColor->r = newColor.r;
                styleColor->g = newColor.g;
                styleColor->b = newColor.b;
            }
            return true;
        }

        return false;
    }
};

#endif