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
        if (options.thickness == 1) {
            lineVertical1px(start, end, options);
        } else {
            int startx = start.x - options.thickness * 0.5;
            int endx = end.x - options.thickness * 0.5;
            for (int i = 0; i < options.thickness; i++) {
                lineVertical1px({ startx + i, start.y }, { endx + i, end.y }, options);
            }
        }
    }

    void lineVertical1px(Point start, Point end, DrawOptions options = {})
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
        if (options.thickness == 1) {
            lineHorizontal1px(start, end, options);
        } else {
            int starty = start.y - options.thickness * 0.5;
            int endy = end.y - options.thickness * 0.5;
            for (int i = 0; i < options.thickness; i++) {
                lineHorizontal1px({ start.x, starty + i }, { end.x, endy + i }, options);
            }
        }
    }

    void lineHorizontal1px(Point start, Point end, DrawOptions options = {})
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
        if (options.thickness == 1) {
            lineDiagonal1px(start, end, options);
        } else {
            int startx = start.x - options.thickness * 0.5;
            int endx = end.x - options.thickness * 0.5;
            for (int i = 0; i < options.thickness; i++) {
                lineDiagonal1px({ startx + i, start.y }, { endx + i, end.y }, options);
            }
        }
    }

    void lineDiagonal1px(Point start, Point end, DrawOptions options = {})
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
    }

    void fastLineDiagonal(Point start, Point end, DrawOptions options = {})
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

        if (strcmp(color, "secondary") == 0) {
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

    void filledPie(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {}) override
    {
        int xi, yi;
        double s, v, x, y, dx, dy;

        // Normalize angles to range [0, 360)
        startAngle = (startAngle % 360 + 360) % 360;
        endAngle = (endAngle % 360 + 360) % 360;

        // Convert angles to radians for comparison
        double startRad = startAngle * M_PI / 180.0;
        double endRad = endAngle * M_PI / 180.0;

        // Ensure angles are in counterclockwise order
        bool wrapAround = (endRad < startRad);

        int n = radius + 1;
        for (yi = position.y - n - 1; yi <= position.y + n + 1; yi++) {
            if (yi < (position.y - 0.5))
                y = yi;
            else
                y = yi + 1;
            s = (y - position.y) / radius;
            s = s * s;
            x = 0.5;
            if (s < 1.0) {
                x = radius * sqrt(1.0 - s);
                if (x >= 0.5) {
                    // Draw only the visible part of the pie within the angle range
                    for (xi = (int)(position.x - x + 1); xi <= (int)(position.x + x - 1); xi++) {
                        // Convert (xi, yi) to polar coordinates (r, theta)
                        double dx = xi - position.x;
                        double dy = yi - position.y;
                        double angle = atan2(dy, dx);
                        if (angle < 0)
                            angle += 2 * M_PI; // Normalize to [0, 2π]

                        // Check if angle is within the start and end angles
                        if ((wrapAround && (angle >= startRad || angle <= endRad)) || (!wrapAround && angle >= startRad && angle <= endRad)) {
                            pixel({ xi, yi }, { options.color });
                        }
                    }
                }
            }
            s = 8 * radius * radius;
            dy = fabs(y - position.y) - 1.0;
            xi = position.x - x; // left
            while (1) {
                dx = (position.x - xi - 1);
                v = s - 4 * (dx - dy) * (dx - dy);
                if (v < 0)
                    break;
                v = (sqrt(v) - 2 * (dx + dy)) / 4;
                if (v < 0)
                    break;
                if (v > 1.0)
                    v = 1.0;

                // Convert (xi, yi) to polar coordinates (r, theta)
                double dx = xi - position.x;
                double dy = yi - position.y;
                double angle = atan2(dy, dx);
                if (angle < 0)
                    angle += 2 * M_PI; // Normalize to [0, 2π]

                // Check if angle is within the start and end angles
                if ((wrapAround && (angle >= startRad || angle <= endRad)) || (!wrapAround && angle >= startRad && angle <= endRad)) {
                    pixel({ xi, yi }, { { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v) } });
                }

                xi -= 1;
            }
            xi = position.x + x; // right
            while (1) {
                dx = (xi - position.x);
                v = s - 4 * (dx - dy) * (dx - dy);
                if (v < 0)
                    break;
                v = (sqrt(v) - 2 * (dx + dy)) / 4;
                if (v < 0)
                    break;
                if (v > 1.0)
                    v = 1.0;

                // Convert (xi, yi) to polar coordinates (r, theta)
                double dx = xi - position.x;
                double dy = yi - position.y;
                double angle = atan2(dy, dx);
                if (angle < 0)
                    angle += 2 * M_PI; // Normalize to [0, 2π]

                // Check if angle is within the start and end angles
                if ((wrapAround && (angle >= startRad || angle <= endRad)) || (!wrapAround && angle >= startRad && angle <= endRad)) {
                    pixel({ xi, yi }, { { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v) } });
                }

                xi += 1;
            }
        }

        // Draw smooth radius lines for the start and end angles
        int r = radius - 1;
        int startX = position.x + (int)(r * cos(startRad));
        int startY = position.y + (int)(r * sin(startRad));
        int endX = position.x + (int)(r * cos(endRad));
        int endY = position.y + (int)(r * sin(endRad));
        line({ startX, startY }, position, { options.color });
        line({ endX, endY }, position, { options.color });
    }

    void arc(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {}) override
    {
        int xi, yi;
        double s, v, x, y, dx, dy;

        // Normalize angles to range [0, 360)
        startAngle = (startAngle % 360 + 360) % 360;
        endAngle = (endAngle % 360 + 360) % 360;

        // Convert angles to radians
        double startRad = startAngle * M_PI / 180.0;
        double endRad = endAngle * M_PI / 180.0;

        // Handle angle wrapping
        bool wrapAround = (endRad < startRad);

        // Calculate inner and outer radii
        double innerRadius = radius - options.thickness / 2.0;
        double outerRadius = radius + options.thickness / 2.0;

        int n = outerRadius + 1;
        for (yi = position.y - n - 1; yi <= position.y + n + 1; yi++) {
            if (yi < (position.y - 0.5))
                y = yi;
            else
                y = yi + 1;
            s = (y - position.y) / outerRadius;
            s = s * s;
            x = 0.5;
            if (s < 1.0) {
                x = outerRadius * sqrt(1.0 - s);
                if (x >= 0.5) {
                    for (xi = (int)(position.x - x + 1); xi <= (int)(position.x + x - 1); xi++) {
                        // Calculate distance to the center
                        double dx = xi - position.x;
                        double dy = yi - position.y;
                        double distance = sqrt(dx * dx + dy * dy);

                        // Determine if within the angle and thickness range
                        if (distance >= innerRadius && distance <= outerRadius) {
                            // Convert (xi, yi) to polar coordinates
                            double angle = atan2(dy, dx);
                            if (angle < 0)
                                angle += 2 * M_PI; // Normalize to [0, 2π]

                            // Check angle constraints
                            if ((wrapAround && (angle >= startRad || angle <= endRad)) || (!wrapAround && angle >= startRad && angle <= endRad)) {
                                // Anti-aliasing based on distance to inner and outer radii
                                double alpha = 1.0;
                                if (distance < innerRadius + 1.0) {
                                    alpha = distance - innerRadius;
                                } else if (distance > outerRadius - 1.0) {
                                    alpha = outerRadius - distance;
                                }
                                if (alpha > 0) {
                                    pixel({ xi, yi }, { { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * alpha) } });
                                }
                            }
                        }
                    }
                }
            }
            s = 8 * outerRadius * outerRadius;
            dy = fabs(y - position.y) - 1.0;
            xi = position.x - x; // left
            while (1) {
                dx = (position.x - xi - 1);
                v = s - 4 * (dx - dy) * (dx - dy);
                if (v < 0)
                    break;
                v = (sqrt(v) - 2 * (dx + dy)) / 4;
                if (v < 0)
                    break;
                if (v > 1.0)
                    v = 1.0;

                // Calculate distance to the center
                double dx = xi - position.x;
                double dy = yi - position.y;
                double distance = sqrt(dx * dx + dy * dy);

                // Determine if within the angle and thickness range
                if (distance >= innerRadius && distance <= outerRadius) {
                    double angle = atan2(dy, dx);
                    if (angle < 0)
                        angle += 2 * M_PI; // Normalize to [0, 2π]

                    // Check angle constraints
                    if ((wrapAround && (angle >= startRad || angle <= endRad)) || (!wrapAround && angle >= startRad && angle <= endRad)) {
                        double alpha = 1.0;
                        if (distance < innerRadius + 1.0) {
                            alpha = distance - innerRadius;
                        } else if (distance > outerRadius - 1.0) {
                            alpha = outerRadius - distance;
                        }
                        if (alpha > 0) {
                            pixel({ xi, yi }, { { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v * alpha) } });
                        }
                    }
                }

                xi -= 1;
            }
            xi = position.x + x; // right
            while (1) {
                dx = (xi - position.x);
                v = s - 4 * (dx - dy) * (dx - dy);
                if (v < 0)
                    break;
                v = (sqrt(v) - 2 * (dx + dy)) / 4;
                if (v < 0)
                    break;
                if (v > 1.0)
                    v = 1.0;

                // Calculate distance to the center
                double dx = xi - position.x;
                double dy = yi - position.y;
                double distance = sqrt(dx * dx + dy * dy);

                // Determine if within the angle and thickness range
                if (distance >= innerRadius && distance <= outerRadius) {
                    double angle = atan2(dy, dx);
                    if (angle < 0)
                        angle += 2 * M_PI; // Normalize to [0, 2π]

                    // Check angle constraints
                    if ((wrapAround && (angle >= startRad || angle <= endRad)) || (!wrapAround && angle >= startRad && angle <= endRad)) {
                        double alpha = 1.0;
                        if (distance < innerRadius + 1.0) {
                            alpha = distance - innerRadius;
                        } else if (distance > outerRadius - 1.0) {
                            alpha = outerRadius - distance;
                        }
                        if (alpha > 0) {
                            pixel({ xi, yi }, { { options.color.r, options.color.g, options.color.b, (uint8_t)(options.color.a * v * alpha) } });
                        }
                    }
                }

                xi += 1;
            }
        }
    }

    void circle(Point position, int radius, DrawOptions options = {}) override
    {
        arc(position, radius, 0, 359, options);
    }

    void filledCircle(Point position, int radius, DrawOptions options = {})
    {
        int xi, yi;
        double s, v, x, y, dx, dy;

        int n = radius + 1;
        for (yi = position.y - n - 1; yi <= position.y + n + 1; yi++) {
            if (yi < (position.y - 0.5))
                y = yi;
            else
                y = yi + 1;
            s = (y - position.y) / radius;
            s = s * s;
            x = 0.5;
            if (s < 1.0) {
                x = radius * sqrt(1.0 - s);
                if (x >= 0.5) {
                    line({ (int)(position.x - x + 1), yi }, { (int)(position.x + x - 1), yi }, options);
                }
            }
            s = 8 * radius * radius;
            dy = fabs(y - position.y) - 1.0;
            xi = position.x - x; // left
            while (1) {
                dx = (position.x - xi - 1);
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
                dx = (xi - position.x);
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
        if (points.size() < 3)
            return; // A polygon must have at least 3 points

        lines(points, { options.color });
        line(points[0], points[points.size() - 1], { options.color });

        // Compute the bounding box of the polygon
        int minY = points[0].y, maxY = points[0].y;
        for (const auto& p : points) {
            minY = std::min(minY, p.y);
            maxY = std::max(maxY, p.y);
        }

        // Use a scan-line approach to fill the polygon
        for (int y = minY; y <= maxY; ++y) {
            std::vector<int> intersections;

            // Find intersections of the polygon with the current scan line
            for (size_t i = 0; i < points.size(); ++i) {
                Point p1 = points[i];
                Point p2 = points[(i + 1) % points.size()]; // Wrap around to the first point

                // Check if the scan line crosses the edge
                if ((p1.y <= y && p2.y > y) || (p2.y <= y && p1.y > y)) {
                    // Compute the x-coordinate of the intersection
                    float t = (y - p1.y) / float(p2.y - p1.y);
                    int x = p1.x + t * (p2.x - p1.x);
                    intersections.push_back(x);
                }
            }

            // Sort the intersections to form spans
            std::sort(intersections.begin(), intersections.end());

            // Fill the spans between pairs of intersections
            for (size_t i = 0; i < intersections.size(); i += 2) {
                if (i + 1 < intersections.size()) {
                    int xStart = intersections[i] + 1;
                    int xEnd = intersections[i + 1];

                    for (int x = xStart; x <= xEnd; ++x) {
                        pixel({ x, y }, options);
                    }
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