#include "helpers/gpio.h"
#include "helpers/st7789.h"

//#define USE_SPI_DEV_MEM
#ifdef USE_SPI_DEV_MEM
// sudo apt-get install libbcm2835-dev
// sudo apt-get install libraspberrypi-dev raspberrypi-kernel-headers
// sudo chown 0:0 test2
// sudo chmod u+s test2
// see:
// https://raspberrypi.stackexchange.com/questions/40105/access-gpio-pins-without-root-no-access-to-dev-mem-try-running-as-root
//
// g++ -o test2 st7789_.cpp -lbcm2835 -fpermissive -lbcm_host && mv test2 ~/. && sudo ~/test2
#include "helpers/SpiDevMem.h"
#else
// g++ -o splash splash.cpp -fpermissive -I. && ./splash
#include "helpers/SpiDevSpi.h"
#endif

// res go to pin 15
// #define PIN_RESET 22
// #define GPIO_TFT_RESET_PIN 22
#define GPIO_TFT_RESET_PIN 2
// DC go to pin 11
// #define PIN_DATA_CONTROL 17
// #define GPIO_TFT_DATA_CONTROL 17
#define GPIO_TFT_DATA_CONTROL 3
// BLK go to pin 13 (should be optional or can be connected directly to 3.3v)
// #define PIN_BACKLIGHT 27
// #define GPIO_TFT_BACKLIGHT 3
#define GPIO_TFT_BACKLIGHT 17

// RPi BOARD (Physical not BCM) Pin #23 (SCLK) -> Display SCL
// RPi BOARD (Physical not BCM) Pin #19 (MOSI) -> Display SDA

void InitSPIDisplay()
{
}

int main(int argc, char* argv[])
{
    initGpio();
    gpioSetMode(GPIO_TFT_DATA_CONTROL, 0x01); // Data/Control pin to output (0x01)
    Spi spi = Spi(GPIO_TFT_DATA_CONTROL);
    spi.init();

    bool startup = argc > 1;

    if (startup) {
        logDebug("Off/On backlight on GPIO pin %d\n", GPIO_TFT_BACKLIGHT);
        gpioSetMode(GPIO_TFT_BACKLIGHT, 1);
        gpioWrite(GPIO_TFT_BACKLIGHT, 0);
        usleep(120 * 1000);
        gpioWrite(GPIO_TFT_BACKLIGHT, 1);

        // #ifdef USE_SPI_DEV_MEM
        logDebug("Resetting display at reset GPIO pin %d\n", GPIO_TFT_RESET_PIN);
        gpioSetMode(GPIO_TFT_RESET_PIN, 1);
        gpioWrite(GPIO_TFT_RESET_PIN, 1);
        usleep(120 * 1000);
        gpioWrite(GPIO_TFT_RESET_PIN, 0);
        usleep(120 * 1000);
        gpioWrite(GPIO_TFT_RESET_PIN, 1);
        usleep(120 * 1000);
        // #endif
    } else {
        logDebug("Skipping reset of display.");
    }

    logDebug("Initializing SPI bus.");

// Do the initialization with a very low SPI bus speed, so that it will succeed even if the bus speed chosen by the user is too high.
#ifdef USE_SPI_DEV_MEM
    spi.setSpeed(34);
#else
    spi.setSpeed(20000);
#endif

    ST7789 st7789([&](uint8_t cmd, uint8_t* data, uint32_t len) { spi.sendCmd(cmd, data, len); });
    // ILI9341 display (1)
    // st7789.madctl = 0x20;
    // st7789.displayInverted = false;
    // st7789.init(320, 240);

    // GMT024 display (2)
    // st7789.madctl = 0x08 | 0x20 |  0x40; // BGR + MV + MX
    // st7789.displayInverted = true;
    // st7789.init(320, 240);

    // st7789.madctl = 0x08;
    // st7789.displayInverted = true;
    // st7789.init(240, 240);

    // st7789.init(240, 320);

    st7789.init(320, 170);

    usleep(10 * 1000); // Delay a bit before restoring CLK, or otherwise this has been observed to cause the display not init if done back to back after the clear operation above.

#ifdef USE_SPI_DEV_MEM
    spi.setSpeed(20);
#else
    spi.setSpeed(16000000); // 16 MHz
#endif

    st7789.fillScreen(st7789.colorToU16({ 0x21, 0x25, 0x2b, 255 })); // #21252b

    // draw some random pixels
    for (int i = 0; i < 100; i++) {
        st7789.drawPixel(rand() % st7789.width, rand() % st7789.height, 0xFFFF);
    }

    // if parameter passed, show the logo
    if (startup) {
        st7789.drawFillRect(10 + 160, 110, 10, 10, st7789.colorToU16({ 212, 211, 211 })); //rgb(212, 211, 211)
        st7789.drawFillRect(30 + 160, 110, 10, 10, st7789.colorToU16({ 170, 170, 170 })); //rgb(170, 170, 170)
        st7789.drawFillRect(50 + 160, 110, 10, 10, st7789.colorToU16({ 127, 127, 127 })); //rgb(127, 127, 127)
        st7789.drawFillRect(70 + 160, 110, 10, 10, st7789.colorToU16({ 85, 85, 85 })); //rgb(85, 85, 85)
        st7789.drawFillRect(90 + 160, 110, 10, 10, st7789.colorToU16({ 42, 42, 42 })); //rgb(42, 42, 42)

        // P
        st7789.drawFillRect(10, 60, 10, 60, st7789.colorToU16({ 212, 211, 211 }));
        st7789.drawFillRect(10, 60, 30, 30, st7789.colorToU16({ 212, 211, 211 }));
        st7789.drawFillRect(20, 70, 10, 10, st7789.colorToU16({ 42, 42, 42 }));

        // I
        st7789.drawFillRect(50, 60, 10, 60, st7789.colorToU16({ 212, 211, 211 }));

        // X
        st7789.drawFillRect(70, 60, 10, 20, st7789.colorToU16({ 212, 211, 211 }));
        st7789.drawFillRect(90, 60, 10, 20, st7789.colorToU16({ 212, 211, 211 }));
        st7789.drawFillRect(80, 80, 10, 20, st7789.colorToU16({ 212, 211, 211 }));
        st7789.drawFillRect(70, 100, 10, 20, st7789.colorToU16({ 212, 211, 211 }));
        st7789.drawFillRect(90, 100, 10, 20, st7789.colorToU16({ 212, 211, 211 }));

        // E
        st7789.drawFillRect(110, 60, 10, 60, st7789.colorToU16({ 212, 211, 211 }));
        st7789.drawFillRect(110, 60, 30, 10, st7789.colorToU16({ 212, 211, 211 }));
        st7789.drawFillRect(110, 85, 20, 10, st7789.colorToU16({ 212, 211, 211 }));
        st7789.drawFillRect(110, 110, 30, 10, st7789.colorToU16({ 212, 211, 211 }));

        // L
        st7789.drawFillRect(150, 60, 10, 60, st7789.colorToU16({ 212, 211, 211 }));
        st7789.drawFillRect(150, 110, 30, 10, st7789.colorToU16({ 212, 211, 211 }));
    } else {
        while (true) {
            // sleep(1);
            usleep(250);
            st7789.fillScreen(st7789.colorToU16({ 0x21, 0x25, 0x2b, 255 })); // #21252b

            // draw some random pixels
            for (int i = 0; i < 100; i++) {
                st7789.drawPixel(rand() % st7789.width, rand() % st7789.height, 0xFFFF);
            }
        }
    }

    return 0;
}
