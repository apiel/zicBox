#ifndef _UI_DRAW_H_
#define _UI_DRAW_H_

#include "../../helpers/gpio.h"
#include "../../helpers/st7789.h"
#include "../../plugins/components/utils/color.h"

// #define USE_SPI_DEV_MEM
#ifdef USE_SPI_DEV_MEM
// sudo apt-get install libraspberradiusYpi-dev raspberradiusYpi-kernel-headers
// sudo chown 0:0 test2
// sudo chmod u+s test2
// see:
// https://raspberradiusYpi.stackexchange.com/questions/40105/access-gpio-pins-without-root-no-access-to-dev-mem-tradiusY-running-as-root
#include "../../helpers/SpiDevMem.h"
#else
#include "../../helpers/SpiDevSpi.h"
#endif

#include "../../log.h"
#include "../../plugins/components/drawInterface.h"
#include "fonts/ArialBold.h"
#include "fonts/ArialNormal.h"
#include "fonts/BigFont.h"
#include "fonts/Sinclair_M.h"
#include "fonts/Sinclair_S.h"
#include "fonts/Ubuntu.h"
#include "fonts/UbuntuBold.h"
#include <cmath>
#include <stdexcept>
#include <string.h>
#include <string>

#include <arpa/inet.h> // htons

// Use max height and width
#define ST7789_ROWS 240
#define ST7789_COLS 240

// res go to pin 15
#define GPIO_TFT_RESET_PIN 22
// DC go to pin 11
#define GPIO_TFT_DATA_CONTROL 17
// BLK go to pin 13 (should be optional or can be connected directly to 3.3v)
#define GPIO_TFT_BACKLIGHT 27

class Draw : public DrawInterface {
public:
    Color screenBuffer[ST7789_ROWS][ST7789_COLS];
    uint16_t cacheBuffer[ST7789_ROWS][ST7789_COLS];

protected:
    bool needRendering = false;

    Spi spi = Spi(GPIO_TFT_DATA_CONTROL);
    ST7789 st7789;

    uint8_t* getFont(const char* name = NULL)
    {
        if (name == NULL) {
            return Sinclair_S;
        }

        if (strcmp(name, "ArialBold") == 0) {
            return ArialBold;
        } else if (strcmp(name, "ArialNormal") == 0) {
            return ArialNormal;
        } else if (strcmp(name, "BigFont") == 0) {
            return BigFont;
        } else if (strcmp(name, "Sinclair_M") == 0) {
            return Sinclair_M;
        } else if (strcmp(name, "Sinclair_S") == 0) {
            return Sinclair_S;
        } else if (strcmp(name, "Ubuntu") == 0) {
            return Ubuntu;
        } else if (strcmp(name, "UbuntuBold") == 0) {
            return UbuntuBold;
        } else {
            throw std::runtime_error("Unknown font " + std::string(name));
        }
    }

    // uint16_t toU16rgb(Color color)
    // {
    //     return (color.r << 16) | (color.g << 8) | color.b;
    // }

    void lineVertical(Point start, Point end, DrawOptions options = {})
    {
        int y = start.y;
        int len = end.y;
        if (start.y > end.y) {
            y = end.y;
            len = start.y;
        }
        for (; y <= len; y++) {
            pixel({ start.x, y }, options);
        }
    }

    void lineHorizontal(Point start, Point end, DrawOptions options = {})
    {
        int x = start.x;
        int len = end.x;
        if (start.x > end.x) {
            x = end.x;
            len = start.x;
        }
        for (; x <= len; x++) {
            pixel({ x, start.y }, options);
        }
    }

    void lineDiagonal(Point start, Point end, DrawOptions options = {})
    {
        if (options.antiAliasing) {
            aaLineDiagonal(start, end, options);
            return;
        }
        unsigned int dx = (end.x > start.x ? end.x - start.x : start.x - end.x);
        short xstep = end.x > start.x ? 1 : -1;
        unsigned int dy = (end.y > start.y ? end.y - start.y : start.y - end.y);
        short ystep = end.y > start.y ? 1 : -1;
        int col = start.x, row = start.y;

        if (dx < dy) {
            int t = -(dy >> 1);
            while (true) {
                pixel({ col, row }, options);
                if (row == end.y)
                    return;
                row += ystep;
                t += dx;
                if (t >= 0) {
                    col += xstep;
                    t -= dy;
                }
            }
        } else {
            int t = -(dx >> 1);
            while (true) {
                pixel({ col, row }, options);
                if (col == end.x)
                    return;
                col += xstep;
                t += dy;
                if (t >= 0) {
                    row += ystep;
                    t -= dx;
                }
            }
        }
    }

