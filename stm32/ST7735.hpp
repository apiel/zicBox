#pragma once

#include "draw/drawPrimitives.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "stm32h7xx_hal.h"
#include <algorithm> // For std::swap
#include <cstdint>
#include <cstring> // For memset
#include <string_view>

class ST7735 : public DrawPrimitives {
protected:
    // Max size of a st7735 is 132x162
    static constexpr uint16_t ST7735_BUFFER = 132 * 162 + 1;
    uint16_t cacheBuffer[ST7735_BUFFER];

public:
    uint16_t buffer[ST7735_BUFFER];

protected:
    // Registers
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

    // LANDSCAPE OFFSETS: These usually flip for landscape
    // Tweak these if your image is shifted!
    uint8_t colStart = 1;
    uint8_t rowStart = 26;

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

    // Updated to send 2 bytes at once (Required for ST7735 16-bit args)
    void writeData16(uint16_t data)
    {
        uint8_t bytes[2] = { uint8_t(data >> 8), uint8_t(data & 0xFF) };
        dcHigh();
        csLow();
        HAL_SPI_Transmit(hspi, bytes, 2, 100);
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
    enum class Orientation {
        PORTRAIT,
        LANDSCAPE,
        PORTRAIT_REV,
        LANDSCAPE_REV
    } orientation
        = Orientation::PORTRAIT;

    ST7735(SPI_HandleTypeDef* spi, uint16_t width, uint16_t height, uint16_t csPin, uint16_t dcPin, uint16_t backlightPin)
        : DrawPrimitives(PoppinsLight_12)
        , hspi(spi)
        , width(width)
        , height(height)
        , csPin(csPin)
        , dcPin(dcPin)
        , backlightPin(backlightPin)
    {
        if (width > height) {
            orientation = Orientation::LANDSCAPE;
        }
    }

    void init()
    {
        writeCommand(SWRESET);
        HAL_Delay(150);
        writeCommand(SLPOUT);
        HAL_Delay(200);

        // Frame rate & Power
        writeCommand(FRMCTR1);
        writeData(0x01);
        writeData(0x2C);
        writeData(0x2D);
        writeCommand(PWCTR1);
        writeData(0xA2);
        writeData(0x02);
        writeData(0x84);
        writeCommand(VMCTR1);
        writeData(0x0E);

        writeCommand(MADCTL);
        setRotation(orientation);

        writeCommand(COLMOD);
        writeData(0x05); // 16-bit color

        writeCommand(INVON); // Most small displays need inversion ON
        writeCommand(NORON);
        HAL_Delay(10);
        writeCommand(DISPON);
        HAL_Delay(100);

        // Clear screen
        for (int32_t i = 0; i < ST7735_BUFFER; i++) {
            buffer[i] = 0x0000; // Black
            cacheBuffer[i] = 0xFFFF; // White, just to force the first render
        }
        render();
    }

    void setRotation(Orientation m)
    {
        writeCommand(MADCTL);

        switch (m) {
        case Orientation::PORTRAIT: {
            writeData(0x08); // BGR bit always on for correct colors
            if (width > height) {
                std::swap(width, height);
            }
            colStart = 26;
            rowStart = 1;
            break;
        }

        case Orientation::LANDSCAPE: {
            writeData(0xA8);
            if (width < height) {
                std::swap(width, height);
            }
            colStart = 1;
            rowStart = 26;
            break;
        }

        case Orientation::PORTRAIT_REV: {
            writeData(0xC8);
            if (width > height) {
                std::swap(width, height);
            }
            colStart = 26;
            rowStart = 1;
            break;
        }

        case Orientation::LANDSCAPE_REV: {
            writeData(0x68);
            if (width < height) {
                std::swap(width, height);
            }
            colStart = 1;
            rowStart = 26;
            break;
        }
        }
    }

    int text(Point position, std::string_view text, uint32_t size, DrawTextOptions options = {})
    {
        return DrawPrimitives::text(position, text, size, width, options);
    }

    void pixel(Point position, Color color) override
    {
        if (position.x < 0 || position.x >= width || position.y < 0 || position.y >= height) return;

        uint16_t rawColor = rgb565(color.r, color.g, color.b);
        // ST7735 SPI expects High Byte first (Big Endian)
        buffer[position.y * width + position.x] = (rawColor << 8) | (rawColor >> 8);
    }

    uint16_t getPixel(int16_t x, int16_t y)
    {
        if (x < 0 || x >= width || y < 0 || y >= height) return 0;
        uint16_t c = buffer[y * width + x];
        return (c << 8) | (c >> 8); // Swap back for user logic
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
                dcHigh();
                csLow();
                // Send entire row span at once (much faster on H7)
                HAL_SPI_Transmit(hspi, (uint8_t*)&buffer[y * width + xStart], spanLen * 2, 100);
                csHigh();
            }
        }
    }

    static constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
    {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }

    void backlightOn() { HAL_GPIO_WritePin(GPIOE, backlightPin, GPIO_PIN_RESET); }
    void backlightOff() { HAL_GPIO_WritePin(GPIOE, backlightPin, GPIO_PIN_SET); }
};