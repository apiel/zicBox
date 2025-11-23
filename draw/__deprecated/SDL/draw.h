/** Description:
This code defines the blueprint for a sophisticated graphical drawing engine used to build a user interface. It acts as the core visual manager for the application.

**Core Functionality and Technology**

The engine is built on top of the popular SDL library (Simple DirectMedia Layer), which handles creating windows and managing graphics, along with the SDL_ttf library specifically for high-quality text rendering.

The `Draw` class manages the screen display by utilizing three main components: the main window (where the user sees the content), a renderer (the tool used to paint), and an off-screen texture or canvas.

**How the Drawing Process Works**

To ensure smooth, flicker-free updates, the system uses a technique called double-buffering. Instead of drawing directly onto the screen, all updates (shapes, text, colors) are first drawn onto the hidden off-screen canvas (the texture). Only when a full frame is complete is this entire canvas instantly copied to the visible screen, making the user experience seamless. The methods `renderNext` and `triggerRendering` manage this efficient update cycle.

**Drawing Capabilities**

This drawing engine provides a rich set of tools to create UI elements, including:

*   **Basic Shapes:** Drawing simple lines, rectangles, and clearing the screen.
*   **Advanced Shapes:** Creating filled shapes, circles, arcs, and polygons. It includes support for rounded rectangles and anti-aliased (smooth-edged) graphics.
*   **Text Handling:** Drawing text with options for precise alignment (left, centered, or right-aligned) and automatic font management.
*   **Styling:** It references defined UI styles to quickly access colors (like "background" or "primary") and manages global settings like screen dimensions.

**Configuration**

The system allows developers and users to customize the visual environment. Through configuration settings, it can adjust the screen resolution and modify the application's predefined color palette using specific color codes.

sha: 2fa9e38c04f36a2f68c47bf98a1cf112e4381ba985e14aef83d446aad52b31dc 
*/
#ifndef _UI_DRAW_H_
#define _UI_DRAW_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdexcept>

