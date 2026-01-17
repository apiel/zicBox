/** Description:
This header file defines a specialized software component, named `DrawWithST7789`, which is responsible for managing and displaying graphics on an external screen that uses the ST7789 controller chip.

### Basic Idea of How it Works

This component acts as a high-speed translator between the program's desired image data and the physical display hardware. It manages this communication using two primary methods common in embedded systems:

1.  **SPI (Serial Peripheral Interface):** This is the high-speed data path used to rapidly send color information (pixels) to the screen controller.
2.  **GPIO (General Purpose Input/Output):** These are individual control wires used to send commands, manage the screen's power, or signal a hardware reset.

The core function of this class is to handle initialization, configuration, and efficient updates of the display.

### Key Functionality

**1. Initialization and Configuration:**
Upon startup, the component performs a precise setup sequence. This includes initializing the necessary input/output pins, configuring the SPI communication channel (sometimes adjusting its speed based on system needs), and executing a hardware reset sequence to wake up and prepare the ST7789 chip for drawing.

**2. Efficient Rendering (Caching):**
To ensure smooth and fast graphics, the class employs an optimization technique called "caching." It maintains two buffers: the current data the program wants to draw, and a snapshot of what was drawn in the *previous* frame. During the rendering process, it compares these two buffers and only sends new color data to the screen for rows that have actually changed. This dramatically reduces communication overhead.

**3. Graphics Management:**
It provides essential utility functions like `clear` (to reset the screen content to the background color) and methods to load specific operational settings (like screen orientation or reset pin assignment) via external configuration files.

Tags: Graphics Rendering, Display Driver, Screen Interface, Embedded Display, Color Management
sha: 0d7133dc60f42d2af6af6442777d326f71e9026e4451f89bf1bd55052c82abe8 
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
#include "draw/drawRenderer.h"

// Old one
// #define GPIO_TFT_DATA_CONTROL 17
// new one
#define GPIO_TFT_DATA_CONTROL 3

// BLK go to pin 13 (should be optional or can be connected directly to 3.3v)
// #define GPIO_TFT_BACKLIGHT 27

class DrawWithST7789 : public DrawRenderer {
public:
    uint16_t cacheBuffer[SCREEN_BUFFER_ROWS][SCREEN_BUFFER_COLS];

protected:
    int8_t resetPin = -1;
    Spi spi = Spi(GPIO_TFT_DATA_CONTROL);
    ST7789 st7789;
    Draw& draw;

    bool fullRendering = false;
    void fullRender()
    {
        uint16_t pixels[SCREEN_BUFFER_COLS];
        for (int i = 0; i < draw.styles.screen.h; i++) {
            for (int j = 0; j < draw.styles.screen.w; j++) {
                Color color = draw.screenBuffer[i][j];
                pixels[j] = st7789.colorToU16(color);
            }
            st7789.drawRow(0, i, draw.styles.screen.w, pixels);
        }
        fullRendering = false;
    }

public:
    DrawWithST7789(Draw& draw)
        : draw(draw)
        , st7789([&](uint8_t cmd, uint8_t* data, uint32_t len) { spi.sendCmd(cmd, data, len); })
    {
        logInfo("construct DrawWithST7789");
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

        st7789.init(draw.styles.screen.w, draw.styles.screen.h);
        usleep(10 * 1000); // Delay a bit before restoring CLK, or otherwise this has been observed to cause the display not init if done back to back after the clear operation above.

#ifdef USE_SPI_DEV_MEM
        spi.setSpeed(20);
#else
        spi.setSpeed(16000000); // 16 MHz
#endif
        draw.clear();

        logDebug("ST7789 initialized.");
    }

    void render() override
    {
        // if (fullRendering) {
        //     fullRender(); // <--- faster because there is no check on each rows
        //     return;
        // }
        // ^^^^^^^^^^^^^^^^^^^^ Is it even necessary, since most screen might have similar design, maybe better to still check on each rows

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
                st7789.yRamMargin = config["st7789"].value("yRamMargin", st7789.yRamMargin);
            }
        } catch (const std::exception& e) {
            logError("screen config: %s", e.what());
        }
    }
};
