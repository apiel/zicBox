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
        uint8_t madctl = 0;
        uint8_t data[4] = { 0, 0, (uint8_t)(240 >> 8), (uint8_t)(240 & 0xFF) };

        sendCmdOnly(0x11 /*Sleep Out*/);
        usleep(120 * 1000);
        sendCmdData(0x3A /*COLMOD: Pixel Format Set*/, 0x05 /*16bpp*/);
        usleep(20 * 1000);

#define MADCTL_COLUMN_ADDRESS_ORDER_SWAP (1 << 6)
#define MADCTL_ROW_ADDRESS_ORDER_SWAP (1 << 7)
#define MADCTL_ROTATE_180_DEGREES (MADCTL_COLUMN_ADDRESS_ORDER_SWAP | MADCTL_ROW_ADDRESS_ORDER_SWAP)
/* RGB/BGR Order ('0' = RGB, '1' = BGR) */
#define ST7789_MADCTL_RGB 0x00
#define ST7789_MADCTL_BGR 0x08

        madctl |= MADCTL_ROW_ADDRESS_ORDER_SWAP;
        madctl |= ST7789_MADCTL_BGR; // Because we swap order, we need to use BGR...
        madctl ^= MADCTL_ROTATE_180_DEGREES;
        sendCmdData(0x36 /*MADCTL: Memory Access Control*/, madctl);
        usleep(10 * 1000);

        sendCmdData(0xBA /*DGMEN: Enable Gamma*/, 0x04);

        sendCmdOnly(0x21 /*Display Inversion On*/);
        // sendCmd(0x20 /*Display Inversion Off*/);

        sendCmdOnly(0x13 /*NORON: Partial off (normal)*/);
        usleep(10 * 1000);
        // The ST7789 controller is actually a unit with 320x240 graphics memory area, but only 240x240 portion
        // of it is displayed. Therefore if we wanted to swap row address mode above, writes to Y=0...239 range will actually land in
        // memory in row addresses Y = 319-(0...239) = 319...80 range. To view this range, we must scroll the view by +80 units in Y
        // direction so that contents of Y=80...319 is displayed instead of Y=0...239.
        if ((madctl & MADCTL_ROW_ADDRESS_ORDER_SWAP)) {
            sendCmd(0x37 /*VSCSAD: Vertical Scroll Start Address of RAM*/, data, 4);
        }

        // drawFillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0); // clear screen
        srand(time(NULL));
        uint16_t randomColor = rand() % 0xFFFFFF;
        drawFillRect(0, 0, width, height, randomColor); // clear screen

        for (int i = 0; i < 100; i++) {
            drawPixel(i, i, 0xFFFF00);
            drawPixel(rand() % width, rand() % height, 0xFFFFFF);
        }

        sendCmdOnly(/*Display ON*/ 0x29);
    }
};

#endif