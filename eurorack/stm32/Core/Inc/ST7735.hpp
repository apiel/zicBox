/**
 * @file ST7735.hpp
 * @brief CORRECTED ST7735 driver for WeAct STM32H723
 * 
 * CORRECT Pin Mapping (from schematic):
 *   PE14: SPI4_MOSI (LCD_SDA)
 *   PE12: SPI4_SCK  (LCD_SCL)
 *   PE11: GPIO      (LCD_CS)
 *   PE13: GPIO      (LCD_WR_RS / DC)
 *   PE10: TIM1_CH2N (LCD_LED backlight via PWM)
 *   
 * RESET is shared with system reset (not controllable)
 */

#pragma once

#include "main.h"
#include <cstdint>

class ST7735 {
public:
    static constexpr uint16_t WIDTH = 80;
    static constexpr uint16_t HEIGHT = 160;
    
    static constexpr uint16_t BLACK   = 0x0000;
    static constexpr uint16_t WHITE   = 0xFFFF;
    static constexpr uint16_t RED     = 0xF800;
    static constexpr uint16_t GREEN   = 0x07E0;
    static constexpr uint16_t BLUE    = 0x001F;
    static constexpr uint16_t CYAN    = 0x07FF;
    static constexpr uint16_t MAGENTA = 0xF81F;
    static constexpr uint16_t YELLOW  = 0xFFE0;
    
private:
    // Commands
    static constexpr uint8_t SWRESET = 0x01;
    static constexpr uint8_t SLPOUT  = 0x11;
    static constexpr uint8_t NORON   = 0x13;
    static constexpr uint8_t INVOFF  = 0x20;
    static constexpr uint8_t INVON   = 0x21;
    static constexpr uint8_t DISPON  = 0x29;
    static constexpr uint8_t CASET   = 0x2A;
    static constexpr uint8_t RASET   = 0x2B;
    static constexpr uint8_t RAMWR   = 0x2C;
    static constexpr uint8_t MADCTL  = 0x36;
    static constexpr uint8_t COLMOD  = 0x3A;
    static constexpr uint8_t FRMCTR1 = 0xB1;
    static constexpr uint8_t PWCTR1  = 0xC0;
    static constexpr uint8_t VMCTR1  = 0xC5;
    
    SPI_HandleTypeDef* hspi;
    TIM_HandleTypeDef* htim;  // For backlight PWM
    uint16_t width = WIDTH;
    uint16_t height = HEIGHT;
    uint8_t colStart = 26;
    uint8_t rowStart = 1;
    
    // Pin control - CORRECTED!
    inline void csLow()   { HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET); }
    inline void csHigh()  { HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET); }
    inline void dcLow()   { HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_RESET); }
    inline void dcHigh()  { HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_SET); }
    
    // NOTE: Reset is shared with system reset, so we can't control it
    // Backlight is PWM on PE10 via TIM1_CH2N
    
    void writeCommand(uint8_t cmd) {
        dcLow();
        csLow();
        HAL_SPI_Transmit(hspi, &cmd, 1, 100);
        csHigh();
    }
    
    void writeData(uint8_t data) {
        dcHigh();
        csLow();
        HAL_SPI_Transmit(hspi, &data, 1, 100);
        csHigh();
    }
    
    void writeData16(uint16_t data) {
        uint8_t buffer[2] = {uint8_t(data >> 8), uint8_t(data & 0xFF)};
        dcHigh();
        csLow();
        HAL_SPI_Transmit(hspi, buffer, 2, 100);
        csHigh();
    }
    
    void setWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
        writeCommand(CASET);
        writeData16(x0 + colStart);
        writeData16(x1 + colStart);
        
        writeCommand(RASET);
        writeData16(y0 + rowStart);
        writeData16(y1 + rowStart);
        
        writeCommand(RAMWR);
    }
    
