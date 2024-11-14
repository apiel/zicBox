#ifndef _UI_DRAW_H_
#define _UI_DRAW_H_

#include "../../fonts/Sinclair_M.h"
#include "../../fonts/Sinclair_S.h"
#include "../../fonts/Tiny.h"
#include "../../helpers/i2c.h"
#include "../../log.h"
#include "../../plugins/components/drawInterface.h"
#include <cmath>
#include <stdexcept>
#include <string>

#define SSD1306_I2C_ADDR 0x3c

#define SSD1306_COMM_CONTROL_BYTE 0x00
#define SSD1306_DATA_CONTROL_BYTE 0x40

#define SSD1306_COMM_PAGE_NUMBER 0xb0
#define SSD1306_COMM_LOW_COLUMN 0x00
#define SSD1306_COMM_HIGH_COLUMN 0x10

#define SSD1306_128_64_LINES 64
#define SSD1306_128_32_LINES 32
#define SSD1306_64_48_LINES 48

#define SSD1306_128_64_COLUMNS 128
#define SSD1306_128_32_COLUMNS 128
#define SSD1306_64_48_COLUMNS 64

// Pixel color
#define SSD1306_BLACK 0
#define SSD1306_WHITE 1
#define SSD1306_INVERSE 2

// Use max height and width
#define SSD1306_BUFFER_SIZE (128 * 64 / 8)

class Draw : public DrawInterface {
public:
    uint8_t oledBuffer[SSD1306_BUFFER_SIZE] = { 0 };

protected:
    I2c i2c;

    void oledInit(uint8_t i2c_dev = 1)
    {
        if (!i2c.init(i2c_dev, SSD1306_I2C_ADDR)) {
            throw std::runtime_error("Could not initialize SSD1306 display.");
        }

        // test i2c connection
        uint8_t cmd = SSD1306_COMM_CONTROL_BYTE;
        uint8_t result = 0;
        i2c.send(&cmd, 1);
        i2c.pull(&result, 1);
        if (result == 0) {
            throw std::runtime_error("Connection to SSD1306 display failed.");
        }
    }

    void validateDisplaySize()
    {
        if (styles.screen.w != SSD1306_128_64_COLUMNS
            && styles.screen.w != SSD1306_128_32_COLUMNS
            && styles.screen.w != SSD1306_64_48_COLUMNS) {
            throw std::runtime_error("Unsupported display width " + std::to_string(styles.screen.w));
        }
        if (styles.screen.h != SSD1306_128_64_LINES
            && styles.screen.h != SSD1306_128_32_LINES
            && styles.screen.h != SSD1306_64_48_LINES) {
            throw std::runtime_error("Unsupported display height " + std::to_string(styles.screen.h));
        }
    }

    void oledConfig()
    {
        uint8_t data[] = {
            SSD1306_COMM_CONTROL_BYTE, // command control byte
            0xae, // display off
            0xa6, // Set Normal Display (default)
            0xd5, // SETDISPLAYCLOCKDIV
            0x80, // the suggested ratio 0x80
            0xa8, // SSD1306_SETMULTIPLEX
            (uint8_t)(styles.screen.h - 1), // height is 32 or 64 (always -1)
            0xd3, // SETDISPLAYOFFSET
            0, // no offset
            0x40, // SETSTARTLINE
            0x8d, // CHARGEPUMP
            0x14, // turn on charge pump
            0x20, // MEMORYMODE
            0x02, // page mode
            0xa0, // SEGREMAP  Mirror screen horizontally (A0)
            0xc0, // COMSCANDEC Rotate screen vertically (C0)
            0xda, // HARDWARE PIN
            styles.screen.h == SSD1306_128_32_LINES ? (uint8_t)0x02 : (uint8_t)0x12, // 0x02 for 32 lines, 0x12 for 64 lines or 48 lines
            0x81, // SETCONTRAST
            0x7f, // default contract value
            0xd9, // SETPRECHARGE
            0xf1, // default precharge value
            0xdb, // SETVCOMDETECT
            0x40, // default deselect value
            0xa4, // DISPLAYALLON_RESUME
            0xa6, // Set Normal Display (default)
            0xaf, // DISPLAY ON
            0x2e, // Stop scroll
        };

        i2c.send(data, sizeof(data) / sizeof(data[0]));
    }

