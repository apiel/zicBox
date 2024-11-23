#ifndef _UI_DRAW_H_
#define _UI_DRAW_H_

#include "../../helpers/gpio.h"
#include "../../helpers/st7789.h"

// #define USE_SPI_DEV_MEM
#ifdef USE_SPI_DEV_MEM
// sudo apt-get install libraspberrypi-dev raspberrypi-kernel-headers
// sudo chown 0:0 test2
// sudo chmod u+s test2
// see:
// https://raspberrypi.stackexchange.com/questions/40105/access-gpio-pins-without-root-no-access-to-dev-mem-try-running-as-root
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
    Color cacheBuffer[ST7789_ROWS][ST7789_COLS];

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
        drawChar({ 60, 10 }, 'B', Sinclair_S, 2.0, { .color = { 255, 0 , 0 } });

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

// Do the initialization with a very low SPI bus speed, so that it will succeed even if the bus speed chosen by the user is too high.
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

    // uint16_t BLACK = 0x0000,
    // uint16_t RED = 0xF800,
    // uint16_t GREEN = 0x07E0,
    // uint16_t BLUE = 0x001F,
    // uint16_t WHITE = 0xFFFF
    // uint16_t getColor(uint8_t r, uint8_t g, uint8_t b) { return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3); }
    

    void render() override
    {
        uint16_t pixels[ST7789_COLS];
        for (int i = 0; i < ST7789_ROWS; i++) {
            for (int j = 0; j < ST7789_COLS; j++) {
                Color color = screenBuffer[i][j];
                // uint16_t rgb = (color.r << 16) | (color.g << 8) | color.b;
                // uint16_t rgb = (color.b << 16) | (color.g << 8) | color.r;
                // uint16_t rgb = ((color.r & 0xF8) << 8) | ((color.g & 0xFC) << 3) | (color.b >> 3);
                // uint16_t rgb = ((color.b & 0xF8) << 8) | ((color.g & 0xFC) << 3) | (color.r >> 3); // good
                // uint16_t rgb = ((color.r >> 3) << 11) | ((color.g >> 2) << 5) | (color.b >> 3);
                uint16_t rgb = ((color.b >> 3) << 11) | ((color.g >> 2) << 5) | (color.r >> 3); // good
                pixels[j] = htons(rgb);
                cacheBuffer[i][j] = color;
            }
            st7789.drawRow(0, i, ST7789_COLS, pixels);
        }
    }

    void clear() override
    {
        // Init buffer with background color
        for (int i = 0; i < ST7789_ROWS; i++) {
            for (int j = 0; j < ST7789_COLS; j++) {
                screenBuffer[i][j] = styles.colors.background;
                cacheBuffer[i][j] = styles.colors.background;
            }
        }
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

    // /**
    //  * Draw filledPie when circle is divided into 8 octants. Each angle correspond to one octant.
    //  * Meaning that angle 0 will correspond to octant 0, angle 45 will correspond to octant 1, etc.
    //  * Therefor starting angle is 0 and ending angle is 8.
    //  * Negative angles will result in anti clockwise rotation. Meaning that angle -1 will correspond to octant 7, etc.
    //  */
    // void filledPie(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {})
    // {
    //     for (int angle = startAngle; angle < endAngle; angle++) {
    //         // for (int angle = 0; angle < 8; angle++) {
    //         int x = 0, y = radius;
    //         int d = 3 - 2 * radius;
    //         drawFilledOctants(position.x, position.y, x, y, (angle + 8) % 8);
    //         while (y >= x) {
    //             if (d > 0) {
    //                 y--;
    //                 d = d + 4 * (x - y) + 10;
    //             } else {
    //                 d = d + 4 * x + 6;
    //             }
    //             x++;
    //             drawFilledOctants(position.x, position.y, x, y, (angle + 8) % 8);
    //         }
    //     }
    // }

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

    void pixel(Point position, DrawOptions options = {}) override
    {
        if (position.x < 0 || position.x >= styles.screen.w || position.y < 0 || position.y >= styles.screen.h) {
            return;
        }
        screenBuffer[position.y][position.x] = options.color;
    }

    bool config(char* key, char* value) override
    {
        return false;
    }
};

#endif