public:
    ST7735(SPI_HandleTypeDef* spi, TIM_HandleTypeDef* tim) 
        : hspi(spi), htim(tim) {}
    
    void init() {
        // Software reset (no hardware reset available)
        writeCommand(SWRESET);
        HAL_Delay(150);
        
        // Out of sleep
        writeCommand(SLPOUT);
        HAL_Delay(500);
        
        // Frame rate
        writeCommand(FRMCTR1);
        writeData(0x01); writeData(0x2C); writeData(0x2D);
        
        // Power control
        writeCommand(PWCTR1);
        writeData(0xA2); writeData(0x02); writeData(0x84);
        
        // VCOM
        writeCommand(VMCTR1);
        writeData(0x0E);
        
        // Display inversion off
        // writeCommand(INVOFF);
        writeCommand(INVON);
        
        // Memory access control
        writeCommand(MADCTL);
        writeData(0xC8);  // MX, MY, RGB
        
        // Color mode: RGB565
        writeCommand(COLMOD);
        writeData(0x05);
        
        // Normal display
        writeCommand(NORON);
        HAL_Delay(10);
        
        // Display on
        writeCommand(DISPON);
        HAL_Delay(100);
        
        // Turn on backlight via PWM
        // backlightOn();
        
        // Clear screen
        fillScreen(BLACK);
    }
    
    // void backlightOn() {
    //     // Start PWM on TIM1 Channel 2N
    //     // Note: OCNPolarity is set to LOW in IOC, so PWM is inverted
    //     HAL_TIM_PWM_Start(htim, TIM_CHANNEL_2);
    // }
    
    // void backlightOff() {
    //     HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_2);
    // }
    
    // void setBacklight(uint8_t brightness) {
    //     // brightness: 0-100
    //     // TIM1 Period is 1000-1 from IOC
    //     uint32_t pulse = (brightness * 1000) / 100;
    //     __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, pulse);
    // }
    
    void fillScreen(uint16_t color) {
        fillRect(0, 0, width, height, color);
    }
    
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
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
    
    void drawPixel(int16_t x, int16_t y, uint16_t color) {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        setWindow(x, y, x, y);
        writeData16(color);
    }
    
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
        int16_t dx = abs(x1 - x0);
        int16_t dy = abs(y1 - y0);
        int16_t sx = x0 < x1 ? 1 : -1;
        int16_t sy = y0 < y1 ? 1 : -1;
        int16_t err = dx - dy;
        
        while (true) {
            drawPixel(x0, y0, color);
            if (x0 == x1 && y0 == y1) break;
            
            int16_t e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }
    }
    
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
        int16_t f = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x = 0;
        int16_t y = r;
        
        drawPixel(x0, y0 + r, color);
        drawPixel(x0, y0 - r, color);
        drawPixel(x0 + r, y0, color);
        drawPixel(x0 - r, y0, color);
        
        while (x < y) {
            if (f >= 0) {
                y--;
                ddF_y += 2;
                f += ddF_y;
            }
            x++;
            ddF_x += 2;
            f += ddF_x;
            
            drawPixel(x0 + x, y0 + y, color);
            drawPixel(x0 - x, y0 + y, color);
            drawPixel(x0 + x, y0 - y, color);
            drawPixel(x0 - x, y0 - y, color);
            drawPixel(x0 + y, y0 + x, color);
            drawPixel(x0 - y, y0 + x, color);
            drawPixel(x0 + y, y0 - x, color);
            drawPixel(x0 - y, y0 - x, color);
        }
    }
    
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
        drawLine(x0 - r, y0, x0 + r, y0, color);
        
        int16_t f = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x = 0;
        int16_t y = r;
        
        while (x < y) {
            if (f >= 0) {
                y--;
                ddF_y += 2;
                f += ddF_y;
            }
            x++;
            ddF_x += 2;
            f += ddF_x;
            
            drawLine(x0 - x, y0 + y, x0 + x, y0 + y, color);
            drawLine(x0 - x, y0 - y, x0 + x, y0 - y, color);
            drawLine(x0 - y, y0 + x, x0 + y, y0 + x, color);
            drawLine(x0 - y, y0 - x, x0 + y, y0 - x, color);
        }
    }
    
    static constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
};