    void aaLineDiagonal(Point start, Point end, DrawOptions options, bool draw_endpoint = true)
    {
        int32_t xx0 = start.x;
        int32_t yy0 = start.y;
        int32_t xx1 = end.x;
        int32_t yy1 = end.y;

        int dx, dy, tmp, xdir, y0p1, x0pxdir;
        if (yy0 > yy1) {
            tmp = yy0;
            yy0 = yy1;
            yy1 = tmp;
            tmp = xx0;
            xx0 = xx1;
            xx1 = tmp;
        }

        dx = xx1 - xx0;
        dy = yy1 - yy0;
        if (dx >= 0) {
            xdir = 1;
        } else {
            xdir = -1;
            dx = (-dx);
        }

#define AAbits 8

        uint32_t erracc = 0;
        uint32_t intshift = 32 - AAbits;

        pixel({ start.x, start.y }, options);

        if (dy > dx) {
            uint32_t erradj = ((dx << 16) / dy) << 16;
            x0pxdir = xx0 + xdir;
            while (--dy) {
                uint32_t erracctmp = erracc;
                erracc += erradj;
                if (erracc <= erracctmp) {
                    xx0 = x0pxdir;
                    x0pxdir += xdir;
                }
                yy0++;
                uint32_t wgt = (erracc >> intshift) & 255;
                pixelWeight({ xx0, yy0 }, options, 255 - wgt);
                pixelWeight({ x0pxdir, yy0 }, options, wgt);
            }

        } else {
            uint32_t erradj = ((dy << 16) / dx) << 16;
            y0p1 = yy0 + 1;
            while (--dx) {

                uint32_t erracctmp = erracc;
                erracc += erradj;
                if (erracc <= erracctmp) {
                    yy0 = y0p1;
                    y0p1++;
                }
                xx0 += xdir;
                uint32_t wgt = (erracc >> intshift) & 255;
                pixelWeight({ xx0, yy0 }, options, 255 - wgt);
                pixelWeight({ xx0, y0p1 }, options, wgt);
            }
        }

        if (draw_endpoint) {
            pixel({ end.x, end.y }, options);
        }
    }

    void pixelWeight(Point position, DrawOptions options, int weight)
    {
        int ax = options.color.a;
        ax = ((ax * weight) >> 8);
        if (ax > 255) {
            options.color.a = 255;
        } else {
            options.color.a = (uint8_t)(ax & 0x000000ff);
        }
        // options.color.a = weight;
        pixel(position, options);
    }

    // For circle
    void drawOctants(int xc, int yc, int x, int y, DrawOptions options = {})
    {
        pixel({ xc + x, yc + y }, options);
        pixel({ xc - x, yc + y }, options);
        pixel({ xc + x, yc - y }, options);
        pixel({ xc - x, yc - y }, options);
        pixel({ xc + y, yc + x }, options);
        pixel({ xc - y, yc + x }, options);
        pixel({ xc + y, yc - x }, options);
        pixel({ xc - y, yc - x }, options);
    }

    void drawOctants(int xc, int yc, int x, int y, uint8_t angle, DrawOptions options = {})
    {
        if (angle == 0)
            pixel({ xc + x, yc - y }, options);
        if (angle == 1)
            pixel({ xc + y, yc - x }, options);
        if (angle == 2)
            pixel({ xc + y, yc + x }, options);
        if (angle == 3)
            pixel({ xc + x, yc + y }, options);
        if (angle == 4)
            pixel({ xc - x, yc + y }, options);
        if (angle == 5)
            pixel({ xc - y, yc + x }, options);
        if (angle == 6)
            pixel({ xc - y, yc - x }, options);
        if (angle == 7)
            pixel({ xc - x, yc - y }, options);
    }

