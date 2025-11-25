/** Description:
This highly specialized C++ program is designed to control a small color display, specifically an ST7789 TFT screen, when connected to a Raspberry Pi.

The code's purpose is to bypass the standard operating system drivers and communicate directly with the Pi's internal hardware. It achieves this by using a technique called memory mapping, which allows the program to read and write directly to the critical control locations (registers) that govern the General Purpose Input/Output (GPIO) pins and the high-speed communication block (SPI).

**How It Works:**

1.  **Setup (InitSPI):** The program first requests direct access to the Raspberry Pi's physical hardware memory. It configures specific GPIO pins to act as control lines for the screen (like reset and data/command toggle) and sets up the SPI communication hardware for fast data transfer.
2.  **Initialization (InitSPIDisplay):** It sends a sequence of specific commands to the ST7789 display controller. These commands reset the screen, define the color format (how colors are encoded), and set the display orientation.
3.  **Graphics:** Once initialized, the program includes functions to draw basic shapes. It defines the screen area being written to (using cursor commands) and then transmits raw color data over the fast SPI channel.
4.  **Result:** The program demonstrates its capability by drawing a large magenta background and a short yellow diagonal line of pixels before turning on the display's backlight, successfully activating the screen using direct hardware control.

In essence, this code acts as a custom, high-speed driver, giving the developer ultimate control over the connected display hardware.

sha: 53cfaa9320f363b60508e3e70dd090a25c9e346262c0700da2ac19577dd14bab 
*/
// sudo chown 0:0 test2
// sudo chmod u+s test2

// sudo apt-get install libraspberrypi-dev raspberrypi-kernel-headers
// cp ../remoteZicBox/st7789.cpp . && g++ -o test2 st7789.cpp -lbcm2835 -lrt -DBCM2835=1 -fpermissive -lbcm_host && sudo ./test2

// https://raspberrypi.stackexchange.com/questions/40105/access-gpio-pins-without-root-no-access-to-dev-mem-try-running-as-root

// g++ -o test2 st7789.cpp -lbcm2835 -fpermissive -lbcm_host && mv test2 ~/. && sudo ~/test2

#include <fcntl.h> // open, O_RDWR, O_SYNC
#include <inttypes.h>
#include <stdio.h> // printf, stderr
#include <sys/mman.h> // mmap, munmap
#include <unistd.h> // usleep

#define HAS_BCM 1 // comment for testing on non-BCM hardware (desktops, etc.)
#ifdef HAS_BCM
#include <bcm_host.h> // bcm_host_get_peripheral_address, bcm_host_get_peripheral_size, bcm_host_get_sdram_address
#endif

// res go to pin 15
// #define PIN_RESET 22
#define GPIO_TFT_RESET_PIN 22
// DC go to pin 11
// #define PIN_DATA_CONTROL 17
#define GPIO_TFT_DATA_CONTROL 17
// BLK go to pin 13 (should be optional or can be connected directly to 3.3v)
// #define PIN_BACKLIGHT 27
#define GPIO_TFT_BACKLIGHT 27

// RPi BOARD (Physical not BCM) Pin #23 (SCLK) -> Display SCL
// RPi BOARD (Physical not BCM) Pin #19 (MOSI) -> Display SDA

// #define GPIO_TFT_DATA_CONTROL 6
// #define GPIO_TFT_RESET_PIN 5
// #define GPIO_TFT_BACKLIGHT 13

#define SPI_BUS_CLOCK_DIVISOR 20

#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240
// #define DISPLAY_WIDTH 170
// #define DISPLAY_HEIGHT 320

#define BCM2835_GPIO_BASE 0x200000 // Address to GPIO register file
#define BCM2835_SPI0_BASE 0x204000 // Address to SPI0 register file
#define BCM2835_TIMER_BASE 0x3000 // Address to System Timer register file