    uint8_t oledGetPageCount()
    {
        return styles.screen.h / 8;
    }

    void oledPixel(uint8_t x, uint8_t y)
    {
        uint16_t tc = (styles.screen.w * (y / 8)) + x;
        oledBuffer[tc] |= (1 << (y & 7));
    }

    void oledPixel(uint8_t x, uint8_t y, uint8_t color)
    {
        uint16_t tc = (styles.screen.w * (y / 8)) + x;
        switch (color) {
        case SSD1306_WHITE:
            oledBuffer[tc] |= (1 << (y & 7));
            break;
        case SSD1306_BLACK:
            oledBuffer[tc] &= ~(1 << (y & 7));
            break;
        case SSD1306_INVERSE:
            oledBuffer[tc] ^= (1 << (y & 7));
            break;
        }
    }

    void oledRender()
    {
        uint8_t pages = oledGetPageCount();

        for (uint8_t page = 0; page < pages; page++) {
            oledRender(page);
        }
    }

    // The GDDRAM is a bit mapped static RAM holding the bit pattern to be displayed. The size of the RAM is
    // 128 x 64 bits and the RAM is divided into eight pages, from PAGE0 to PAGE7
    void oledRender(int16_t page)
    {
        oledPosition(0, page);
        uint8_t data[1 + styles.screen.w];
        data[0] = SSD1306_DATA_CONTROL_BYTE;
        uint16_t offset = styles.screen.w * page;
        for (uint16_t i = 0; i < styles.screen.w; i++) {
            data[i + 1] = oledBuffer[i + offset];
        }
        i2c.send(data, 1 + styles.screen.w);
    }

    void oledPosition(uint8_t x, uint8_t page)
    {
        uint8_t data[4];
        data[0] = SSD1306_COMM_CONTROL_BYTE;
        data[1] = SSD1306_COMM_PAGE_NUMBER | (page & 0x0f);
        data[2] = SSD1306_COMM_LOW_COLUMN | (x & 0x0f);
        data[3] = SSD1306_COMM_HIGH_COLUMN | ((x >> 4) & 0x0f);
        i2c.send(data, 4);
    }

    void oledClearPage(uint8_t page)
    {
        uint16_t offset = styles.screen.w * page;
        for (uint16_t i = 0; i < styles.screen.w; i++) {
            oledBuffer[i + offset] = 0x00;
        }
        oledRender(page);
    }

    void lineVertical(Point start, Point end, DrawOptions options = {})
    {
        int ystep = start.y < end.y ? 1 : -1;
        for (int y = start.y; y != end.y; y += ystep) {
            oledPixel(start.x, y);
        }
    }

    void lineHorizontal(Point start, Point end, DrawOptions options = {})
    {
        int xstep = start.x < end.x ? 1 : -1;
        for (int x = start.x; x != end.x; x += xstep) {
            oledPixel(x, start.y);
        }
    }

    void lineDiagonal(Point start, Point end, DrawOptions options = {})
    {
        int x0, y0, x1, y1;
        if (start.x < end.x) {
            x0 = start.x;
            x1 = end.x;
            y0 = start.y;
            y1 = end.y;
        } else {
            x0 = end.x;
            x1 = start.x;
            y0 = end.y;
            y1 = start.y;
        }
        int deltaX = x1 - x0;
        int deltaY = abs(y1 - y0);
        int eps = 0;
        int ystep = y0 < y1 ? 1 : -1;

        int y = y0;
        for (int x = x0; x <= x1; x++) {
            oledPixel(x, y);
            eps += deltaY;
            if ((eps << 1) >= deltaX) {
                y += ystep;
                eps -= deltaX;
            }
        }
    }

    // For circle
    void drawOctants(int xc, int yc, int x, int y)
    {
        oledPixel(xc + x, yc + y);
        oledPixel(xc - x, yc + y);
        oledPixel(xc + x, yc - y);
        oledPixel(xc - x, yc - y);
        oledPixel(xc + y, yc + x);
        oledPixel(xc - y, yc + x);
        oledPixel(xc + y, yc - x);
        oledPixel(xc - y, yc - x);
    }

