#pragma once

#include "stm32h7xx_hal.h"

#include <cstdint>

class ST7735 {
protected:
    // Max size of a st7735 is 132x162,
    // So we take the higher value to set the buffer size
    static constexpr uint16_t ST7735_BUFFER = 132 * 162 + 1; // Let's add +1 to be sure :p
    uint16_t cacheBuffer[ST7735_BUFFER];

public:
    uint16_t buffer[ST7735_BUFFER];

protected:
    static constexpr uint8_t SWRESET = 0x01;
    static constexpr uint8_t SLPOUT = 0x11;
    static constexpr uint8_t NORON = 0x13;
    static constexpr uint8_t INVOFF = 0x20;
    static constexpr uint8_t INVON = 0x21;
    static constexpr uint8_t DISPON = 0x29;
    static constexpr uint8_t CASET = 0x2A;
    static constexpr uint8_t RASET = 0x2B;
    static constexpr uint8_t RAMWR = 0x2C;
    static constexpr uint8_t MADCTL = 0x36;
    static constexpr uint8_t COLMOD = 0x3A;
    static constexpr uint8_t FRMCTR1 = 0xB1;
    static constexpr uint8_t PWCTR1 = 0xC0;
    static constexpr uint8_t VMCTR1 = 0xC5;

    SPI_HandleTypeDef* hspi;
    uint16_t width;
    uint16_t height;
    uint8_t colStart = 26;
    uint8_t rowStart = 1;

    uint16_t csPin;
    uint16_t dcPin;
    uint16_t backlightPin;

    inline void csLow() { HAL_GPIO_WritePin(GPIOE, csPin, GPIO_PIN_RESET); }
    inline void csHigh() { HAL_GPIO_WritePin(GPIOE, csPin, GPIO_PIN_SET); }
    inline void dcLow() { HAL_GPIO_WritePin(GPIOE, dcPin, GPIO_PIN_RESET); }
    inline void dcHigh() { HAL_GPIO_WritePin(GPIOE, dcPin, GPIO_PIN_SET); }

    void writeCommand(uint8_t cmd)
    {
        dcLow();
        csLow();
        HAL_SPI_Transmit(hspi, &cmd, 1, 100);
        csHigh();
    }

    void writeData(uint8_t data)
    {
        dcHigh();
        csLow();
        HAL_SPI_Transmit(hspi, &data, 1, 100);
        csHigh();
    }

    void writeData16(uint16_t data)
    {
        uint8_t buffer[2] = { uint8_t(data >> 8), uint8_t(data & 0xFF) };
        dcHigh();
        csLow();
        HAL_SPI_Transmit(hspi, buffer, 2, 100);
        csHigh();
    }

    void setWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
    {
        writeCommand(CASET);
        writeData16(x0 + colStart);
        writeData16(x1 + colStart);

        writeCommand(RASET);
        writeData16(y0 + rowStart);
        writeData16(y1 + rowStart);

        writeCommand(RAMWR);
    }

public:
    ST7735(SPI_HandleTypeDef* spi, uint16_t width, uint16_t height, uint16_t csPin, uint16_t dcPin, uint16_t backlightPin)
        : hspi(spi)
        , width(width)
        , height(height)
        , csPin(csPin)
        , dcPin(dcPin)
        , backlightPin(backlightPin)
    {
    }

    void init()
    {
        // Software reset (no hardware reset available)
        writeCommand(SWRESET);
        HAL_Delay(150);

        // Out of sleep
        writeCommand(SLPOUT);
        HAL_Delay(500);

        // Frame rate
        writeCommand(FRMCTR1);
        writeData(0x01);
        writeData(0x2C);
        writeData(0x2D);

        // Power control
        writeCommand(PWCTR1);
        writeData(0xA2);
        writeData(0x02);
        writeData(0x84);

        // VCOM
        writeCommand(VMCTR1);
        writeData(0x0E);

        // Display inversion off
        // writeCommand(INVOFF);
        writeCommand(INVON);

        // Memory access control
        writeCommand(MADCTL);
        writeData(0xC8); // MX, MY, RGB

        // Color mode: RGB565
        writeCommand(COLMOD);
        writeData(0x05);

        // Normal display
        writeCommand(NORON);
        HAL_Delay(10);

        // Display on
        writeCommand(DISPON);
        HAL_Delay(100);

        // Clear screen
        uint16_t black = rgb565(0, 0, 0);
        for (int32_t i = 0; i < ST7735_BUFFER; i++) {
            buffer[i] = black;
            cacheBuffer[i] = 255; // just to force the first render
        }
        render();
    }

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
    {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        if (x + w > width) w = width - x;
        if (y + h > height) h = height - y;
        for (int32_t i = 0; i < h; i++) {
            for (int32_t j = 0; j < w; j++) {
                buffer[(y + i) * width + x + j] = color;
            }
        }
    }

    void setPixel(int16_t x, int16_t y, uint16_t color)
    {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        buffer[y * width + x] = color;
    }

    uint16_t getPixel(int16_t x, int16_t y)
    {
        if (x < 0 || x >= width || y < 0 || y >= height) return 0;
        return buffer[y * width + x];
    }

    void render()
    {
        for (uint16_t y = 0; y < height; y++) {
            int16_t xStart = -1;
            int16_t xEnd = -1;

            for (uint16_t x = 0; x < width; x++) {
                uint32_t idx = y * width + x;
                if (buffer[idx] != cacheBuffer[idx]) {
                    if (xStart == -1) xStart = x;
                    xEnd = x;
                    cacheBuffer[idx] = buffer[idx];
                }
            }

            if (xStart != -1) {
                setWindow(xStart, y, xEnd, y);

                uint16_t spanLen = (xEnd - xStart) + 1;
                uint16_t* dataPtr = &buffer[y * width + xStart];

                dcHigh();
                csLow();
                for (uint16_t i = 0; i < spanLen; i++) {
                    uint16_t color = dataPtr[i];
                    uint8_t bytes[2] = { static_cast<uint8_t>(color >> 8), static_cast<uint8_t>(color & 0xFF) };
                    HAL_SPI_Transmit(hspi, bytes, 2, 10);
                }
                csHigh();
            }
        }
    }

    static constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
    {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }

    // Display is on when gpio is low (default value set in .ioc)
    void backlightOn() { HAL_GPIO_WritePin(GPIOE, backlightPin, GPIO_PIN_RESET); } // display on when gpio is low
    void backlightOff() { HAL_GPIO_WritePin(GPIOE, backlightPin, GPIO_PIN_SET); } // display off when gpio is high
};