#define BCM2835_SPI0_CS_RXF 0x00100000 // Receive FIFO is full
#define BCM2835_SPI0_CS_RXR 0x00080000 // FIFO needs reading
#define BCM2835_SPI0_CS_TXD 0x00040000 // TXD TX FIFO can accept Data
#define BCM2835_SPI0_CS_RXD 0x00020000 // RXD RX FIFO contains Data
#define BCM2835_SPI0_CS_DONE 0x00010000 // Done transfer Done
#define BCM2835_SPI0_CS_ADCS 0x00000800 // Automatically Deassert Chip Select
#define BCM2835_SPI0_CS_INTR 0x00000400 // Fire interrupts on RXR?
#define BCM2835_SPI0_CS_INTD 0x00000200 // Fire interrupts on DONE?
#define BCM2835_SPI0_CS_DMAEN 0x00000100 // Enable DMA transfers?
#define BCM2835_SPI0_CS_TA 0x00000080 // Transfer Active
#define BCM2835_SPI0_CS_CLEAR 0x00000030 // Clear FIFO Clear RX and TX
#define BCM2835_SPI0_CS_CLEAR_RX 0x00000020 // Clear FIFO Clear RX
#define BCM2835_SPI0_CS_CLEAR_TX 0x00000010 // Clear FIFO Clear TX
#define BCM2835_SPI0_CS_CPOL 0x00000008 // Clock Polarity
#define BCM2835_SPI0_CS_CPHA 0x00000004 // Clock Phase
#define BCM2835_SPI0_CS_CS 0x00000003 // Chip Select

#define BCM2835_SPI0_CS_RXF_SHIFT 20
#define BCM2835_SPI0_CS_RXR_SHIFT 19
#define BCM2835_SPI0_CS_TXD_SHIFT 18
#define BCM2835_SPI0_CS_RXD_SHIFT 17
#define BCM2835_SPI0_CS_DONE_SHIFT 16
#define BCM2835_SPI0_CS_ADCS_SHIFT 11
#define BCM2835_SPI0_CS_INTR_SHIFT 10
#define BCM2835_SPI0_CS_INTD_SHIFT 9
#define BCM2835_SPI0_CS_DMAEN_SHIFT 8
#define BCM2835_SPI0_CS_TA_SHIFT 7
#define BCM2835_SPI0_CS_CLEAR_RX_SHIFT 5
#define BCM2835_SPI0_CS_CLEAR_TX_SHIFT 4
#define BCM2835_SPI0_CS_CPOL_SHIFT 3
#define BCM2835_SPI0_CS_CPHA_SHIFT 2
#define BCM2835_SPI0_CS_CS_SHIFT 0

#define GPIO_SPI0_MOSI 10 // Pin P1-19, MOSI when SPI0 in use
#define GPIO_SPI0_MISO 9 // Pin P1-21, MISO when SPI0 in use
#define GPIO_SPI0_CLK 11 // Pin P1-23, CLK when SPI0 in use
#define GPIO_SPI0_CE0 8 // Pin P1-24, CE0 when SPI0 in use
#define GPIO_SPI0_CE1 7 // Pin P1-26, CE1 when SPI0 in use

#define DISPLAY_SPI_DRIVE_SETTINGS (1 | BCM2835_SPI0_CS_CPOL | BCM2835_SPI0_CS_CPHA)

typedef struct GPIORegisterFile {
    uint32_t gpfsel[6], reserved0; // GPIO Function Select registers, 3 bits per pin, 10 pins in an uint32_t
    uint32_t gpset[2], reserved1; // GPIO Pin Output Set registers, write a 1 to bit at index I to set the pin at index I high
    uint32_t gpclr[2], reserved2; // GPIO Pin Output Clear registers, write a 1 to bit at index I to set the pin at index I low
    uint32_t gplev[2];
} GPIORegisterFile;
volatile GPIORegisterFile* gpio = 0;

extern volatile void* bcm2835;

int mem_fd = -1;
volatile void* bcm2835 = 0;

typedef struct SPIRegisterFile {
    uint32_t cs; // SPI Master Control and Status register
    uint32_t fifo; // SPI Master TX and RX FIFOs
    uint32_t clk; // SPI Master Clock Divider
    uint32_t dlen; // SPI Master Number of DMA Bytes to Write
} SPIRegisterFile;
volatile SPIRegisterFile* spi;

