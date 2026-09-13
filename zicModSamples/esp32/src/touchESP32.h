#pragma once

#include <Arduino.h>
#include <Arduino.h>
#include <Wire.h>
#include <algorithm>
#include "../../displayView.h"
#include "../../zicApp.h"

// CST816S / FT6336 I2C Touch Controller Address
#define TOUCH_I2C_ADD 0x15

class TouchESP32 {
public:
    bool lastTouchState = false;

    void init()
    {
        pinMode(21, OUTPUT);
        digitalWrite(21, LOW);
        delay(10);
        digitalWrite(21, HIGH);
        delay(50);

        Wire.begin(18, 17); // Touch SDA=18, SCL=17
    }

    void update(DisplayView& displayView, ZicApp& app)
    {
        Wire.beginTransmission(TOUCH_I2C_ADD);
        Wire.write(0x02); // Read touch state register
        if (Wire.endTransmission() != 0) return;

        Wire.requestFrom(TOUCH_I2C_ADD, 5);
        if (Wire.available() >= 5) {
            uint8_t touchNum = Wire.read() & 0x0F;
            uint8_t xHigh = Wire.read() & 0x0F;
            uint8_t xLow = Wire.read();
            uint8_t yHigh = Wire.read() & 0x0F;
            uint8_t yLow = Wire.read();

            int rawX = (xHigh << 8) | xLow;
            int rawY = (yHigh << 8) | yLow;

            // Map CST816S sensor orientation to 320x170 landscape (Rotation 1)
            int touchX = std::clamp((DisplayView::SCREEN_W - 1) - rawY, 0, DisplayView::SCREEN_W - 1);
            int touchY = std::clamp(rawX, 0, DisplayView::SCREEN_H - 1);

            if (touchNum > 0) {
                if (!lastTouchState) {
                    lastTouchState = true;
                    displayView.handleTouchDown(touchX, touchY);
                } else {
                    displayView.handleTouchMove(touchX, touchY);
                }
            } else {
                if (lastTouchState) {
                    lastTouchState = false;
                    displayView.handleTouchUp(touchX, touchY, app);
                }
            }
        }
    }
};