#include "../../plugins/components/drawInterface.h"
#include "../../plugins/components/utils/color.h"
#include "drawGfx.h"

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
        TTF_Font* font = (TTF_Font*)options.font;
        // if (font == NULL) {
        //     SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "------------> Using default font %s\n", styles.font.regular);
        //     SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "------------> Using font size %d\n", size);
        //     font = TTF_OpenFont(styles.font.regular, size);
        // }
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

    Color* getStyleColor(std::string& color)
    {
        if (color == "background") {
            return &styles.colors.background;
        }

        if (color == "white") {
            return &styles.colors.white;
        }

        if (color == "primary") {
            return &styles.colors.primary;
        }

        if (color == "secondary") {
            return &styles.colors.secondary;
        }

        if (color == "tertiary") {
            return &styles.colors.tertiary;
        }

        if (color == "quaternary") {
            return &styles.colors.quaternary;
        }

        if (color == "text") {
            return &styles.colors.text;
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

        TTF_Init();

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        texture = (SDL_Texture*)setTextureRenderer(styles.screen);
    }

    void renderNext() override
    {
        needRendering = true;
    }

    void triggerRendering() override
    {
        if (needRendering) {
            render();
            needRendering = false;
        }
    }

    void render() override
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

    DrawTextOptions getDefaultTextOptions(DrawTextOptions options, int fontSize)
    {
        return {
            options.color,
            options.font == NULL ? getFont(NULL, fontSize) : options.font,
            options.maxWidth ? options.maxWidth : styles.screen.w,
        };
    }

    // TODO cache font...
    void* getFont(const char* name = NULL, int size = -1) override
    {
        if (name == NULL || strcmp(name, "default") == 0) {
            return TTF_OpenFont(styles.font.regular, size);
        }
        return TTF_OpenFont(name, size);
    }

    int textCentered(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) override
    {
        options = getDefaultTextOptions(options, size);
        SDL_Surface* surface = getTextSurface(text.c_str(), size, options);
        int w = surface->w > options.maxWidth ? options.maxWidth : surface->w;
        int x = position.x - (w * 0.5);
        textToRenderer({ x, position.y }, surface, options.maxWidth);
        int xEnd = x + w;
        SDL_FreeSurface(surface);

        return xEnd;
    }

    int text(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) override
    {
        options = getDefaultTextOptions(options, size);
        SDL_Surface* surface = getTextSurface(text.c_str(), size, options);
        textToRenderer(position, surface, options.maxWidth);
        int xEnd = position.x + surface->w;
        SDL_FreeSurface(surface);

        return xEnd;
    }

    int textRight(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) override
    {
        options = getDefaultTextOptions(options, size);
        SDL_Surface* surface = getTextSurface(text.c_str(), size, options);
        int w = surface->w > options.maxWidth ? options.maxWidth : surface->w;
        int x = position.x - w;
        textToRenderer({ x, position.y }, surface, options.maxWidth);
        SDL_FreeSurface(surface);

        return x;
    }

    void clear() override
    {
        Color color = styles.colors.background;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderClear(renderer);
    }

    void filledRect(Point position, Size size, DrawOptions options = {}) override
    {
        Color color = options.color;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect rect = { position.x, position.y, size.w, size.h };
        SDL_RenderFillRect(renderer, &rect);
    }

    void rect(Point position, Size size, DrawOptions options = {}) override
    {
        Color color = options.color;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect rect = { position.x, position.y, size.w, size.h };
        SDL_RenderDrawRect(renderer, &rect);
    }

    void filledRect(Point position, Size size, uint8_t radius, DrawOptions options = {}) override
    {
        filledRect({ position.x + radius, position.y }, { size.w - 2 * radius, size.h }, options);
        filledRect({ position.x, position.y + radius }, { size.w, size.h - 2 * radius }, options);
        filledPie({ position.x + radius, position.y + radius }, radius, 180, 270, options);
        filledPie({ position.x + size.w - radius, position.y + radius }, radius, 270, 360, options);
        filledPie({ position.x + radius, position.y + size.h - radius }, radius, 90, 180, options);
        filledPie({ position.x + size.w - radius, position.y + size.h - radius }, radius, 0, 90, options);
    }

    void rect(Point position, Size size, uint8_t radius, DrawOptions options = {}) override
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

    void arc(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {}) override
    {
        Color color = options.color;
        // arcRGBA(renderer, position.x, position.y, radius, startAngle, endAngle, color.r, color.g, color.b, color.a);
        aaArcRGBA(renderer, position.x, position.y, radius, radius, startAngle, endAngle, 1, color.r, color.g, color.b, color.a);
    }

    void filledPie(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {})
    {
        Color color = options.color;
        // https://github.com/rtrussell/BBCSDL/blob/master/include/SDL2_gfxPrimitives.h
        aaFilledEllipseRGBA(renderer, position.x, position.y, radius, radius, color.r, color.g, color.b, color.a);
    }

    void circle(Point position, int radius, DrawOptions options = {}) override
    {
        Color color = options.color;
        // https://github.com/rtrussell/BBCSDL/blob/master/include/SDL2_gfxPrimitives.h
        aaellipseRGBA(renderer, position.x, position.y, radius, radius, color.r, color.g, color.b, color.a);
    }

    void filledCircle(Point position, int radius, DrawOptions options = {}) override
    {
        filledPie(position, radius, 0, 360, options);
    }

    void filledPolygon(std::vector<Point> points, DrawOptions options = {}) override
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

    void line(Point start, Point end, DrawOptions options = {}) override
    {
        Color color = options.color;
        // https://github.com/rtrussell/BBCSDL/blob/master/include/SDL2_gfxPrimitives.h
        aalineRGBA(renderer, start.x, start.y, end.x, end.y, color.r, color.g, color.b, color.a);

        // SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        // SDL_RenderDrawLine(renderer, start.x, start.y, end.x, end.y);
    }

    void lines(std::vector<Point> points, DrawOptions options = {}) override
    {
        Color color = options.color;
        for (int i = 0; i < points.size() - 1; i++) {
            aalineRGBA(renderer, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, color.r, color.g, color.b, color.a);
        }

        // SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        // SDL_RenderDrawLines(renderer, (SDL_Point*)points.data(), points.size());
    }

    void pixel(Point position, DrawOptions options = {}) override
    {
        Color color = options.color;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawPoint(renderer, position.x, position.y);
    }

    void* setTextureRenderer(Size size) override
    {
        SDL_Texture* texture = SDL_CreateTexture(renderer, PIXEL_FORMAT, SDL_TEXTUREACCESS_TARGET, size.w, size.h);
        SDL_SetRenderTarget(renderer, texture);
        return texture;
    }

    void setMainRenderer() override
    {
        SDL_SetRenderTarget(renderer, texture);
    }

    void destroyTexture(void* texture) override
    {
        SDL_DestroyTexture((SDL_Texture*)texture);
    }

    void applyTexture(void* texture, Rect dest) override
    {
        SDL_Rect rect = { dest.position.x, dest.position.y, dest.size.w, dest.size.h };
        SDL_RenderCopy(renderer, (SDL_Texture*)texture, NULL, &rect);
        SDL_RenderPresent(renderer);
    }

    Color getColor(std::string color, Color defaultColor = { 0xFF, 0xFF, 0xFF }) override
    {
        // if first char is # then call hex2rgb
        if (color[0] == '#') {
            return hex2rgb(color);
        }

        Color* styleColor = getStyleColor(color);
        if (styleColor != NULL) {
            return *styleColor;
        }

        return defaultColor;
    }

    bool config(char* key, char* value) override
    {
        /*#md
### SET_COLOR

`SET_COLOR` give the possibility to customize the pre-defined color for the UI. To change a color, use `SET_COLOR: name_of_color #xxxxxx`.

```coffee
SET_COLOR: background #00FFFF
```

In this example, we change the `background` color to `#00FFFF`.




- `#21252b` ![#21252b](https://via.placeholder.com/15/21252b/000000?text=+) background
- `#00b300` ![#00b300](https://via.placeholder.com/15/00b300/000000?text=+) on
- `#ffffff` ![#ffffff](https://via.placeholder.com/15/ffffff/000000?text=+) white
- `#adcdff` ![#adcdff](https://via.placeholder.com/15/adcdff/000000?text=+) blue
- `#ff8d99` ![#ff8d99](https://via.placeholder.com/15/ff8d99/000000?text=+) red

> This list might be outdated, to get the list of existing colors, look at `./styles.h`
        */
        if (strcmp(key, "SET_COLOR") == 0) {
            std::string name = strtok(value, " ");
            std::string color = strtok(NULL, " ");
            Color* styleColor = getStyleColor(name);
            if (styleColor != NULL) {
                Color newColor = hex2rgb(color);
                //    *styleColor = newColor; // Dont do like this, to keep transparency
                styleColor->r = newColor.r;
                styleColor->g = newColor.g;
                styleColor->b = newColor.b;
            }
            return true;
        } else if (strcmp(key, "SCREEN") == 0) {
            styles.screen.w = atoi(strtok(value, " "));
            styles.screen.h = atoi(strtok(NULL, " "));
            return true;
        } else if (strcmp(key, "SDL_FLAGS") == 0) {
            flags |= atoi(value);
            return true;
        }

        return false;
    }
};

#endif