#ifndef _UI_DRAW_H_
#define _UI_DRAW_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdexcept>

#include "drawGfx.h"
#include "plugins/components/drawInterface.h"

#ifdef USE_WAVESHARE
#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "LCD_1in47.h"

#ifndef WAVESHARE_RGB
#define WAVESHARE_RGB(r, g, b) (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3))
#endif
#endif

#define PIXEL_FORMAT SDL_PIXELFORMAT_RGBA8888

class Draw : public DrawInterface {
protected:
    SDL_Texture* texture = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Window* window = NULL;

#ifdef USE_WAVESHARE
    UWORD* BlackImage;
#endif

    bool needRendering = false;

    SDL_Surface* getTextSurface(const char* text, Color color, uint32_t size, const char* fontPath)
    {
        TTF_Font* font = TTF_OpenFont(fontPath, size);
        if (font == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open font\n");
            return 0;
        }
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

    float min(float a, float b)
    {
        return a < b ? a : b;
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

    Color hex2rgb(char* hex)
    {

        hex++;
        unsigned int color = strtol(hex, NULL, 16);
        return Color({
            .r = (uint8_t)((color & 0x00FF0000) >> 16),
            .g = (uint8_t)((color & 0x0000FF00) >> 8),
            .b = (uint8_t)((color & 0x000000FF)),
            .a = 255,
        });
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

    void init()
    {
        window = SDL_CreateWindow(
            "Zic",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            styles.screen.w, styles.screen.h,
            SDL_WINDOW_SHOWN
#ifdef IS_RPI
                | SDL_WINDOW_FULLSCREEN_DESKTOP
#endif
        );

        if (window == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not create window: %s", SDL_GetError());
            throw std::runtime_error("Could not create window");
        }

        TTF_Init();

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        texture = (SDL_Texture*)setTextureRenderer(styles.screen);

#ifdef USE_WAVESHARE
        if (DEV_ModuleInit() != 0) {
            DEV_ModuleExit();
            exit(0);
        }

        /* LCD Init */
        printf("1.47inch LCD demo...\r\n");
        LCD_1IN47_Init(HORIZONTAL);
        LCD_1IN47_Clear(BLACK);
        LCD_SetBacklight(1023);

        UDOUBLE Imagesize = LCD_1IN47_HEIGHT * LCD_1IN47_WIDTH * 2;
        printf("Imagesize = %d\r\n", Imagesize);
        if ((BlackImage = (UWORD*)malloc(Imagesize)) == NULL) {
            printf("Failed to apply for black memory...\r\n");
            exit(0);
        }
        /*1.Create a new image cache named IMAGE_RGB and fill it with white*/
        Paint_NewImage(BlackImage, LCD_1IN47_WIDTH, LCD_1IN47_HEIGHT, 90, BLACK, 16);
        Paint_Clear(BLACK);
#endif
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
#ifdef USE_WAVESHARE
        // FIXME need to find a more efficient way to access pixel and draw them to the SPI display
        Uint32 r_mask, g_mask, b_mask, a_mask;
        int bbp;
        SDL_PixelFormatEnumToMasks(PIXEL_FORMAT, &bbp, &r_mask, &g_mask, &b_mask, &a_mask);
        SDL_Surface* surface = SDL_CreateRGBSurface(0, styles.screen.w, styles.screen.h, bbp, r_mask, g_mask, b_mask, a_mask);
        SDL_RenderReadPixels(renderer, NULL, PIXEL_FORMAT, (void*)surface->pixels, surface->pitch);

        for (int x = 0; x < styles.screen.w; x++) {
            for (int y = 0; y < styles.screen.h; y++) {
                SDL_Color color;
                SDL_GetRGBA(((Uint32*)surface->pixels)[x + y * styles.screen.w], surface->format, &color.r, &color.g, &color.b, &color.a);
                Paint_SetPixel(x, y, WAVESHARE_RGB(color.r, color.g, color.b));
            }
        }
        LCD_1IN47_Display(BlackImage);
#endif

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

    int textCentered(Point position, const char* text, Color color, uint32_t size, DrawTextOptions options = {})
    {
        options = getDefaultTextOptions(options);
        SDL_Surface* surface = getTextSurface(text, color, size, options.fontPath);
        int x = position.x - (surface->w * 0.5);
        textToRenderer({ x, position.y }, surface, options.maxWidth);
        int xEnd = x + surface->w;
        SDL_FreeSurface(surface);

        return xEnd;
    }

    int text(Point position, const char* text, Color color, uint32_t size, DrawTextOptions options = {})
    {
        options = getDefaultTextOptions(options);
        SDL_Surface* surface = getTextSurface(text, color, size, options.fontPath);
        textToRenderer(position, surface, options.maxWidth);
        int xEnd = position.x + surface->w;
        SDL_FreeSurface(surface);

        return xEnd;
    }

    int textRight(Point position, const char* text, Color color, uint32_t size, DrawTextOptions options = {})
    {
        options = getDefaultTextOptions(options);
        SDL_Surface* surface = getTextSurface(text, color, size, options.fontPath);
        int x = position.x - surface->w;
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

    void filledRect(Point position, Size size, Color color)
    {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect rect = { position.x, position.y, size.w, size.h };
        SDL_RenderFillRect(renderer, &rect);
    }

    void rect(Point position, Size size, Color color)
    {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect rect = { position.x, position.y, size.w, size.h };
        SDL_RenderDrawRect(renderer, &rect);
    }

    void filledPie(Point position, int radius, int startAngle, int endAngle, Color color)
    {
        // https://github.com/rtrussell/BBCSDL/blob/master/include/SDL2_gfxPrimitives.h
        aaFilledPieRGBA(renderer, position.x, position.y, radius, radius, startAngle, endAngle, 0, color.r, color.g, color.b, color.a);
    }

    void filledEllipse(Point position, int radiusX, int radiusY, Color color)
    {
        // https://github.com/rtrussell/BBCSDL/blob/master/include/SDL2_gfxPrimitives.h
        aaFilledEllipseRGBA(renderer, position.x, position.y, radiusX, radiusY, color.r, color.g, color.b, color.a);
    }

    void ellipse(Point position, int radiusX, int radiusY, Color color)
    {
        // https://github.com/rtrussell/BBCSDL/blob/master/include/SDL2_gfxPrimitives.h
        aaellipseRGBA(renderer, position.x, position.y, radiusX, radiusY, color.r, color.g, color.b, color.a);
    }

    void filledPolygon(std::vector<Point> points, Color color)
    {
        double x[points.size()];
        double y[points.size()];
        for (int i = 0; i < points.size(); i++) {
            x[i] = points[i].x;
            y[i] = points[i].y;
        }
        // https://github.com/rtrussell/BBCSDL/blob/master/include/SDL2_gfxPrimitives.h
        aaFilledPolygonRGBA(renderer, x, y, points.size(), color.r, color.g, color.b, color.a);
    }

    void polygon(std::vector<Point> points, Color color)
    {
        Sint16 x[points.size()];
        Sint16 y[points.size()];
        for (int i = 0; i < points.size(); i++) {
            x[i] = points[i].x;
            y[i] = points[i].y;
        }
        // https://github.com/rtrussell/BBCSDL/blob/master/include/SDL2_gfxPrimitives.h
        aapolygonRGBA(renderer, x, y, points.size(), color.r, color.g, color.b, color.a);
    }

    void aaline(Point start, Point end, Color color)
    {
        // https://github.com/rtrussell/BBCSDL/blob/master/include/SDL2_gfxPrimitives.h
        aalineRGBA(renderer, start.x, start.y, end.x, end.y, color.r, color.g, color.b, color.a);
    }

    void aalines(std::vector<Point> points, Color color)
    {
        for (int i = 0; i < points.size() - 1; i++) {
            aaline(points[i], points[i + 1], color);
        }
    }

    void line(Point start, Point end, Color color)
    {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawLine(renderer, start.x, start.y, end.x, end.y);
    }

    void lines(std::vector<Point> points, Color color)
    {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawLines(renderer, (SDL_Point*)points.data(), points.size());
    }

    void pixel(Point position, Color color)
    {
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

    Color darken(Color color, float amount)
    {
        return {
            (uint8_t)(color.r * (1.0f - amount)),
            (uint8_t)(color.g * (1.0f - amount)),
            (uint8_t)(color.b * (1.0f - amount)),
            color.a
        };
    }

    Color lighten(Color color, float amount)
    {
        return {
            (uint8_t)min(color.r * (1.0f + amount), 255.0f),
            (uint8_t)min(color.g * (1.0f + amount), 255.0f),
            (uint8_t)min(color.b * (1.0f + amount), 255.0f),
            color.a
        };
    }

    Color alpha(Color color, float amount)
    {
        return {
            color.r,
            color.g,
            color.b,
            (uint8_t)(amount * 255.0f),
        };
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

    // TODO use string...
    void setColor(char* name, char* color)
    {
        Color* styleColor = getStyleColor(name);
        if (styleColor != NULL) {
            Color newColor = hex2rgb(color);
            //    *styleColor = newColor; // Dont do like this, to keep transparency
            styleColor->r = newColor.r;
            styleColor->g = newColor.g;
            styleColor->b = newColor.b;
        }
    }
};

#endif