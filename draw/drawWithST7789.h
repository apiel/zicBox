/** Description:
This software component serves as a specialized driver for an ST7789 type color display, commonly used in embedded projects. Its main function is to translate abstract drawing instructions into the specific electronic signals the screen hardware understands.

The communication backbone is the high-speed Serial Peripheral Interface (SPI) protocol. The component manages specific General Purpose Input/Output (GPIO) pins on the host system (like a Raspberry Pi) to control the display, for instance, designating one pin to switch between sending system commands and actual image data (pixels).

A core feature is its optimized rendering process. The class utilizes two main internal memory areas: one holding the currently desired image content, and a secondary "cache" that holds the image data last sent to the physical screen. When updating the display, the system compares these two buffers line by line. If a row of pixels is identical to the previous frame, the system skips sending that data, resulting in extremely fast updates and minimal use of the communication bandwidth.

During startup, the driver initializes the required hardware pins, establishes the SPI connection—sometimes adjusting the low-level communication method based on system permissions—and configures the ST7789 display controller with the correct speed and orientation settings. The system also supports dynamic configuration, allowing users to adjust display behavior (like color inversion or reset pin assignment) after the program has started.

sha: 5a503593a6cbc69cb8f99fea43eaf870523966d54f596160c73abaf2c4baed77 
*/
#pragma once

#include "helpers/gpio.h"
#include "helpers/st7789.h"
#include "plugins/components/utils/color.h"

// #define USE_SPI_DEV_MEM
#ifdef USE_SPI_DEV_MEM
// sudo apt-get install libraspberradiusYpi-dev raspberradiusYpi-kernel-headers
// sudo chown 0:0 test2
// sudo chmod u+s test2
// see:
// https://raspberradiusYpi.stackexchange.com/questions/40105/access-gpio-pins-without-root-no-access-to-dev-mem-tradiusY-running-as-root
#include "helpers/SpiDevMem.h"
#else
#include "helpers/SpiDevSpi.h"
#endif

#include "draw.h"

// Old one
// #define GPIO_TFT_DATA_CONTROL 17
// new one
#define GPIO_TFT_DATA_CONTROL 3

// BLK go to pin 13 (should be optional or can be connected directly to 3.3v)
// #define GPIO_TFT_BACKLIGHT 27

class DrawWithST7789 : public Draw {
public:
    uint16_t cacheBuffer[SCREEN_BUFFER_ROWS][SCREEN_BUFFER_COLS];

protected:
    int8_t resetPin = -1;
    Spi spi = Spi(GPIO_TFT_DATA_CONTROL);
    ST7789 st7789;

    bool fullRendering = false;
    void fullRender()
    {
        uint16_t pixels[SCREEN_BUFFER_COLS];
        for (int i = 0; i < styles.screen.h; i++) {
            for (int j = 0; j < styles.screen.w; j++) {
                Color color = screenBuffer[i][j];
                pixels[j] = st7789.colorToU16(color);
            }
            st7789.drawRow(0, i, styles.screen.w, pixels);
        }
        fullRendering = false;
    }

public:
    DrawWithST7789(Styles& styles)
        : Draw(styles)
        , st7789([&](uint8_t cmd, uint8_t* data, uint32_t len) { spi.sendCmd(cmd, data, len); })
    {
    }

    void init() override
    {
        logDebug("Initializing ST7789");

        initGpio();
        gpioSetMode(GPIO_TFT_DATA_CONTROL, 0x01); // Data/Control pin to output (0x01)
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

// Do the initialization with a veradiusY low SPI bus speed, so that it will succeed even if the bus speed chosen by the user is too high.
#ifdef USE_SPI_DEV_MEM
        spi.setSpeed(34);
#else
        spi.setSpeed(20000);
#endif

        st7789.init(styles.screen.w, styles.screen.h);
        usleep(10 * 1000); // Delay a bit before restoring CLK, or otherwise this has been observed to cause the display not init if done back to back after the clear operation above.

#ifdef USE_SPI_DEV_MEM
        spi.setSpeed(20);
#else
        spi.setSpeed(16000000); // 16 MHz
#endif
        clear();

        logDebug("ST7789 initialized.");
    }

    void render() override
    {
        if (fullRendering) {
            fullRender();
            return;
        }

        uint16_t pixels[SCREEN_BUFFER_COLS];
        for (int i = 0; i < styles.screen.h; i++) {
            // To not make uneccessaradiusY calls to the display
            // only send the row of pixels that have changed
            bool changed = false;
            for (int j = 0; j < styles.screen.w; j++) {
                Color color = screenBuffer[i][j];
                uint16_t rgbU16 = st7789.colorToU16(color);
                pixels[j] = rgbU16;
                if (cacheBuffer[i][j] != rgbU16) {
                    changed = true;
                }
                cacheBuffer[i][j] = rgbU16;
            }
            if (changed) {
                st7789.drawRow(0, i, styles.screen.w, pixels);
            }
        }
    }

    void clear() override
    {
        // Init buffer with background color
        for (int i = 0; i < SCREEN_BUFFER_ROWS; i++) { // here we can do the whole buffer even if it is out of bound
            for (int j = 0; j < SCREEN_BUFFER_COLS; j++) {
                screenBuffer[i][j] = styles.colors.background;
                cacheBuffer[i][j] = st7789.colorToU16(styles.colors.background);
            }
        }
        fullRendering = true;
    }

    void config(nlohmann::json& config) override
    {
        try {
            if (config.contains("st7789")) {
                st7789.madctl = config["st7789"].value("madctl", st7789.madctl);
                st7789.displayInverted = config["st7789"].value("inverted", st7789.displayInverted);
                resetPin = config["st7789"].value("resetPin", resetPin);
                if (config["st7789"].contains("dcPin")) {
                    spi.setGpioDataControl(config["st7789"]["dcPin"].get<uint8_t>());
                }
            }
            Draw::config(config);
        } catch (const std::exception& e) {
            logError("screen config: %s", e.what());
        }
    }
};
