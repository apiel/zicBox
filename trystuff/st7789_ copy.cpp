// sudo chown 0:0 test2
// sudo chmod u+s test2

// sudo apt-get install libraspberrypi-dev raspberrypi-kernel-headers
// cp ../remoteZicBox/st7789.cpp . && g++ -o test2 st7789.cpp -lbcm2835 -lrt -DBCM2835=1 -fpermissive -lbcm_host && sudo ./test2

// g++ -o test2 st7789_.cpp -lbcm2835 -lrt -DBCM2835=1 -fpermissive -lbcm_host && mv test2 ~/. && sudo ~/test2

// https://raspberrypi.stackexchange.com/questions/40105/access-gpio-pins-without-root-no-access-to-dev-mem-try-running-as-root

#include <fcntl.h> // open, O_RDWR, O_SYNC
#include <inttypes.h>
#include <stdio.h> // printf, stderr
#include <sys/mman.h> // mmap, munmap
#include <unistd.h> // usleep
#include "../helpers/gpio.h"
#include "../helpers/st7789.h"

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

#define SPI_BUS_CLOCK_DIVISOR 20

#define BCM2835_SPI0_BASE 0x204000 // Address to SPI0 register file

#define BCM2835_SPI0_CS_RXF 0x00100000 // Receive FIFO is full
#define BCM2835_SPI0_CS_RXR 0x00080000 // FIFO needs reading
#define BCM2835_SPI0_CS_TXD 0x00040000 // TXD TX FIFO can accept Data
#define BCM2835_SPI0_CS_RXD 0x00020000 // RXD RX FIFO contains Data
#define BCM2835_SPI0_CS_DONE 0x00010000 // Done transfer Done
#define BCM2835_SPI0_CS_TA 0x00000080 // Transfer Active
#define BCM2835_SPI0_CS_CLEAR 0x00000030 // Clear FIFO Clear RX and TX
#define BCM2835_SPI0_CS_CLEAR_RX 0x00000020 // Clear FIFO Clear RX
#define BCM2835_SPI0_CS_CLEAR_TX 0x00000010 // Clear FIFO Clear TX
#define BCM2835_SPI0_CS_CPOL 0x00000008 // Clock Polarity
#define BCM2835_SPI0_CS_CPHA 0x00000004 // Clock Phase

#define GPIO_SPI0_MOSI 10 // Pin P1-19, MOSI when SPI0 in use
#define GPIO_SPI0_CLK 11 // Pin P1-23, CLK when SPI0 in use

#define DISPLAY_SPI_DRIVE_SETTINGS (1 | BCM2835_SPI0_CS_CPOL | BCM2835_SPI0_CS_CPHA)

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

    spi->cs = BCM2835_SPI0_CS_TA | DISPLAY_SPI_DRIVE_SETTINGS; // Spi begins transfer

    // An SPI transfer to the display always starts with one control (command) byte, followed by N data bytes.
    setGpio(GPIO_TFT_DATA_CONTROL, 0);

    spi->fifo = cmd;
    while (!(spi->cs & (BCM2835_SPI0_CS_RXD | BCM2835_SPI0_CS_DONE))) /*nop*/
        ;

    if (payloadSize > 0) {
        setGpio(GPIO_TFT_DATA_CONTROL, 1);

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

    setGpioMode(GPIO_TFT_DATA_CONTROL, 0x01); // Data/Control pin to output (0x01)
    setGpioMode(GPIO_SPI0_MOSI, 0x04);
    setGpioMode(GPIO_SPI0_CLK, 0x04);

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

void InitSPIDisplay()
{
    printf("Resetting display at reset GPIO pin %d\n", GPIO_TFT_RESET_PIN);
    setGpioMode(GPIO_TFT_RESET_PIN, 1);
    setGpio(GPIO_TFT_RESET_PIN, 1);
    usleep(120 * 1000);
    setGpio(GPIO_TFT_RESET_PIN, 0);
    usleep(120 * 1000);
    setGpio(GPIO_TFT_RESET_PIN, 1);
    usleep(120 * 1000);

    printf("Initializing SPI bus\n");

    // Do the initialization with a very low SPI bus speed, so that it will succeed even if the bus speed chosen by the user is too high.
    spi->clk = 34;
    __sync_synchronize();

    ST7789 st7789([](uint8_t cmd, uint8_t* data, uint32_t len) { sendCmd(cmd, data, len); }, 240, 240);
    st7789.init();

    //     // And speed up to the desired operation speed finally after init is done.
    usleep(10 * 1000); // Delay a bit before restoring CLK, or otherwise this has been observed to cause the display not init if done back to back after the clear operation above.
    spi->clk = SPI_BUS_CLOCK_DIVISOR;
}

int main(int argc, char* argv[])
{
    initGpio();
    InitSPI();
    InitSPIDisplay();
    return 0;
}
