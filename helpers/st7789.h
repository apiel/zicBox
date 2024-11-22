#ifndef _LIB_ST7789_H_
#define _LIB_ST7789_H_

#include <functional>

#include <stdint.h>
#include <time.h>
#include <unistd.h>

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
    uint16_t width;
    uint16_t height;

    std::function<void(uint8_t, uint8_t*, uint32_t)> sendCmd;

    void sendCmdOnly(uint8_t cmd)
    {
        sendCmd(cmd, NULL, 0);
    }

    void sendCmdData(uint8_t cmd, uint8_t data)
    {
        sendCmd(cmd, &data, 1);
    }

    void sendAddr(uint8_t cmd, uint16_t addr0, uint16_t addr1)
    {
        uint8_t addr[4] = { U16_TO_U8(addr0), U16_TO_U8(addr1) };
        sendCmd(cmd, addr, 4);
    }

public:
    ST7789(std::function<void(uint8_t, uint8_t*, uint32_t)> sendCmd, uint16_t width, uint16_t height)
        : sendCmd(sendCmd)
        , width(width)
        , height(height)
    {
    }

    void drawPixel(uint16_t x, uint16_t y, uint16_t color)
    {
        sendAddr(DISPLAY_SET_CURSOR_X, (uint16_t)x, (uint16_t)x);
        sendAddr(DISPLAY_SET_CURSOR_Y, (uint16_t)y, (uint16_t)y);

        uint8_t pixel[BYTESPERPIXEL] = { U16_TO_U8(color) };
        sendCmd(DISPLAY_WRITE_PIXELS, pixel, 2);
    }

    void drawFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
    {
        int yPos;
        uint16_t size = w * BYTESPERPIXEL;
        uint8_t pixels[size];
        uint8_t pixel[BYTESPERPIXEL] = { U16_TO_U8(color) };

        for (uint16_t i = 0; i < size; i += BYTESPERPIXEL) {
            pixels[i] = pixel[0];
            pixels[i + 1] = pixel[1];
        }

        for (yPos = 0; yPos < h; ++yPos) {
            sendAddr(DISPLAY_SET_CURSOR_X, x, x + w);
            sendAddr(DISPLAY_SET_CURSOR_Y, y + yPos, y + yPos);
            sendCmd(DISPLAY_WRITE_PIXELS, pixels, size);
        }
    }

    void fillScreen(uint16_t color)
    {
        uint32_t size = width * height * BYTESPERPIXEL;
        uint8_t pixels[size];
        uint8_t pixel[BYTESPERPIXEL] = { U16_TO_U8(color) };

        for (int i = 0; i < size; i += BYTESPERPIXEL) {
            pixels[i] = pixel[0];
            pixels[i + 1] = pixel[1];
        }

        sendAddr(DISPLAY_SET_CURSOR_X, 0, width);
        sendAddr(DISPLAY_SET_CURSOR_Y, 0, height);
        sendCmd(DISPLAY_WRITE_PIXELS, pixels, size);
    }

    void init()
    {
        sendCmdOnly(0x01); // Software Reset
        usleep(150 * 1000);
        sendCmdOnly(0x11); // Sleep Out
        usleep(500 * 1000);
        sendCmdData(0x3A, 0x55); // Set Color Mode: 16-bit
        usleep(10 * 1000);
        sendCmdData(0x36, 0x08); // Memory Access Control: Row/col addr, bottom-top refresh
        // sendCmdData(0x36, 0x00); // Memory Access Control: RGB
        // uint8_t x[4] = { 0, 0, 0, 0x1a }; // xstart = 0, xend = 170
        uint8_t x[4] = { 0, 0, U16_TO_U8(width) };
        sendCmd(0x2A, x, 4); // Set Column Address
        uint8_t y[4] = { 0, 0, U16_TO_U8(height) }; // uint8_t y[4] = { 0, 0, 0x01, 0x3f }; // ystart = 0, yend = 320
        sendCmd(0x2B, y, 4); // Set Row Address
        sendCmdOnly(0x21); // Display Inversion
        usleep(10 * 1000);
        sendCmdOnly(0x13); // Normal Display Mode
        usleep(10 * 1000);
        sendCmdOnly(0x29); // Display On
        usleep(500 * 1000);
    }

    void test()
    {
        // drawFillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0); // clear screen
        srand(time(NULL));
        uint16_t randomColor = rand() % 0xFFFF;
        fillScreen(randomColor); // clear screen

        for (int i = 0; i < 100; i++) {
            drawPixel(i, i * 2, 0xFFF0);
            drawPixel(rand() % width, rand() % height, 0xFFFF);
        }
    }
};

#endif