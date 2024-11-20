// cp ../remoteZicBox/st7789_2.cpp . && g++ -o test23 st7789_2.cpp -lbcm2835 -lrt -DBCM2835=1 -fpermissive -lbcm_host && ./test23

#include <fcntl.h> // open, O_RDWR, O_SYNC
#include <inttypes.h>
#include <stdio.h> // printf, stderr
#include <string.h> // memset
#include <sys/ioctl.h>
#include <sys/mman.h> // mmap, munmap
#include <unistd.h> // usleep

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

static uint32_t spiSpeeds[7][3] = {
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
};
static int spiFds[7][3] = {
    { -1, -1, -1 },
    { -1, -1, -1 },
    { -1, -1, -1 },
    { -1, -1, -1 },
    { -1, -1, -1 },
    { -1, -1, -1 },
    { -1, -1, -1 },
};
typedef __signed__ char __s8;
typedef unsigned char __u8;
typedef __signed__ short __s16;
typedef unsigned short __u16;
typedef __signed__ int __s32;
typedef unsigned int __u32;
typedef __signed__ long long __s64;
typedef unsigned long long __u64;
#define SPI_IOC_MAGIC 'k'
/* Read / Write of SPI mode (SPI_MODE_0..SPI_MODE_3) (limited to 8 bits) */
#define SPI_IOC_RD_MODE _IOR(SPI_IOC_MAGIC, 1, __u8)
#define SPI_IOC_WR_MODE _IOW(SPI_IOC_MAGIC, 1, __u8)
/* Read / Write SPI bit justification */
#define SPI_IOC_RD_LSB_FIRST _IOR(SPI_IOC_MAGIC, 2, __u8)
#define SPI_IOC_WR_LSB_FIRST _IOW(SPI_IOC_MAGIC, 2, __u8)
/* Read / Write SPI device word length (1..N) */
#define SPI_IOC_RD_BITS_PER_WORD _IOR(SPI_IOC_MAGIC, 3, __u8)
#define SPI_IOC_WR_BITS_PER_WORD _IOW(SPI_IOC_MAGIC, 3, __u8)
/* Read / Write SPI device default max speed hz */
#define SPI_IOC_RD_MAX_SPEED_HZ _IOR(SPI_IOC_MAGIC, 4, __u32)
#define SPI_IOC_WR_MAX_SPEED_HZ _IOW(SPI_IOC_MAGIC, 4, __u32)
/* Read / Write of the SPI mode field */
#define SPI_IOC_RD_MODE32 _IOR(SPI_IOC_MAGIC, 5, __u32)
#define SPI_IOC_WR_MODE32 _IOW(SPI_IOC_MAGIC, 5, __u32)
static const uint8_t spiBPW = 8;
int wiringPiSPIxSetupMode(const int number, const int channel, const int speed, const int mode)
{
    int fd;
    char spiDev[32];

    if (mode < 0 || mode > 3) { // Mode is 0, 1, 2 or 3 original
        fprintf(stderr, "wiringPiSPI: Invalid mode (%d, valid range 0-%d)", mode, 3);
        return -1;
    }

    snprintf(spiDev, 31, "/dev/spidev%d.%d", number, channel);
    if ((fd = open(spiDev, O_RDWR)) < 0) {
        printf("Unable to open SPI device %s\n", spiDev);
        return -1;
    }
    spiSpeeds[number][channel] = speed;
    spiFds[number][channel] = fd;

    // Set SPI parameters.

    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
        // return wiringPiFailure(WPI_ALMOST, "SPI mode change failure: %s\n", strerror(errno));
        printf("SPI mode change failure\n");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spiBPW) < 0) {
        // return wiringPiFailure(WPI_ALMOST, "SPI BPW change failure: %s\n", strerror(errno));
        printf("SPI BPW change failure\n");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        // return wiringPiFailure(WPI_ALMOST, "SPI speed change failure: %s\n", strerror(errno));
        printf("SPI speed change failure\n");
        return -1;
    }

    return fd;
}
struct spi_ioc_transfer {
    __u64 tx_buf;
    __u64 rx_buf;
    __u32 len;
    __u32 speed_hz;
    __u16 delay_usecs;
    __u8 bits_per_word;
    __u8 cs_change;
    __u8 tx_nbits;
    __u8 rx_nbits;
    __u8 word_delay_usecs;
    __u8 pad;
};
static const uint16_t spiDelay = 0;
/* not all platforms use <asm-generic/ioctl.h> or _IOC_TYPECHECK() ... */
#define SPI_MSGSIZE(N)                                                  \
    ((((N) * (sizeof(struct spi_ioc_transfer))) < (1 << _IOC_SIZEBITS)) \
            ? ((N) * (sizeof(struct spi_ioc_transfer)))                 \
            : 0)
#define SPI_IOC_MESSAGE(N) _IOW(SPI_IOC_MAGIC, 0, char[SPI_MSGSIZE(N)])
int wiringPiSPIxDataRW(const int number, const int channel, unsigned char* data, const int len)
{

    if (-1 == spiFds[number][channel]) {
        fprintf(stderr, "wiringPiSPI: Invalid SPI number/channel (need wiringPiSPIxSetupMode before read/write)");
        return -1;
    }

    struct spi_ioc_transfer spi;
    // Mentioned in spidev.h but not used in the original kernel documentation
    //	test program )-:
    memset(&spi, 0, sizeof(spi));

    spi.tx_buf = (unsigned long)data;
    spi.rx_buf = (unsigned long)data;
    spi.len = len;
    spi.delay_usecs = spiDelay;
    spi.speed_hz = spiSpeeds[number][channel];
    spi.bits_per_word = spiBPW;

    return ioctl(spiFds[number][channel], SPI_IOC_MESSAGE(1), &spi);
}

void sendCmd(uint8_t cmd, uint8_t* payload, uint32_t payloadSize)
{
    int8_t* tStart;
    uint8_t* tEnd;
    uint8_t* tPrefillEnd;
    uint32_t cs;

    // An SPI transfer to the display always starts with one control (command) byte, followed by N data bytes.
    clearGpio(GPIO_TFT_DATA_CONTROL);

    if (payloadSize > 0) {
        setGpio(GPIO_TFT_DATA_CONTROL);

        // tStart = payload;
        // tEnd = payload + payloadSize;
        // tPrefillEnd = tStart + (payloadSize > 15 ? 15 : payloadSize);

        // while (tStart < tPrefillEnd)
        //     spi->fifo = *tStart++;
        // while (tStart < tEnd) {
        //     cs = spi->cs;
        //     if ((cs & BCM2835_SPI0_CS_TXD))
        //         spi->fifo = *tStart++;
        //     if ((cs & (BCM2835_SPI0_CS_RXR | BCM2835_SPI0_CS_RXF)))
        //         spi->cs = BCM2835_SPI0_CS_CLEAR_RX | BCM2835_SPI0_CS_TA | DISPLAY_SPI_DRIVE_SETTINGS;
        // }

        wiringPiSPIxDataRW(0, 0, payload, payloadSize);
    }

    // WaitForPolledSPITransferToFinish();
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

    int spi_fd = wiringPiSPIxSetupMode(0, 0, 2500000, 0);

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

    printf("Test stuff\n");
    // setGpioMode(GPIO_TFT_BACKLIGHT, 1);
    // setGpio(GPIO_TFT_BACKLIGHT);
    // usleep(100 * 1000);
    // clearGpio(GPIO_TFT_BACKLIGHT);
    // usleep(100 * 1000);
    // setGpio(GPIO_TFT_BACKLIGHT);

    return 0;
}