    // For filled circle
    void drawFilledOctants(int xc, int yc, int x, int y, DrawOptions options = {})
    {
        for (int xx = xc - x; xx <= xc + x; xx++)
            pixel({ xx, yc + y }, options);
        for (int xx = xc - x; xx <= xc + x; xx++)
            pixel({ xx, yc - y }, options);
        for (int xx = xc - y; xx <= xc + y; xx++)
            pixel({ xx, yc + x }, options);
        for (int xx = xc - y; xx <= xc + y; xx++)
            pixel({ xx, yc - x }, options);
    }

    void drawChar(Point position, unsigned char character, uint8_t* font, float scale = 1.0, DrawOptions options = {})
    {
        float x = position.x;
        float y = position.y;
        uint16_t height = font[0];
        uint16_t width = font[1];

        uint8_t mod = width / 8;
        float x0 = x;
        uint16_t len = (mod * height);
        uint16_t temp = ((character - 32) * len) + 2;
        for (uint16_t i = 0; i < len; i++) {
            uint8_t ch = font[temp];
            for (uint8_t j = 0; j < 8; j++) {
                if (ch & 0x80) {
                    pixel({ (int)x, (int)y }, options);
                }
                ch <<= 1;
                x += scale;
                if ((x - x0) == height * scale) {
                    x = x0;
                    y += scale;
                    break;
                }
            }
            temp++;
        }
    }

    Color* getStyleColor(char* color)
    {
        if (strcmp(color, "background") == 0) {
            return &styles.colors.background;
        }

        if (strcmp(color, "white") == 0) {
            return &styles.colors.white;
        }

        if (strcmp(color, "primary") == 0) {
            return &styles.colors.primary;
        }

        if (strcmp(color, "seconday") == 0) {
            return &styles.colors.secondary;
        }

        if (strcmp(color, "tertiary") == 0) {
            return &styles.colors.tertiary;
        }

        if (strcmp(color, "text") == 0) {
            return &styles.colors.text;
        }

        return NULL;
    }

public:
    void test()
    {
        // line({ 0, 0 }, { 127, 64 });
        // line({ 0, 64 }, { 127, 0 });
        // rect({ 10, 10 }, { 30, 30 }, 5);
        // filledPie({ 100, 32 }, 10, 4, 6);
        circle({ 100, 32 }, 10);

        // drawChar({ 10, 10 }, 'A', ArialBold);
        // drawChar({ 10, 10 }, 'B', UbuntuBold);
        drawChar({ 10, 10 }, 'B', UbuntuBold, 0.5);

        filledRect({ 58, 10 }, { 20, 20 });
        drawChar({ 60, 10 }, 'B', Sinclair_S, 2.0, { .color = { 255, 0, 0 } });

        filledRect({ 40, 45 }, { 30, 20 });
        textRight({ 120, 50 }, "Hello World!", 16, { .color = { 0, 255, 0 }, .fontPath = "Sinclair_S" });

        render();
    }

    Draw(Styles& styles)
        : DrawInterface(styles)
        , st7789([&](uint8_t cmd, uint8_t* data, uint32_t len) { spi.sendCmd(cmd, data, len); }, ST7789_ROWS, ST7789_COLS)
    {
    }

    ~Draw()
    {
    }

    void quit()
    {
    }