void gpioSetMode(uint8_t pin, uint8_t mode)
{
#ifdef HAS_BCM
    gpio->gpfsel[(pin) / 10] = (gpio->gpfsel[(pin) / 10] & ~(0x7 << ((pin) % 10) * 3)) | ((mode) << ((pin) % 10) * 3);
#endif
}

void setGpio(uint8_t pin)
{
#ifdef HAS_BCM
    gpio->gpset[0] = 1 << (pin); // Pin must be (0-31)
#endif
}

void clearGpio(uint8_t pin)
{
#ifdef HAS_BCM
    gpio->gpclr[0] = 1 << (pin); // Pin must be (0-31)
#endif
}

void WaitForPolledSPITransferToFinish()
{
#ifdef HAS_BCM
    uint32_t cs;
    while (!(((cs = spi->cs) ^ BCM2835_SPI0_CS_TA) & (BCM2835_SPI0_CS_DONE | BCM2835_SPI0_CS_TA))) // While TA=1 and DONE=0
        if ((cs & (BCM2835_SPI0_CS_RXR | BCM2835_SPI0_CS_RXF)))
            spi->cs = BCM2835_SPI0_CS_CLEAR_RX | BCM2835_SPI0_CS_TA | DISPLAY_SPI_DRIVE_SETTINGS;

    if ((cs & BCM2835_SPI0_CS_RXD))
        spi->cs = BCM2835_SPI0_CS_CLEAR_RX | BCM2835_SPI0_CS_TA | DISPLAY_SPI_DRIVE_SETTINGS;
#endif
}

void sendCmd(uint8_t cmd, uint8_t* payload, uint32_t payloadSize)
{
#ifdef HAS_BCM
    int8_t* tStart;
    uint8_t* tEnd;
    uint8_t* tPrefillEnd;
    uint32_t cs;

    // WaitForPolledSPITransferToFinish();

    spi->cs = BCM2835_SPI0_CS_TA | DISPLAY_SPI_DRIVE_SETTINGS; // Spi begins transfer

    // An SPI transfer to the display always starts with one control (command) byte, followed by N data bytes.
    clearGpio(GPIO_TFT_DATA_CONTROL);

    spi->fifo = cmd;
    while (!(spi->cs & (BCM2835_SPI0_CS_RXD | BCM2835_SPI0_CS_DONE))) /*nop*/
        ;

    if (payloadSize > 0) {
        setGpio(GPIO_TFT_DATA_CONTROL);

        tStart = payload;
        tEnd = payload + payloadSize;
        tPrefillEnd = tStart + (payloadSize > 15 ? 15 : payloadSize);

        while (tStart < tPrefillEnd)
            spi->fifo = *tStart++;
        while (tStart < tEnd) {
            cs = spi->cs;
            if ((cs & BCM2835_SPI0_CS_TXD))
                spi->fifo = *tStart++;
            if ((cs & (BCM2835_SPI0_CS_RXR | BCM2835_SPI0_CS_RXF)))
                spi->cs = BCM2835_SPI0_CS_CLEAR_RX | BCM2835_SPI0_CS_TA | DISPLAY_SPI_DRIVE_SETTINGS;
        }
    }
#endif

    WaitForPolledSPITransferToFinish();
}

void sendCmdOnly(uint8_t cmd)
{
    sendCmd(cmd, NULL, 0);
}

void sendCmdData(uint8_t cmd, uint8_t data)
{
    sendCmd(cmd, &data, 1);
}

