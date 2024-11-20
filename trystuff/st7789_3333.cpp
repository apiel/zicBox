// cp ../remoteZicBox/st7789_2.cpp . && g++ -o test23 st7789_2.cpp -lbcm2835 -lrt -DBCM2835=1 -fpermissive -lbcm_host && ./test23

#include <fcntl.h> // open, O_RDWR, O_SYNC
#include <inttypes.h>
#include <stdio.h> // printf, stderr
#include <string.h> // memset
#include <sys/ioctl.h>
#include <sys/mman.h> // mmap, munmap
#include <unistd.h> // usleep

#include <cmath>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <linux/spi/spidev.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// res go to pin 15
// #define PIN_RESET 22
#define GPIO_TFT_RESET_PIN 22
// DC go to pin 11
// #define PIN_DATA_CONTROL 17
#define GPIO_TFT_DATA_CONTROL 17
// BLK go to pin 13 (should be optional or can be connected directly to 3.3v)
// #define PIN_BACKLIGHT 27
#define GPIO_TFT_BACKLIGHT 27

typedef struct GPIORegisterFile {
    uint32_t gpfsel[6], reserved0; // GPIO Function Select registers, 3 bits per pin, 10 pins in an uint32_t
    uint32_t gpset[2], reserved1; // GPIO Pin Output Set registers, write a 1 to bit at index I to set the pin at index I high
    uint32_t gpclr[2], reserved2; // GPIO Pin Output Clear registers, write a 1 to bit at index I to set the pin at index I low
    uint32_t gplev[2];
} GPIORegisterFile;
volatile GPIORegisterFile* gpio = 0;

// static volatile unsigned int *gpio ;

// extern volatile void* bcm2835;
// volatile void* bcm2835 = 0;

void setGpioMode(uint8_t pin, uint8_t mode)
{
    gpio->gpfsel[(pin) / 10] = (gpio->gpfsel[(pin) / 10] & ~(0x7 << ((pin) % 10) * 3)) | ((mode) << ((pin) % 10) * 3);
}

void setGpio(uint8_t pin)
{
    gpio->gpset[0] = 1 << (pin); // Pin must be (0-31)
}

void clearGpio(uint8_t pin)
{
    gpio->gpclr[0] = 1 << (pin); // Pin must be (0-31)
}

// #define BCM2835_GPIO_BASE 0x200000 // Address to GPIO register file
// #define BCM2835_SPI0_BASE 0x204000 // Address to SPI0 register file

#define BLOCK_SIZE (4 * 1024)

#define GPIO_BASE 0x00200000 // 0_00200000

#define GPIO_SPI0_MOSI 10 // Pin P1-19, MOSI when SPI0 in use
#define GPIO_SPI0_MISO 9 // Pin P1-21, MISO when SPI0 in use
#define GPIO_SPI0_CLK 11 // Pin P1-23, CLK when SPI0 in use

/// spi
static void pabort(const char* s)
{
    printf("!! %s\n", s);
    perror(s);
    abort();
}

// uint32_t speed = 16000000; // 16 MHz;
uint32_t speed = 2500000;
int init_spi(void)
{
    int fd;
    int ret;
    uint8_t mode;
    uint8_t bits;

    fd = open("/dev/spidev0.0", O_RDWR);
    if (fd < 0)
        pabort("Can't open device");

    // Normal/default SPI mode
    //
    mode = 0;
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        pabort("Can't set SPI write mode");
    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        pabort("Can't get SPI read mode");
    printf("SPI mode: %d\n", mode);

    // Bits per word
    bits = 8;
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("Can't set bits");
    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("Can't get bits\n");
    printf("Bits per word: %d\n", bits);

    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("Can't set max speed");
    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("Can't get max speed\n");
    printf("Max speed: %ld\n", speed);

    return fd;
}

void write_buffer(int fd, uint8_t* buffer, size_t len)
{
    int ret;
    uint8_t* pos;
    size_t remain;
    size_t send_len;
    struct spi_ioc_transfer tr = { 0 };

    // There's a limit of about 4K per ioctl here, so send everything in
    // 4K chunks

#define SPI_BUFFER_MAX (4095)

    pos = buffer;
    remain = len;

    while (remain) {
        send_len = (remain > SPI_BUFFER_MAX) ? SPI_BUFFER_MAX : remain;

        tr.tx_buf = (unsigned long)pos;
        tr.len = send_len;

        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if (ret < 1)
            pabort("Can't write SPI buffer");

        pos += send_len;
        remain -= send_len;
    }
}

