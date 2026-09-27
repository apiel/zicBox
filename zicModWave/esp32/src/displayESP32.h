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
            { 0, 0, 0, 255 },       // Black Background
            { 255, 255, 255, 255 },
            { 255, 255, 255, 255 },
            { 100, 150, 200, 255 }, // Blue-Gray
            { 70, 130, 180, 255 },
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

inline void pushDisplayESP32()
{
    Draw& d = getDrawer();

    static constexpr int CHUNK_ROWS = 16;
    static uint16_t chunkBuf[DisplayView::NATIVE_W * CHUNK_ROWS];

    // Set full screen draw window ONCE per frame
    LCD_SetCursor(0, 0, DisplayView::NATIVE_W - 1, DisplayView::NATIVE_H - 1);

    for (int yStart = 0; yStart < DisplayView::NATIVE_H; yStart += CHUNK_ROWS) {
        int rowsInChunk = std::min(CHUNK_ROWS, DisplayView::NATIVE_H - yStart);
        int idx = 0;
        
        for (int y = yStart; y < yStart + rowsInChunk; ++y) {
            Color* srcRow = d.screenBuffer[y];
            for (int x = 0; x < DisplayView::NATIVE_W; ++x) {
                Color c = srcRow[x];
                // Byte 0 (sent 1st over SPI): RRRRRGGG
                uint16_t b0 = (c.r & 0xF8) | (c.g >> 5);
                // Byte 1 (sent 2nd over SPI): GGGBBBBB
                uint16_t b1 = ((c.g & 0x1C) << 3) | (c.b >> 3);
                // On Little-Endian Xtensa: b0 goes to low byte (b0), b1 to high byte (b1 << 8)
                chunkBuf[idx++] = b0 | (b1 << 8);
            }
        }

        LCD_WriteData_nbyte((uint8_t*)chunkBuf, idx * sizeof(uint16_t));
    }
}

inline void renderDisplayESP32(DisplayView& displayView, ZicApp& app)
{
    Draw& d = getDrawer();
    d.clear();
    displayView.render(d, app);
    pushDisplayESP32();
}