int InitSPI()
{
#ifdef HAS_BCM
    // Memory map GPIO and SPI peripherals for direct access
    mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd < 0) {
        fprintf(stderr, "can't open /dev/mem (run as sudo)\n");
        return -1;
    }

    printf("bcm_host_get_peripheral_address: %p, bcm_host_get_peripheral_size: %u, bcm_host_get_sdram_address: %p\n", bcm_host_get_peripheral_address(), bcm_host_get_peripheral_size(), bcm_host_get_sdram_address());
    bcm2835 = mmap(NULL, bcm_host_get_peripheral_size(), (PROT_READ | PROT_WRITE), MAP_SHARED, mem_fd, bcm_host_get_peripheral_address());
    if (bcm2835 == MAP_FAILED) {
        fprintf(stderr, "mapping /dev/mem failed\n");
        return -1;
    }
    spi = (volatile SPIRegisterFile*)((uintptr_t)bcm2835 + BCM2835_SPI0_BASE);
    gpio = (volatile GPIORegisterFile*)((uintptr_t)bcm2835 + BCM2835_GPIO_BASE);

    gpioSetMode(GPIO_TFT_DATA_CONTROL, 0x01); // Data/Control pin to output (0x01)
    gpioSetMode(GPIO_SPI0_MOSI, 0x04);
    gpioSetMode(GPIO_SPI0_CLK, 0x04);

    spi->cs = BCM2835_SPI0_CS_CLEAR | DISPLAY_SPI_DRIVE_SETTINGS; // Initialize the Control and Status register to defaults: CS=0 (Chip Select), CPHA=0 (Clock Phase), CPOL=0 (Clock Polarity), CSPOL=0 (Chip Select Polarity), TA=0 (Transfer not active), and reset TX and RX queues.
    spi->clk = SPI_BUS_CLOCK_DIVISOR; // Clock Divider determines SPI bus speed, resulting speed=256MHz/clk

    // Enable fast 8 clocks per byte transfer mode, instead of slower 9 clocks per byte.
    // Errata to BCM2835 behavior: documentation states that the SPI0 DLEN register is only used for DMA. However, even when DMA is not being utilized, setting it from
    // a value != 0 or 1 gets rid of an excess idle clock cycle that is present when transmitting each byte. (by default in Polled SPI Mode each 8 bits transfer in 9 clocks)
    // With DLEN=2 each byte is clocked to the bus in 8 cycles, observed to improve max throughput from 56.8mbps to 63.3mbps (+11.4%, quite close to the theoretical +12.5%)
    // https://www.raspberrypi.org/forums/viewtopic.php?f=44&t=181154
    spi->dlen = 2;
#endif

    return 0;
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

void InitSPIDisplay()
{
    uint8_t madctl = 0;
    uint8_t data[4] = { 0, 0, (uint8_t)(240 >> 8), (uint8_t)(240 & 0xFF) };

    printf("Resetting display at reset GPIO pin %d\n", GPIO_TFT_RESET_PIN);
    gpioSetMode(GPIO_TFT_RESET_PIN, 1);
    setGpio(GPIO_TFT_RESET_PIN);
    usleep(120 * 1000);
    clearGpio(GPIO_TFT_RESET_PIN);
    usleep(120 * 1000);
    setGpio(GPIO_TFT_RESET_PIN);
    usleep(120 * 1000);

    printf("Initializing SPI bus\n");

    // Do the initialization with a very low SPI bus speed, so that it will succeed even if the bus speed chosen by the user is too high.
    spi->clk = 34;
    __sync_synchronize();

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
    // The ST7789 controller is actually a unit with 320x240 graphics memory area, but only 240x240 portion
    // of it is displayed. Therefore if we wanted to swap row address mode above, writes to Y=0...239 range will actually land in
    // memory in row addresses Y = 319-(0...239) = 319...80 range. To view this range, we must scroll the view by +80 units in Y
    // direction so that contents of Y=80...319 is displayed instead of Y=0...239.
    if ((madctl & MADCTL_ROW_ADDRESS_ORDER_SWAP)) {
        sendCmd(0x37 /*VSCSAD: Vertical Scroll Start Address of RAM*/, data, 4);
    }

    // drawFillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0); // clear screen
    // uint16_t randomColor = rand() % 0xFFFFFF;
    // drawFillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, randomColor); // clear screen
    drawFillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0xFF00FF); // clear screen

    for (int i = 0; i < 100; i++) {
        drawPixel(i, i, 0xFFFF00);
    }

    gpioSetMode(GPIO_TFT_BACKLIGHT, 1);
    setGpio(GPIO_TFT_BACKLIGHT);

    sendCmdOnly(/*Display ON*/ 0x29);
    usleep(100 * 1000);

    //     // And speed up to the desired operation speed finally after init is done.
    usleep(10 * 1000); // Delay a bit before restoring CLK, or otherwise this has been observed to cause the display not init if done back to back after the clear operation above.
    spi->clk = SPI_BUS_CLOCK_DIVISOR;
}

