#pragma once

#include "helpers/gpio.h"
#include "helpers/st7789.h"
#include "draw/utils/color.h"


#include "helpers/SpiDevSpi.h"
#include "draw.h"

class DrawToST7789 {
public:
    uint16_t cacheBuffer[SCREEN_BUFFER_ROWS][SCREEN_BUFFER_COLS];

protected:
    int8_t resetPin = -1;
    Spi spi;
    ST7789 st7789;
    Draw& draw;

public:
    DrawToST7789(Draw& draw)
        : draw(draw)
        , st7789([&](uint8_t cmd, uint8_t* data, uint32_t len) { spi.sendCmd(cmd, data, len); })
    {
        logInfo("construct DrawToST7789");
    }

    void init()
    {
        logDebug("Initializing ST7789");

        initGpio();
        // TODO should this come from spi.gpioDataControl
        gpioSetMode(spi.getGpioDataControl(), 0x01); // Data/Control pin to output (0x01)
        spi.init();

        if (resetPin != -1) {
            // resetPin = 2;
            logDebug("Resetting ST7789 on pin %d", resetPin);
            gpioSetMode(resetPin, 1);
            gpioWrite(resetPin, 1);
            usleep(120 * 1000);
            gpioWrite(resetPin, 0);
            usleep(120 * 1000);
            gpioWrite(resetPin, 1);
            usleep(120 * 1000);
        }

        spi.setSpeed(20000);

        st7789.init(draw.styles.screen.w, draw.styles.screen.h);
        usleep(10 * 1000); // Delay a bit before restoring CLK, or otherwise this has been observed to cause the display not init if done back to back after the clear operation above.

        spi.setSpeed(16000000); // 16 MHz
        draw.clear();

        logDebug("ST7789 initialized.");
    }

    void render()
    {
        uint16_t pixels[SCREEN_BUFFER_COLS];
        for (int i = 0; i < draw.styles.screen.h; i++) {
            // To not make uneccessaradiusY calls to the display
            // only send the row of pixels that have changed
            bool changed = false;
            for (int j = 0; j < draw.styles.screen.w; j++) {
                Color color = draw.screenBuffer[i][j];
                uint16_t rgbU16 = st7789.colorToU16(color);
                pixels[j] = rgbU16;
                if (cacheBuffer[i][j] != rgbU16) {
                    changed = true;
                }
                cacheBuffer[i][j] = rgbU16;
            }
            if (changed) {
                st7789.drawRow(0, i, draw.styles.screen.w, pixels);
            }
        }
    }

    void setResetPin(int8_t pin) { resetPin = pin; }
    void setMadctl(uint8_t madctl) { st7789.madctl = madctl; }
    void setDisplayInverted(bool inverted) { st7789.displayInverted = inverted; }
    void setYRamMargin(int margin) { st7789.yRamMargin = margin; }
    void setDcPin(uint8_t pin) { spi.setGpioDataControl(pin); }
};
