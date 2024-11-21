#ifndef _LIB_ST7789_H_
#define _LIB_ST7789_H_

#include <functional>

#include <stdint.h>
#include <time.h>
#include <unistd.h>

#define DISPLAY_SET_CURSOR_X 0x2A
#define DISPLAY_SET_CURSOR_Y 0x2B
#define DISPLAY_WRITE_PIXELS 0x2C

#define BYTESPERPIXEL 2

class ST7789 {
protected:
    uint16_t width;
    uint16_t height;

    // uint8_t colstart = 0x23; // try 0x24 too
    // uint8_t rowstart = 0x00;

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
        uint8_t addr[4] = { (uint8_t)(addr0 >> 8), (uint8_t)(addr0 & 0xFF), (uint8_t)(addr1 >> 8), (uint8_t)(addr1 & 0xFF) };
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
        uint8_t data[BYTESPERPIXEL] = { color >> 8, color & 0xFF };

        sendAddr(DISPLAY_SET_CURSOR_X, (uint16_t)x, (uint16_t)x);
        sendAddr(DISPLAY_SET_CURSOR_Y, (uint16_t)y, (uint16_t)y);
        sendCmd(DISPLAY_WRITE_PIXELS, data, 2);
    }

    void drawFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
    {
        int yPos;
        uint16_t size = w * BYTESPERPIXEL;
        uint8_t pixels[size];
        uint8_t pixel[BYTESPERPIXEL] = { color >> 8, color & 0xFF };

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
        uint8_t x[4] = { 0, 0, (width-1) >> 8, (width-1) & 0xFF };
        printf("x: %x %x %x %x\n", x[0], x[1], x[2], x[3]);
        sendCmd(0x2A, x, 4); // Set Column Address
        // uint8_t y[4] = { 0, 0, 0x01, 0x3f }; // ystart = 0, yend = 320
        uint8_t y[4] = { 0, 0, (height-1) >> 8, (height-1) & 0xFF };
        printf("y: %x %x %x %x\n", y[0], y[1], y[2], y[3]);
        sendCmd(0x2B, y, 4); // Set Row Address
        sendCmdOnly(0x21); // Display Inversion
        usleep(10 * 1000);
        sendCmdOnly(0x13); // Normal Display Mode
        usleep(10 * 1000);
        sendCmdOnly(0x29); // Display On
        usleep(500 * 1000);

        // drawFillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0); // clear screen
        srand(time(NULL));
        uint16_t randomColor = rand() % 0xFFFFFF;
        drawFillRect(0, 0, width, height, randomColor); // clear screen

        for (int i = 0; i < 100; i++) {
            drawPixel(i, i*2, 0xFFFF00);
            drawPixel(rand() % width, rand() % height, 0xFFFFFF);
        }
    }
};

#endif