// #define ST7789_SURFACE "_SDL_DummySurface"

// int SDL_ST7789_CreateWindowFramebuffer(_THIS, SDL_Window *window, Uint32 *format, void **pixels, int *pitch)
// {
//     SDL_Surface *surface;
//     const Uint32 surface_format = SDL_PIXELFORMAT_RGB888;
//     int w, h;

//     InitSPI();

//     printf("Initializing display\n");
//     InitSPIDisplay();

//     /* Free the old framebuffer surface */
//     SDL_ST7789_DestroyWindowFramebuffer(_this, window);

//     /* Create a new one */
//     SDL_GetWindowSizeInPixels(window, &w, &h);
//     surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 0, surface_format);
//     if (!surface) {
//         return -1;
//     }

//     // SDL_PIXELFORMAT_RGB888 => SDL_PIXELFORMAT_XRGB8888
//     printf("............... Creating ST7789 window framebuffer w %d h %d BytesPerPixel %d format %d %s\n", w, h, surface->format->BytesPerPixel, surface->format->format, surface->format->palette ? "palette" : "no palette");

//     /* Save the info and return! */
//     SDL_SetWindowData(window, ST7789_SURFACE, surface);
//     *format = surface_format;
//     *pixels = surface->pixels;
//     *pitch = surface->pitch;
//     return 0;
// }

// int SDL_ST7789_UpdateWindowFramebuffer(_THIS, SDL_Window *window, const SDL_Rect *rects, int numrects)
// {
//     SDL_Surface *surface;
//     int x, y, w, h;
//     int pos;
//     uint8_t *pixels;
//     uint16_t rgb;
//     uint16_t size;
//     uint8_t pixelsBuffer[2048];

//     surface = (SDL_Surface *)SDL_GetWindowData(window, ST7789_SURFACE);
//     if (!surface) {
//         return SDL_SetError("Couldn't find st7789 surface for window");
//     }

//     w = surface->w > DISPLAY_WIDTH ? DISPLAY_WIDTH : surface->w;
//     h = surface->h > DISPLAY_HEIGHT ? DISPLAY_HEIGHT : surface->h;

//     size = w * BYTESPERPIXEL;
//     if (size > sizeof(pixelsBuffer)) {
//         return SDL_SetError("pixels buffer too small");
//     }

//     // draw row by row
//     for (x = 0; x < w; x++) {
//         for (y = 0; y < h; y++) {
//             pos = (y * surface->w + x) * surface->format->BytesPerPixel;
//             pixels = surface->pixels + pos;

//             // rgb = (((pixels[0] >> 3) << 11) | ((pixels[1] >> 2) << 5) | (pixels[2] >> 3));
//             rgb = ((pixels[0] & 0xF8) << 8) | ((pixels[1] & 0xFC) << 3) | (pixels[2] >> 3);
//             pixelsBuffer[(y * BYTESPERPIXEL)] = (uint8_t)(rgb >> 8);
//             pixelsBuffer[(y * BYTESPERPIXEL) + 1] = (uint8_t)(rgb & 0xFF);
//         }

//         // Let's rotate 90 degrees
//         sendAddr(DISPLAY_SET_CURSOR_Y, (uint16_t)x, (uint16_t)x);
//         sendAddr(DISPLAY_SET_CURSOR_X, (uint16_t)0, (uint16_t)h - 1);
//         sendCmd(DISPLAY_WRITE_PIXELS, pixelsBuffer, size);
//     }

//     return 0;
// }

// void SDL_ST7789_DestroyWindowFramebuffer(_THIS, SDL_Window *window)
// {
//     SDL_Surface *surface;

//     surface = (SDL_Surface *)SDL_SetWindowData(window, ST7789_SURFACE, NULL);
//     SDL_FreeSurface(surface);
// }

// void SDL_ST7789_VideoQuit(_THIS)
// {
//     printf("st7789 quit\n");
//     sendCmdOnly(/*Display OFF*/ 0x28);
//     clearGpio(GPIO_TFT_BACKLIGHT);
//     DeinitSPI();
// }

int main(int argc, char* argv[])
{
    InitSPI();
    InitSPIDisplay();
    return 0;
}