    void drawOctants(int xc, int yc, int x, int y, uint8_t angle)
    {
        if (angle == 0)
            oledPixel(xc + x, yc - y);
        if (angle == 1)
            oledPixel(xc + y, yc - x);
        if (angle == 2)
            oledPixel(xc + y, yc + x);
        if (angle == 3)
            oledPixel(xc + x, yc + y);
        if (angle == 4)
            oledPixel(xc - x, yc + y);
        if (angle == 5)
            oledPixel(xc - y, yc + x);
        if (angle == 6)
            oledPixel(xc - y, yc - x);
        if (angle == 7)
            oledPixel(xc - x, yc - y);
    }

    // For filled circle
    void drawFilledOctants(int xc, int yc, int x, int y)
    {
        for (int xx = xc - x; xx <= xc + x; xx++)
            oledPixel(xx, yc + y);
        for (int xx = xc - x; xx <= xc + x; xx++)
            oledPixel(xx, yc - y);
        for (int xx = xc - y; xx <= xc + y; xx++)
            oledPixel(xx, yc + x);
        for (int xx = xc - y; xx <= xc + y; xx++)
            oledPixel(xx, yc - x);
    }

    // void drawFilledOctants(int xc, int yc, int x, int y, uint8_t angle)
    // {
    //     if (angle == 0)
    //         line({xc + x, yc - y}, {xc, yc});
    //     if (angle == 1)
    //         line({xc + y, yc - x}, {xc, yc});
    //     if (angle == 2)
    //         line({xc + y, yc + x}, {xc, yc});
    //     if (angle == 3)
    //         line({xc + x, yc + y}, {xc, yc});
    //     if (angle == 4)
    //         line({xc - x, yc + y}, {xc, yc});
    //     if (angle == 5)
    //         line({xc - y, yc + x}, {xc, yc});
    //     if (angle == 6)
    //         line({xc - y, yc - x}, {xc, yc});
    //     if (angle == 7)
    //         line({xc - x, yc - y}, {xc, yc});
    // }

    void drawChar(Point position, unsigned char character, uint8_t* font, float scale = 1.0, uint8_t color = SSD1306_WHITE)
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
                    oledPixel(x, y, color);
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

        // drawChar(10, 10, 'A', Sinclair_S, 3.0);
        // drawChar(10, 10, 'A', TinyFont);
        // drawChar(10, 10, 'A', Sinclair_S);
        drawChar({ 10, 10 }, 'B', Sinclair_M);

        filledRect({ 58, 10 }, { 20, 20 });
        drawChar({ 60, 10 }, 'B', Sinclair_S, 2.0, SSD1306_BLACK);

        drawChar({ 90, 5 }, 'B', Sinclair_S, 2.0);

        render();
    }

    Draw(Styles& styles)
        : DrawInterface(styles)
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
        validateDisplaySize();
        oledInit();
        oledConfig();
        clear();

        test();
    }

    void renderNext() override
    {
    }

    void triggerRendering() override
    {
    }

    void render() override
    {
        oledRender();
    }

    void clear()
    {
        // TODO uncomment, commented the time of testing...
        // uint8_t pages = oledGetPageCount();
        // for (uint8_t page = 0; page < pages; page++) {
        //     oledClearPage(page);
        // }
    }

    void filledRect(Point position, Size size, DrawOptions options = {})
    {
        for (int y = position.y; y < position.y + size.h; y++) {
            lineHorizontal({ position.x, y }, { position.x + size.w, y }, options);
        }
    }

    void rect(Point position, Size size, DrawOptions options = {})
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
    void arc(Point position, int radius, int startAngle, int endAngle, DrawOptions options = {})
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

    void circle(Point position, int radius, DrawOptions options = {})
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

    void filledCircle(Point position, int radius, DrawOptions options = {})
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

    void line(Point start, Point end, DrawOptions options = {})
    {
        if (start.x == end.x) {
            lineVertical(start, end, options);
        } else if (start.y == end.y) {
            lineHorizontal(start, end, options);
        } else {
            lineDiagonal(start, end, options);
        }
    }

    void lines(std::vector<Point> points, DrawOptions options = {})
    {
        for (int i = 0; i < points.size() - 1; i++) {
            line(points[i], points[i + 1], options);
        }
    }

    void pixel(Point position, DrawOptions options = {})
    {
        oledPixel(position.x, position.y);
    }

    bool config(char* key, char* value)
    {
        return false;
    }
};

#endif