    void init() override
    {
        initGpio();
        setGpioMode(GPIO_TFT_DATA_CONTROL, 0x01); // Data/Control pin to output (0x01)
        spi.init();

#ifdef USE_SPI_DEV_MEM
        setGpioMode(GPIO_TFT_RESET_PIN, 1);
        setGpio(GPIO_TFT_RESET_PIN, 1);
        usleep(120 * 1000);
        setGpio(GPIO_TFT_RESET_PIN, 0);
        usleep(120 * 1000);
        setGpio(GPIO_TFT_RESET_PIN, 1);
        usleep(120 * 1000);
#endif

// Do the initialization with a veradiusY low SPI bus speed, so that it will succeed even if the bus speed chosen by the user is too high.
#ifdef USE_SPI_DEV_MEM
        spi.setSpeed(34);
#else
        spi.setSpeed(20000);
#endif

        st7789.init();
        usleep(10 * 1000); // Delay a bit before restoring CLK, or otherwise this has been observed to cause the display not init if done back to back after the clear operation above.

#ifdef USE_SPI_DEV_MEM
        spi.setSpeed(20);
#else
        spi.setSpeed(16000000); // 16 MHz
#endif
        clear();
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

    uint16_t colorToU16(Color color)
    {
        // uint16_t rgb = (color.r << 16) | (color.g << 8) | color.b;
        // uint16_t rgb = (color.b << 16) | (color.g << 8) | color.r;
        // uint16_t rgb = ((color.r & 0xF8) << 8) | ((color.g & 0xFC) << 3) | (color.b >> 3);
        // uint16_t rgb = ((color.b & 0xF8) << 8) | ((color.g & 0xFC) << 3) | (color.r >> 3); // good
        // uint16_t rgb = ((color.r >> 3) << 11) | ((color.g >> 2) << 5) | (color.b >> 3);

        uint16_t rgb = ((color.b >> 3) << 11) | ((color.g >> 2) << 5) | (color.r >> 3); // good
        return htons(rgb);
    }

    bool fullRendering = false;
    void fullRender()
    {
        uint16_t pixels[ST7789_COLS];
        for (int i = 0; i < ST7789_ROWS; i++) {
            for (int j = 0; j < ST7789_COLS; j++) {
                Color color = screenBuffer[i][j];
                pixels[j] = colorToU16(color);
            }
            st7789.drawRow(0, i, ST7789_COLS, pixels);
        }
        fullRendering = false;
    }

    void render() override
    {
        if (fullRendering) {
            fullRender();
            return;
        }

        uint16_t pixels[ST7789_COLS];
        for (int i = 0; i < ST7789_ROWS; i++) {
            // To not make uneccessaradiusY calls to the display
            // only send the row of pixels that have changed
            bool changed = false;
            for (int j = 0; j < ST7789_COLS; j++) {
                Color color = screenBuffer[i][j];
                uint16_t rgbU16 = colorToU16(color);
                pixels[j] = rgbU16;
                if (cacheBuffer[i][j] != rgbU16) {
                    changed = true;
                }
                cacheBuffer[i][j] = rgbU16;
            }
            if (changed) {
                st7789.drawRow(0, i, ST7789_COLS, pixels);
            }
        }
    }

    void clear() override
    {
        // Init buffer with background color
        for (int i = 0; i < ST7789_ROWS; i++) {
            for (int j = 0; j < ST7789_COLS; j++) {
                screenBuffer[i][j] = styles.colors.background;
                cacheBuffer[i][j] = colorToU16(styles.colors.background);
            }
        }
        fullRendering = true;
    }

    int text(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) override
    {
        uint8_t* font = getFont(options.fontPath);
        uint16_t height = font[0];
        uint16_t width = font[1];
        float scale = size / (float)height;
        uint16_t len = text.length();

        float x = position.x;
        float xInc = width * scale;
        for (uint16_t i = 0; i < len; i++) {
            if ((x + xInc) > styles.screen.w) {
                break;
            }
            drawChar({ (int)x, position.y }, text[i], font, scale, { .color = { options.color } });
            x += xInc;
        }
        return x;
    }

    int textRight(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) override
    {
        uint8_t* font = getFont(options.fontPath);
        uint16_t height = font[0];
        uint16_t width = font[1];
        float scale = size / (float)height;
        uint16_t len = text.length();

        float x = position.x;
        float xInc = width * scale;
        for (uint16_t i = 0; i < len; i++) {
            x -= xInc;
            if (x < 0) {
                break;
            }
            drawChar({ (int)x, position.y }, text[len - i - 1], font, scale, { .color = { options.color } });
        }
        return x;
    }

    int textCentered(Point position, std::string text, uint32_t size, DrawTextOptions options = {}) override
    {
        uint8_t* font = getFont(options.fontPath);
        uint16_t height = font[0];
        uint16_t width = font[1];
        float scale = size / (float)height;
        uint16_t len = text.length();

        float x = position.x - (len * width * scale) / 2;
        float xInc = width * scale;
        for (uint16_t i = 0; i < len; i++) {
            if ((x + xInc) > styles.screen.w) {
                break;
            }
            if (x > 0) {
                drawChar({ (int)x, position.y }, text[i], font, scale, { .color = { options.color } });
            }
            x += xInc;
        }
        return 0;
    }

    void filledRect(Point position, Size size, DrawOptions options = {}) override
    {
        for (int y = position.y; y < position.y + size.h; y++) {
            lineHorizontal({ position.x, y }, { position.x + size.w, y }, options);
        }
    }

    void rect(Point position, Size size, DrawOptions options = {}) override
    {
        Point a = position;
        Point b = { position.x + size.w, position.y };
        Point c = { position.x + size.w, position.y + size.h };
        Point d = { position.x, position.y + size.h };
        lineHorizontal(a, b, options);
        lineVertical(b, c, options);
        lineHorizontal(c, d, options);
        lineVertical(d, a, options);
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
        arc({ position.x + radius, position.y + radius }, radius, -2, 0, options);
        lineVertical({ position.x, position.y + radius }, { position.x, position.y + size.h - radius }, options);
        arc({ position.x + size.w - radius, position.y + radius }, radius, 0, 2, options);
        lineHorizontal({ position.x + radius, position.y }, { position.x + size.w - radius, position.y }, options);
        arc({ position.x + radius, position.y + size.h - radius }, radius, 4, 6, options);
        lineHorizontal({ position.x + radius, position.y + size.h }, { position.x + size.w - radius, position.y + size.h }, options);
        arc({ position.x + size.w - radius, position.y + size.h - radius }, radius, 2, 4, options);
        lineVertical({ position.x + size.w, position.y + size.h - radius }, { position.x + size.w, position.y + radius }, options);
    }

    // int aaFilledPieRGBA(float cx, float cy, float rx, float ry,	float start, float end, Uint32 chord, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
    // aaFilledPieRGBA(position.x, position.y, radius, radius, startAngle, endAngle, 0, color.r, color.g, color.b, color.a);
    void filledPie(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {}) override
    {
        int nverts, i, result;

        // Sanity check radii
        if ((radius <= 0) || (startAngle == endAngle))
            return;

        // Convert degrees to radians
        startAngle = fmod(startAngle, 360.0) * 2.0 * M_PI / 360.0;
        endAngle = fmod(endAngle, 360.0) * 2.0 * M_PI / 360.0;
        while (startAngle >= endAngle)
            endAngle += 2.0 * M_PI;

        // Calculate number of vertices on perimeter
        nverts = (endAngle - startAngle) * sqrt(radius * radius) / M_PI;
        if (nverts < 2)
            nverts = 2;
        if (nverts > 180)
            nverts = 180;

        std::vector<Point> points;

        // // Allocate combined vertex array
        // vx = vy = (double*)malloc(2 * sizeof(double) * (nverts + 1));
        // if (vx == NULL)
        //     return;

        // // Update pointer to start of vy
        // vy += nverts + 1;

        // Calculate vertices:
        for (i = 0; i < nverts; i++) {
            double angle = startAngle + (endAngle - startAngle) * (double)i / (double)(nverts - 1);
            // vx[i] = position.x + radius * cos(angle);
            // vy[i] = position.y + radius * sin(angle);
            points.push_back({ (int)(position.x + radius * cos(angle)), (int)(position.y + radius * sin(angle)) });
        }

        // // Center:
        // vx[i] = position.x;
        // vy[i] = position.y;
        points.push_back({ position.x, position.y });

        // aaFilledPolygonRGBA(renderer, vx, vy, nverts + 1 - (0 != 0), r, g, b, a);
        filledPolygon(points, options);

        // free(vx);
    }

    /**
     * Draw arc when circle is divided into 8 octants. Each angle correspond to one octant.
     * Meaning that angle 0 will correspond to octant 0, angle 45 will correspond to octant 1, etc.
     * Therefor starting angle is 0 and ending angle is 8.
     * Negative angles will result in anti clockwise rotation. Meaning that angle -1 will correspond to octant 7, etc.
     */
    void arc(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {}) override
    {
        for (int angle = startAngle; angle < endAngle; angle++) {
            // for (int angle = 0; angle < 8; angle++) {
            int x = 0, y = radius;
            int d = 3 - 2 * radius;
            drawOctants(position.x, position.y, x, y, (angle + 8) % 8);
            while (y >= x) {
                if (d > 0) {
                    y--;
                    d = d + 4 * (x - y) + 10;
                } else {
                    d = d + 4 * x + 6;
                }
                x++;
                drawOctants(position.x, position.y, x, y, (angle + 8) % 8);
            }
        }
    }

    void circle(Point position, int radius, DrawOptions options = {}) override
    {
        int x = 0, y = radius;
        int d = 3 - 2 * radius;
        drawOctants(position.x, position.y, x, y);
        while (y >= x) {
            if (d > 0) {
                y--;
                d = d + 4 * (x - y) + 10;
            } else {
                d = d + 4 * x + 6;
            }
            x++;
            drawOctants(position.x, position.y, x, y);
        }
    }

    void filledCircle(Point position, int radius, DrawOptions options = {}) override
    {
        int x = 0, y = radius;
        int d = 3 - 2 * radius;
        drawFilledOctants(position.x, position.y, x, y);
        while (y >= x) {
            if (d > 0) {
                y--;
                d = d + 4 * (x - y) + 10;
            } else {
                d = d + 4 * x + 6;
            }
            x++;
            drawFilledOctants(position.x, position.y, x, y);
        }
    }

    void line(Point start, Point end, DrawOptions options = {}) override
    {
        if (start.x == end.x) {
            lineVertical(start, end, options);
        } else if (start.y == end.y) {
            lineHorizontal(start, end, options);
        } else {
            lineDiagonal(start, end, options);
        }
    }

    void lines(std::vector<Point> points, DrawOptions options = {}) override
    {
        for (int i = 0; i < points.size() - 1; i++) {
            line(points[i], points[i + 1], options);
        }
    }

    void filledPolygon(std::vector<Point> points, DrawOptions options = {}) override
    {
        int i, j, xi, yi, result;
        double x1, x2, y0, y1, y2, minx, maxx, prec;
        float *list, *strip;

        if (points.size() < 3)
            return;

        int n = points.size();
        // Find extrema:
        minx = 99999.0;
        maxx = -99999.0;
        prec = 0.00001;
        for (i = 0; i < n; i++) {
            double x = points[i].x;
            double y = fabs(points[i].y);
            if (x < minx)
                minx = x;
            if (x > maxx)
                maxx = x;
            if (y > prec)
                prec = y;
        }
        minx = floor(minx);
        maxx = floor(maxx);
        prec = floor(pow(2, 19) / prec);

#define POLYSIZE 16384

        list = (float*)malloc(POLYSIZE * sizeof(float));
        if (list == NULL)
            return;

        // Build vertex list.  Special x-values used to indicate vertex type:
        // x = -100001.0 indicates /\, x = -100003.0 indicates \/, x = -100002.0 neither
        yi = 0;
        y0 = floor(points[n - 1].y * prec) / prec;
        y1 = floor(points[0].y * prec) / prec;
        for (i = 1; i <= n; i++) {
            if (yi > POLYSIZE - 4) {
                free(list);
                return;
            }
            y2 = floor(points[i % n].y * prec) / prec;
            if (((y1 < y2) - (y1 > y2)) == ((y0 < y1) - (y0 > y1))) {
                list[yi++] = -100002.0;
                list[yi++] = y1;
                list[yi++] = -100002.0;
                list[yi++] = y1;
            } else {
                if (y0 != y1) {
                    list[yi++] = (y1 < y0) - (y1 > y0) - 100002.0;
                    list[yi++] = y1;
                }
                if (y1 != y2) {
                    list[yi++] = (y1 < y2) - (y1 > y2) - 100002.0;
                    list[yi++] = y1;
                }
            }
            y0 = y1;
            y1 = y2;
        }
        xi = yi;

        // Sort vertex list:
        qsort(list, yi / 2, sizeof(float) * 2, Draw::gfxPrimitivesCompareFloat2);

        // Append line list to vertex list:
        for (i = 1; i <= n; i++) {
            double x, y;
            double d = 0.5 / prec;

            x1 = points[i - 1].x;
            y1 = floor(points[i - 1].y * prec) / prec;
            x2 = points[i % n].x;
            y2 = floor(points[i % n].y * prec) / prec;

            if (y2 < y1) {
                double tmp;
                tmp = x1;
                x1 = x2;
                x2 = tmp;
                tmp = y1;
                y1 = y2;
                y2 = tmp;
            }
            if (y2 != y1)
                y0 = (x2 - x1) / (y2 - y1);

            for (j = 1; j < xi; j += 4) {
                y = list[j];
                if (((y + d) <= y1) || (y == list[j + 4]))
                    continue;
                if ((y -= d) >= y2)
                    break;
                if (yi > POLYSIZE - 4) {
                    free(list);
                    return;
                }
                if (y > y1) {
                    list[yi++] = x1 + y0 * (y - y1);
                    list[yi++] = y;
                }
                y += d * 2.0;
                if (y < y2) {
                    list[yi++] = x1 + y0 * (y - y1);
                    list[yi++] = y;
                }
            }

            y = floor(y1) + 1.0;
            while (y <= y2) {
                x = x1 + y0 * (y - y1);
                if (yi > POLYSIZE - 2) {
                    free(list);
                    return;
                }
                list[yi++] = x;
                list[yi++] = y;
                y += 1.0;
            }
        }

        // Sort combined list:
        qsort(list, yi / 2, sizeof(float) * 2, Draw::gfxPrimitivesCompareFloat2);

        // Plot lines:
        strip = (float*)malloc((maxx - minx + 2) * sizeof(float));
        if (strip == NULL) {
            free(list);
            return;
        }
        memset(strip, 0, (maxx - minx + 2) * sizeof(float));
        n = yi;
        yi = list[1];
        j = 0;

        for (i = 0; i < n - 7; i += 4) {
            float x1 = list[i + 0];
            float y1 = list[i + 1];
            float x3 = list[i + 2];
            float x2 = list[i + j + 0];
            float y2 = list[i + j + 1];
            float x4 = list[i + j + 2];

            if (x1 + x3 == -200002.0)
                j += 4;
            else if (x1 + x3 == -200006.0)
                j -= 4;
            else if ((x1 >= minx) && (x2 >= minx)) {
                if (x1 > x2) {
                    float tmp = x1;
                    x1 = x2;
                    x2 = tmp;
                }
                if (x3 > x4) {
                    float tmp = x3;
                    x3 = x4;
                    x4 = tmp;
                }

                for (xi = x1 - minx; xi <= x4 - minx; xi++) {
                    float u, v;
                    float x = minx + xi;
                    if (x < x2)
                        u = (x - x1 + 1) / (x2 - x1 + 1);
                    else
                        u = 1.0;
                    if (x >= x3 - 1)
                        v = (x4 - x) / (x4 - x3 + 1);
                    else
                        v = 1.0;
                    if ((u > 0.0) && (v > 0.0))
                        strip[xi] += (y2 - y1) * (u + v - 1.0);
                }
            }

            if ((yi == (list[i + 5] - 1.0)) || (i == n - 8)) {
                for (xi = 0; xi <= maxx - minx; xi++) {
                    if (strip[xi] != 0.0) {
                        if (strip[xi] >= 0.996) {
                            int x0 = xi;
                            while (strip[++xi] >= 0.996)
                                ;
                            xi--;
                            line({ (int)(minx + x0), yi }, { (int)(minx + xi), yi }, options);
                        } else {
                            pixel({ (int)(minx + xi), yi }, { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * strip[xi]) });
                        }
                    }
                }
                memset(strip, 0, (maxx - minx + 2) * sizeof(float));
                yi++;
            }
        }

        // Free arrays:
        free(list);
        free(strip);
    }

