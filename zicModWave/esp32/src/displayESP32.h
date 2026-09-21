#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "../../displayView.h"
#include "../../zicApp.h"
#include "draw/drawTFT_eSPI.h"

inline void initDisplayESP32(TFT_eSPI& tft)
{
    tft.init();
    tft.setRotation(1); // Landscape mode 320x172
    tft.fillScreen(TFT_BLACK);
}

inline void renderDisplayESP32(TFT_eSPI& tft, DisplayView& displayView, ZicApp& app)
{
    DrawTFT_eSPI drawer(tft);
    displayView.render(drawer, app);
}
