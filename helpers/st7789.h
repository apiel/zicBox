#ifndef _LIB_ST7789_H_
#define _LIB_ST7789_H_

#include <functional>

#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h> // htons

#include "../plugins/components/utils/color.h"

#define DISPLAY_SET_CURSOR_X 0x2A
#define DISPLAY_SET_CURSOR_Y 0x2B

// For 170x320 display ??
// #define DISPLAY_SET_CURSOR_X 0x23 // try 0x24 too
// #define DISPLAY_SET_CURSOR_X 0x24
// #define DISPLAY_SET_CURSOR_Y 0x00

#define DISPLAY_WRITE_PIXELS 0x2C

#define BYTESPERPIXEL 2

#define U16_TO_U8(x) (uint8_t)((x >> 8) & 0xFF), (uint8_t)(x & 0xFF)

class ST7789 {
protected:
    uint16_t width = 320;
    uint16_t height = 240;

    std::function<void(uint8_t, uint8_t*, uint32_t)> sendSpiCmd;

    void sendCmd(uint8_t cmd)
    {
        sendSpiCmd(cmd, NULL, 0);
    }

    void sendCmd(uint8_t cmd, uint8_t data)
    {
        sendSpiCmd(cmd, &data, 1);
    }

    void sendCmd(uint8_t cmd, uint8_t* data, uint32_t len)
    {
        sendSpiCmd(cmd, data, len);
    }

    void sendAddr(uint8_t cmd, uint16_t addr0, uint16_t addr1)
    {
        uint8_t addr[4] = { U16_TO_U8(addr0), U16_TO_U8(addr1) };
        sendCmd(cmd, addr, 4);
    }

    void fillScreen(uint16_t color) // just for testing
    {
        drawFillRect(0, 0, width, height, color);
    }

    void drawFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) // just for testing
    {
        uint16_t pixels[w];
        for (uint16_t i = 0; i < w; ++i) {
            pixels[i] = color;
        }
        for (uint16_t i = 0; i < h; ++i) {
            drawRow(x, y + i, w, pixels);
        }
    }

public:
    // ILI9341 display (1)
    // uint8_t madctl = 0x20;
    // bool displayInverted = false;

    // GMT024 display (2) 320x240 landscape view
    // uint8_t madctl = 0x08 | 0x20 | 0x40; // BGR + MV + MX
    // bool displayInverted = true;

    // 240x320 display portrait view
    uint8_t madctl = 0x08;
    bool displayInverted = true;

    // 240x240 display
    // uint8_t madctl = 0x08; // BGR
    // bool displayInverted = true;

    ST7789(std::function<void(uint8_t, uint8_t*, uint32_t)> sendSpiCmd)
        : sendSpiCmd(sendSpiCmd)
    {
    }

    void drawPixel(uint16_t x, uint16_t y, uint16_t color)
    {
        sendAddr(DISPLAY_SET_CURSOR_X, (uint16_t)x, (uint16_t)x);
        sendAddr(DISPLAY_SET_CURSOR_Y, (uint16_t)y, (uint16_t)y);

        uint8_t pixel[BYTESPERPIXEL] = { U16_TO_U8(color) };
        sendCmd(DISPLAY_WRITE_PIXELS, pixel, 2);
    }

    void drawRow(uint16_t x, uint16_t y, uint16_t w, uint16_t* pixels)
    {
        sendAddr(DISPLAY_SET_CURSOR_X, x, x + w);
        sendAddr(DISPLAY_SET_CURSOR_Y, y, y);
        sendCmd(DISPLAY_WRITE_PIXELS, (uint8_t*)pixels, w * BYTESPERPIXEL);
    }

    void init(uint16_t w, uint16_t h)
    {
        width = w;
        height = h;

        sendCmd(0x01); // Software Reset
        usleep(150 * 1000);
        sendCmd(0x11); // Sleep Out
        usleep(500 * 1000);
        sendCmd(0x3A, 0x55); // Set Color Mode: 16-bit
        usleep(10 * 1000);
        sendCmd(0x36, madctl);
        // uint8_t x[4] = { 0, 0, 0, 0x1a }; // xstart = 0, xend = 170
        uint8_t x[4] = { 0, 0, U16_TO_U8(width) };
        sendCmd(0x2A, x, 4); // Set Column Address
        uint8_t y[4] = { 0, 0, U16_TO_U8(height) }; // uint8_t y[4] = { 0, 0, 0x01, 0x3f }; // ystart = 0, yend = 320
        sendCmd(0x2B, y, 4); // Set Row Address
        sendCmd(displayInverted ? 0x21 /*Display Inversion On*/ : 0x20 /*Display Inversion Off*/);
        usleep(10 * 1000);
        sendCmd(0x13); // Normal Display Mode
        usleep(10 * 1000);
        sendCmd(0x29); // Display On
        usleep(500 * 1000);
    }

    void test()
    {
        fillScreen(colorToU16({ 0x21, 0x25, 0x2b, 255 })); // #21252b

        // drawFillRect(40, 20, 30, 30, colorToU16({ 0xFF, 0x00, 0x00 })); // #FF0000
        // drawFillRect(width - 40, height - 20, 10, 10, colorToU16({ 0x00, 0x00, 0xFF })); // #0000FF

        // for (int i = 0; i < 100; i++) {
        //     drawPixel(i, i * 2, 0xFFF0);
        //     drawPixel(rand() % width, rand() % height, 0xFFFF);
        // }

        drawFillRect(10, 30, 10, 10, colorToU16({ 212, 211, 211 })); //rgb(212, 211, 211)
        drawFillRect(30, 30, 10, 10, colorToU16({ 170, 170, 170 })); //rgb(170, 170, 170)
        drawFillRect(50, 30, 10, 10, colorToU16({ 127, 127, 127 })); //rgb(127, 127, 127)
        drawFillRect(70, 30, 10, 10, colorToU16({ 85, 85, 85 })); //rgb(85, 85, 85)
        drawFillRect(90, 30, 10, 10, colorToU16({ 42, 42, 42 })); //rgb(42, 42, 42)
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
        // return rgb;
    }
};

#endif