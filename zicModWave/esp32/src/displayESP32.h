#pragma once

#include <Arduino.h>

#include "../../displayView.h"
#include "../../zicApp.h"
#include "Display_ST7789.h"
#include "draw/draw.h"

inline Draw& getDrawer()
{
    static Styles styles = {
        .screen = { 320, 172 },
        .margin = 0,
        .colors = {
            { 15, 18, 24, 255 },
            { 255, 255, 255, 255 },
            { 255, 255, 255, 255 },
            { 0, 220, 255, 255 },
            { 0, 220, 140, 255 },
            { 40, 45, 55, 255 },
            { 22, 26, 34, 255 }
        }
    };
    static Draw drawer(styles);
    return drawer;
}

inline void initDisplayESP32()
{
    LCD_Init();
}

inline void renderDisplayESP32(DisplayView& displayView, ZicApp& app)
{
    Draw& d = getDrawer();
    d.clear();
    displayView.render(d, app);

    static uint16_t lineBuf[DisplayView::NATIVE_W];
    static uint8_t dummyRead[DisplayView::NATIVE_W * 2];

    for (int y = 0; y < DisplayView::NATIVE_H; ++y) {
        for (int x = 0; x < DisplayView::NATIVE_W; ++x) {
            Color c = d.screenBuffer[y][x];
            uint16_t r = (c.r >> 3) & 0x1F;
            uint16_t g = (c.g >> 2) & 0x3F;
            uint16_t b = (c.b >> 3) & 0x1F;
            uint16_t rgb565 = (r << 11) | (g << 5) | b;
            lineBuf[x] = (rgb565 >> 8) | ((rgb565 & 0xFF) << 8); // Swap bytes for ST7789 SPI
        }
        LCD_SetCursor(0, y, DisplayView::NATIVE_W - 1, y);
        LCD_WriteData_nbyte((uint8_t*)lineBuf, dummyRead, DisplayView::NATIVE_W * sizeof(uint16_t));
    }
}