// void write_command(int fd, uint8_t value)
// {
//     command_mode();
//     write_buffer(fd, &value, 1);
// }

// void write_data(int fd, uint8_t value)
// {
//     data_mode();
//     write_buffer(fd, &value, 1);
// }

// void write_data16(int fd, uint16_t value)
// {
//     uint8_t buffer[2];

//     buffer[0] = value >> 8;
//     buffer[1] = value & 0xff;

//     data_mode();
//     write_buffer(fd, buffer, 2);
// }

// void write_data_buffer(int fd, uint8_t* buffer, size_t len)
// {
//     data_mode();
//     write_buffer(fd, buffer, len);
// }

int fdspi;
///////end

void sendCmd(uint8_t cmd, uint8_t* payload, uint32_t payloadSize)
{
    // An SPI transfer to the display always starts with one control (command) byte, followed by N data bytes.
    clearGpio(GPIO_TFT_DATA_CONTROL);
    write_buffer(fdspi, &cmd, 1);

    if (payloadSize > 0) {
        setGpio(GPIO_TFT_DATA_CONTROL);
        write_buffer(fdspi, payload, payloadSize);

        // uint8_t* p = new uint8_t[payloadSize + 1];
        // p[0] = cmd;
        // memcpy(p + 1, payload, payloadSize);
        // // spiWrite((char*)p, payloadSize + 1);
        // // spiRead((char*)p, payloadSize + 1);
        // // spiXfer((char*)p, NULL, payloadSize + 1);

        // write_buffer(fdspi, p, payloadSize + 1);

        // delete[] p;

        // spiXfer((char*)&cmd, NULL, 1);
        // spiXfer((char*)payload, NULL, payloadSize);
    }

    // WaitForPolledSPITransferToFinish();
    // usleep(1000);
}

void sendCmdOnly(uint8_t cmd)
{
    sendCmd(cmd, NULL, 0);
}

void sendCmdData(uint8_t cmd, uint8_t data)
{
    sendCmd(cmd, &data, 1);
}

#define DISPLAY_SET_CURSOR_X 0x2A
#define DISPLAY_SET_CURSOR_Y 0x2B
#define DISPLAY_WRITE_PIXELS 0x2C

#define BYTESPERPIXEL 2

void sendAddr(uint8_t cmd, uint16_t addr0, uint16_t addr1)
{
    uint8_t addr[4] = { (uint8_t)(addr0 >> 8), (uint8_t)(addr0 & 0xFF), (uint8_t)(addr1 >> 8), (uint8_t)(addr1 & 0xFF) };
    sendCmd(cmd, addr, 4);
}

void drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    uint8_t data[BYTESPERPIXEL] = { color >> 8, color & 0xFF };

    sendAddr(DISPLAY_SET_CURSOR_X, (uint16_t)x, (uint16_t)x);
    sendAddr(DISPLAY_SET_CURSOR_Y, (uint16_t)y, (uint16_t)y);
    sendCmd(DISPLAY_WRITE_PIXELS, data, 2);
}

void drawFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    int yPos;
    uint16_t size = w * BYTESPERPIXEL;
    uint8_t pixels[size];
    uint8_t pixel[BYTESPERPIXEL] = { color >> 8, color & 0xFF };

    for (uint16_t i = 0; i < size; i += BYTESPERPIXEL) {
        pixels[i] = pixel[0];
        pixels[i + 1] = pixel[1];
    }

    for (yPos = 0; yPos < h; ++yPos) {
        sendAddr(DISPLAY_SET_CURSOR_X, x, x + w);
        sendAddr(DISPLAY_SET_CURSOR_Y, y + yPos, y + yPos);
        sendCmd(DISPLAY_WRITE_PIXELS, pixels, size);
    }
}