    static int gfxPrimitivesCompareFloat2(const void* a, const void* b)
    {
        float diff = *((float*)a + 1) - *((float*)b + 1);
        if (diff != 0.0)
            return (diff > 0) - (diff < 0);
        diff = *(float*)a - *(float*)b;
        return (diff > 0) - (diff < 0);
    }

    void filledEllipse(Point position, int radiusX, int radiusY, DrawOptions options = {}) override
    {
        if ((radiusX <= 0) || (radiusY <= 0))
            return;

        int xi, yi;
        double s, v, x, y, dx, dy;

        if (radiusX >= radiusY) {
            int n = radiusY + 1;
            for (yi = position.y - n - 1; yi <= position.y + n + 1; yi++) {
                if (yi < (position.y - 0.5))
                    y = yi;
                else
                    y = yi + 1;
                s = (y - position.y) / radiusY;
                s = s * s;
                x = 0.5;
                if (s < 1.0) {
                    x = radiusX * sqrt(1.0 - s);
                    if (x >= 0.5) {
                        line({ (int)(position.x - x + 1), yi }, { (int)(position.x + x - 1), yi }, options);
                    }
                }
                s = 8 * radiusY * radiusY;
                dy = fabs(y - position.y) - 1.0;
                xi = position.x - x; // left
                while (1) {
                    dx = (position.x - xi - 1) * radiusY / radiusX;
                    v = s - 4 * (dx - dy) * (dx - dy);
                    if (v < 0)
                        break;
                    v = (sqrt(v) - 2 * (dx + dy)) / 4;
                    if (v < 0)
                        break;
                    if (v > 1.0)
                        v = 1.0;
                    pixel({ xi, yi }, { { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v) } });
                    xi -= 1;
                }
                xi = position.x + x; // right
                while (1) {
                    dx = (xi - position.x) * radiusY / radiusX;
                    v = s - 4 * (dx - dy) * (dx - dy);
                    if (v < 0)
                        break;
                    v = (sqrt(v) - 2 * (dx + dy)) / 4;
                    if (v < 0)
                        break;
                    if (v > 1.0)
                        v = 1.0;
                    pixel({ xi, yi }, { { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v) } });
                    xi += 1;
                }
            }
        } else {
            int n = radiusX + 1;
            for (xi = position.x - n - 1; xi <= position.x + n + 1; xi++) {
                if (xi < (position.x - 0.5))
                    x = xi;
                else
                    x = xi + 1;
                s = (x - position.x) / radiusX;
                s = s * s;
                y = 0.5;
                if (s < 1.0) {
                    y = radiusY * sqrt(1.0 - s);
                    if (y >= 0.5) {
                        line({ xi, (int)(position.y - y + 1) }, { xi, (int)(position.y + y - 1) }, options);
                    }
                }
                s = 8 * radiusX * radiusX;
                dx = fabs(x - position.x) - 1.0;
                yi = position.y - y; // top
                while (1) {
                    dy = (position.y - yi - 1) * radiusX / radiusY;
                    v = s - 4 * (dy - dx) * (dy - dx);
                    if (v < 0)
                        break;
                    v = (sqrt(v) - 2 * (dy + dx)) / 4;
                    if (v < 0)
                        break;
                    if (v > 1.0)
                        v = 1.0;
                    pixel({ xi, yi }, { { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v) } });
                    yi -= 1;
                }
                yi = position.y + y; // bottom
                while (1) {
                    dy = (yi - position.y) * radiusX / radiusY;
                    v = s - 4 * (dy - dx) * (dy - dx);
                    if (v < 0)
                        break;
                    v = (sqrt(v) - 2 * (dy + dx)) / 4;
                    if (v < 0)
                        break;
                    if (v > 1.0)
                        v = 1.0;
                    pixel({ xi, yi }, { { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v) } });
                    yi += 1;
                }
            }
        }
    }

    void pixel(Point position, DrawOptions options = {}) override
    {
        if (position.x < 0 || position.x >= styles.screen.w || position.y < 0 || position.y >= styles.screen.h) {
            return;
        }
        if (options.color.a != 255) {
            screenBuffer[position.y][position.x] = applyAlphaColor(screenBuffer[position.y][position.x], options.color);
        } else {
            screenBuffer[position.y][position.x] = options.color;
        }
    }

    Color applyAlphaColor(Color currentColor, Color newColor)
    {
        Color resultColor;

        // Normalize the alpha value of newColor to [0, 1]
        float alphaNew = newColor.a / 255.0f;
        float alphaCurrent = currentColor.a / 255.0f;

        // Apply the alpha blending formula to each component
        resultColor.r = (uint8_t)((alphaNew * newColor.r) + ((1 - alphaNew) * currentColor.r));
        resultColor.g = (uint8_t)((alphaNew * newColor.g) + ((1 - alphaNew) * currentColor.g));
        resultColor.b = (uint8_t)((alphaNew * newColor.b) + ((1 - alphaNew) * currentColor.b));

        // For alpha, you can just take the alpha of the new color,
        // since this is the final transparenposition.y level.
        resultColor.a = newColor.a;

        return resultColor;
    }

    Color getColor(char* color) override
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

    bool config(char* key, char* value) override
    {
        return false;
    }
};

#endif