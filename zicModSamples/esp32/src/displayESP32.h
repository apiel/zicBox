#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "../../displayView.h"
#include "../../zicApp.h"

class DisplayESP32 {
public:
    TFT_eSPI tft = TFT_eSPI();
    TFT_eSprite sprite = TFT_eSprite(&tft);

    void init()
    {
        // Enable Power Rail for LCD on LilyGO T-Display-S3 (GPIO 15)
        pinMode(15, OUTPUT);
        digitalWrite(15, HIGH);

        pinMode(TFT_BL, OUTPUT);
        digitalWrite(TFT_BL, HIGH); // Enable backlight

        tft.init();
        tft.setRotation(1); // Horizontal landscape (320x170)
        tft.setSwapBytes(true);
        tft.fillScreen(TFT_BLACK);
    }

    void render(Draw& d, DisplayView& displayView, ZicApp& app)
    {
        displayView.render(d, app);

        uint16_t lineBuf[DisplayView::SCREEN_W];
        for (int y = 0; y < DisplayView::SCREEN_H; ++y) {
            for (int x = 0; x < DisplayView::SCREEN_W; ++x) {
                Color c = d.screenBuffer[y][x];
                lineBuf[x] = tft.color565(c.r, c.g, c.b);
            }
            tft.pushImage(0, y, DisplayView::SCREEN_W, 1, lineBuf);
        }
    }
};
