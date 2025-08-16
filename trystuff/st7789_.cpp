#include "../helpers/gpio.h"

#include "../helpers/st7789.h"

#define USE_SPI_DEV_MEM
#ifdef USE_SPI_DEV_MEM
// sudo apt-get install libbcm2835-dev
// sudo apt-get install libraspberrypi-dev raspberrypi-kernel-headers
// sudo chown 0:0 test2
// sudo chmod u+s test2
// see:
// https://raspberrypi.stackexchange.com/questions/40105/access-gpio-pins-without-root-no-access-to-dev-mem-try-running-as-root
//
// g++ -o test2 st7789_.cpp -lbcm2835 -fpermissive -lbcm_host && mv test2 ~/. && sudo ~/test2
#include "../helpers/SpiDevMem.h"
#else
// g++ -o test2 st7789_.cpp -fpermissive && ./test2
#include "../helpers/SpiDevSpi.h"
#endif

// res go to pin 15
// #define PIN_RESET 22
// #define GPIO_TFT_RESET_PIN 22
#define GPIO_TFT_RESET_PIN 3
// DC go to pin 11
// #define PIN_DATA_CONTROL 17
#define GPIO_TFT_DATA_CONTROL 17
// BLK go to pin 13 (should be optional or can be connected directly to 3.3v)
// #define PIN_BACKLIGHT 27
#define GPIO_TFT_BACKLIGHT 27

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

#ifdef USE_SPI_DEV_MEM
    printf("Resetting display at reset GPIO pin %d\n", GPIO_TFT_RESET_PIN);
    gpioSetMode(GPIO_TFT_RESET_PIN, 1);
    gpioWrite(GPIO_TFT_RESET_PIN, 1);
    usleep(120 * 1000);
    gpioWrite(GPIO_TFT_RESET_PIN, 0);
    usleep(120 * 1000);
    gpioWrite(GPIO_TFT_RESET_PIN, 1);
    usleep(120 * 1000);
#endif

    printf("Initializing SPI bus\n");

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
    st7789.madctl = 0x08 | 0x20 |  0x40; // BGR + MV + MX
    st7789.displayInverted = true;
    st7789.init(320, 240);

    // st7789.madctl = 0x08;
    // st7789.displayInverted = true;
    // st7789.init(240, 240);

    usleep(10 * 1000); // Delay a bit before restoring CLK, or otherwise this has been observed to cause the display not init if done back to back after the clear operation above.

#ifdef USE_SPI_DEV_MEM
    spi.setSpeed(20);
#else
    spi.setSpeed(16000000); // 16 MHz
#endif

    st7789.test();
    return 0;
}
