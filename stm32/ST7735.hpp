#pragma once

#include "stm32h7xx_hal.h"

#include <cstdint>

class ST7735 {
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
        fillScreen(rgb565(0, 0, 0));
    }

    void fillScreen(uint16_t color)
    {
        fillRect(0, 0, width, height, color);
    }

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
    {
        if (x >= width || y >= height) return;
        if (x + w > width) w = width - x;
        if (y + h > height) h = height - y;

        setWindow(x, y, x + w - 1, y + h - 1);

        uint8_t hi = color >> 8;
        uint8_t lo = color & 0xFF;

        dcHigh();
        csLow();

        for (int32_t i = w * h; i > 0; i--) {
            HAL_SPI_Transmit(hspi, &hi, 1, 100);
            HAL_SPI_Transmit(hspi, &lo, 1, 100);
        }

        csHigh();
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color)
    {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        setWindow(x, y, x, y);
        writeData16(color);
    }

    static constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
    {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }

    // Display is on when gpio is low (default value set in .ioc)
    void backlightOn() { HAL_GPIO_WritePin(GPIOE, backlightPin, GPIO_PIN_RESET); } // display on when gpio is low
    void backlightOff() { HAL_GPIO_WritePin(GPIOE, backlightPin, GPIO_PIN_SET); }  // display off when gpio is high
};