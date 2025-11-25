/** Description:
This program acts as a sophisticated driver designed to initialize and manage a small color liquid crystal display (LCD) that uses the ST7789 controller chip. It is typically intended for use on a single-board computer, like a Raspberry Pi.

The core function is establishing reliable, high-speed communication between the computer and the screen using specific digital channels.

### System Setup and Communication

The software first prepares the computer’s General Purpose Input/Output (GPIO) pins. These pins are essential for control: one pin dictates whether the incoming digital signal is a command (like “turn on”) or image data, and another is used to physically reset the screen. The program utilizes a fast data transfer method called SPI (Serial Peripheral Interface) to move visual information quickly.

Since accessing hardware directly can be complex, the code is structured to handle two different methods of physical hardware interaction, one of which requires special permissions for direct control access.

### Display Initialization Sequence

The sequence begins with a physical reset, sending precise signals to the screen's reset pin to ensure it starts in a known state.

Next, the program begins the configuration process. To prevent errors during this critical setup phase, it temporarily sets the SPI communication speed very low. It sends a series of complex commands to the ST7789 controller, informing it of crucial details like the screen’s physical size (e.g., 320x240 pixels), how colors should be ordered, and its orientation.

Once all configuration commands are successfully delivered, the program increases the SPI communication speed significantly. This ensures that when the screen is used for drawing graphics or video, the updates happen as quickly as possible.

The program concludes by running a built-in test routine, displaying a pattern to visually confirm that the software, hardware, and physical wiring are all working correctly.

sha: dd11e8ef61b06a6a22b556abc5b06d50cdfe22245b1a8696859621926f5f1509 
*/
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
