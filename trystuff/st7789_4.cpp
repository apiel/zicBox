#ifdef PIGPIO
#include <pigpio.h>
#endif

#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstdint>

// res go to pin 15
#define PIN_RESET 22
#define GPIO_TFT_RESET_PIN 22
// DC go to pin 11
#define PIN_DATA_CONTROL 17
#define GPIO_TFT_DATA_CONTROL 17
// BLK go to pin 13 (should be optional or can be connected directly to 3.3v)
#define PIN_BACKLIGHT 27
#define GPIO_TFT_BACKLIGHT 27

#define SPI_Command_Mode 0
#define SPI_Data_Mode 1

int mode = -1;

// int spiWrite(unsigned handle, char *buf, unsigned count);

void writeMode(int spi, int _mode, const char* buf, unsigned count)
{
    if (mode != _mode) {
#ifdef PIGPIO
        gpioWrite(PIN_DATA_CONTROL, _mode);
#else
        printf("set mode %d\n", _mode);
#endif
        mode = _mode;
    }
#ifdef PIGPIO
    spiWrite(spi, (char*)buf, count);
#else
    printf("write data: ");
    for (int i = 0; i < count; i++) {
        printf("%02x ", buf[i]);
    }
    printf("\n");
#endif
}

void writeCmd(int spi, char cmd)
{
    writeMode(spi, SPI_Command_Mode, &cmd, 1);
}

void writeData(int spi, const char* data, unsigned count)
{
    writeMode(spi, SPI_Data_Mode, data, count);
}

void writeData(int spi, char data)
{
    writeData(spi, &data, 1);
}

void writeAddr(int spi, int addr1, int addr2)
{
    char data[4];
    data[0] = (addr1 >> 8) & 0xFF;
    data[1] = addr1 & 0xFF;
    data[2] = (addr2 >> 8) & 0xFF;
    data[3] = addr2 & 0xFF;
    writeData(spi, data, 4);
}

// void drawFilledRect(int spi, int x, int y, int w, int h, int color)
// {
//     writeCmd(spi, 0x2A); // set column address
//     writeAddr(spi, x, x + w - 1);
//     writeCmd(spi, 0x2B); // set row address
//     writeAddr(spi, y, y + h - 1);

// }

void drawPixel(int spi, int x, int y, uint16_t color)
{
    writeCmd(spi, 0x2A); // set column address
    writeAddr(spi, x, x);
    writeCmd(spi, 0x2B); // set row address
    writeAddr(spi, y, y);

    writeCmd(spi, 0x2C); // memory Write

    char data[2];
    data[0] = (color >> 8) & 0xFF;
    data[1] = color & 0xFF;
    writeData(spi, data, 2);
}

int main(int argc, char** argv)
{
#ifdef PIGPIO
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Failed to initialise GPIO\n");
        return 1;
    }
#endif

    // initialize SPI
#ifdef PIGPIO
    int spi = spiOpen(0, 32000, 0);
    // int spi = spiOpen(0, 1000000, 0);
#else
    int spi = 0;
#endif

    if (spi < 0) {
        fprintf(stderr, "spiOpen failed\n");
        return 1;
    }

#ifdef PIGPIO
    gpioSetMode(PIN_RESET, PI_OUTPUT);
    gpioWrite(PIN_RESET, 1);
    usleep(50000); // sleep 50ms
    gpioWrite(PIN_RESET, 0);
    usleep(50000); // sleep 50ms
    gpioWrite(PIN_RESET, 1);
    usleep(50000); // sleep 50ms

    gpioSetMode(PIN_BACKLIGHT, PI_OUTPUT);
    gpioWrite(PIN_BACKLIGHT, 0);
    gpioSetMode(PIN_DATA_CONTROL, PI_OUTPUT);
#endif

    writeCmd(spi, 0x01); // reset
    usleep(150000); // sleep 150ms
    writeCmd(spi, 0x11); // sleep out
    usleep(255000); // sleep 255ms
    writeCmd(spi, 0x3A); // set pixel format
    // writeCmd(spi, 0x55); // 16bit
    writeCmd(spi, 0x05); // 16bpp
    usleep(10000); // sleep 10ms

    writeCmd(spi, 0x36); // set memory address
    usleep(10000); // sleep 10ms

    writeCmd(spi, 0x2A); // set column address
    // char columnData[4] = { 0x00, 0x00, 0x00, 128 }; // 240 ???
    char columnData[4] = { 0x00, 0x00, 0x00, (char)240 };
    // char columnData[4] = { 0x00, 0x00, 240 >> 8, 240 & 0xFF };
    writeData(spi, columnData, 4);

    writeCmd(spi, 0x2B); // set row address
    // char rowData[4] = { 0x00, 0x00, 0x00, 128 }; // 240 ???
    char rowData[4] = { 0x00, 0x00, 0x00, (char)240 };
    // char rowData[4] = { 0x00, 0x00, 240 >> 8, 240 & 0xFF };
    writeData(spi, rowData, 4);

    writeCmd(spi, 0x21); // Display Inversion On
    writeCmd(spi, 0x13); // Normal Display Mode On
    writeCmd(spi, 0x29); // display on
    usleep(255000); // sleep 255ms

#ifdef PIGPIO
    gpioWrite(PIN_BACKLIGHT, 1);
#endif

    usleep(150000); // sleep 150ms

    // draw some random pixel
    for (int i = 0; i < 100; i++) {
        int x = rand() % 120;
        int y = rand() % 120;
        uint16_t color = rand() % 0xFFFFFF;
        drawPixel(spi, x, y, color);
    }


    printf("sleep for 5 seconds\n");
    usleep(5000000); // sleep 5s
    printf("end\n");

#ifdef PIGPIO
    spiClose(spi);
#endif
    return 0;
}