int main(int argc, char* argv[])
{
    printf("Start spi display test.\n");
    int gpiomem_fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (gpiomem_fd < 0) {
        fprintf(stderr, "can't open /dev/gpiomem (fix permissions: https://raspberrypi.stackexchange.com/questions/40105/access-gpio-pins-without-root-no-access-to-dev-mem-try-running-as-root)\n");
        return -1;
    }

    printf("Nmap gpiomem...\n");
    gpio = (volatile GPIORegisterFile*)mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, gpiomem_fd, GPIO_BASE);
    if (gpio == MAP_FAILED) {
        fprintf(stderr, "mmap (GPIO) failed\n");
        return -1;
    }

    setGpioMode(GPIO_TFT_DATA_CONTROL, 0x01); // Data/Control pin to output (0x01)
    setGpioMode(GPIO_SPI0_MOSI, 0x04);
    setGpioMode(GPIO_SPI0_CLK, 0x04);

    fdspi = init_spi();

    printf("Resetting display at reset GPIO pin %d\n", GPIO_TFT_RESET_PIN);
    setGpioMode(GPIO_TFT_RESET_PIN, 1);
    setGpio(GPIO_TFT_RESET_PIN);
    usleep(120 * 1000);
    clearGpio(GPIO_TFT_RESET_PIN);
    usleep(120 * 1000);
    setGpio(GPIO_TFT_RESET_PIN);
    usleep(120 * 1000);

    printf("Initializing SPI bus\n");

    sendCmdOnly(0x11 /*Sleep Out*/);
    usleep(120 * 1000);
    sendCmdData(0x3A /*COLMOD: Pixel Format Set*/, 0x05 /*16bpp*/);
    usleep(20 * 1000);

#define MADCTL_COLUMN_ADDRESS_ORDER_SWAP (1 << 6)
#define MADCTL_ROW_ADDRESS_ORDER_SWAP (1 << 7)
#define MADCTL_ROTATE_180_DEGREES (MADCTL_COLUMN_ADDRESS_ORDER_SWAP | MADCTL_ROW_ADDRESS_ORDER_SWAP)
/* RGB/BGR Order ('0' = RGB, '1' = BGR) */
#define ST7789_MADCTL_RGB 0x00
#define ST7789_MADCTL_BGR 0x08

    uint8_t madctl = 0;
    madctl |= MADCTL_ROW_ADDRESS_ORDER_SWAP;
    madctl |= ST7789_MADCTL_BGR; // Because we swap order, we need to use BGR...
    madctl ^= MADCTL_ROTATE_180_DEGREES;
    sendCmdData(0x36 /*MADCTL: Memory Access Control*/, madctl);
    usleep(10 * 1000);

    sendCmdData(0xBA /*DGMEN: Enable Gamma*/, 0x04);

    sendCmdOnly(0x21 /*Display Inversion On*/);
    // sendCmd(0x20 /*Display Inversion Off*/);

    sendCmdOnly(0x13 /*NORON: Partial off (normal)*/);
    usleep(10 * 1000);

    uint8_t data[4] = { 0, 0, (uint8_t)(240 >> 8), (uint8_t)(240 & 0xFF) };
    // The ST7789 controller is actually a unit with 320x240 graphics memory area, but only 240x240 portion
    // of it is displayed. Therefore if we wanted to swap row address mode above, writes to Y=0...239 range will actually land in
    // memory in row addresses Y = 319-(0...239) = 319...80 range. To view this range, we must scroll the view by +80 units in Y
    // direction so that contents of Y=80...319 is displayed instead of Y=0...239.
    if ((madctl & MADCTL_ROW_ADDRESS_ORDER_SWAP)) {
        sendCmd(0x37 /*VSCSAD: Vertical Scroll Start Address of RAM*/, data, 4);
    }

#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240

    // drawFillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0); // clear screen
    // uint16_t randomColor = rand() % 0xFFFFFF;
    // drawFillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, randomColor); // clear screen
    drawFillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0xFF00FF); // clear screen

    for (int i = 0; i < 100; i++) {
        drawPixel(i, i, 0xFFFF00);
    }

    // printf("Test stuff\n");
    // setGpioMode(GPIO_TFT_BACKLIGHT, 1);
    // setGpio(GPIO_TFT_BACKLIGHT);
    // usleep(100 * 1000);
    // clearGpio(GPIO_TFT_BACKLIGHT);
    // usleep(100 * 1000);
    // setGpio(GPIO_TFT_BACKLIGHT);

    return 0;
}
