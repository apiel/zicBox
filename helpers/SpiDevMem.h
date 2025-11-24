/** Description:
This C++ header file acts as a high-performance software driver designed specifically to control the Serial Peripheral Interface (SPI) hardware on a Raspberry Pi.

**Core Purpose:**
The main goal of this code is to establish extremely fast, direct communication between the software and an external device (like a display screen or sensor) connected via the Raspberry Pi's SPI pins.

**How It Works (Direct Hardware Access):**

1.  **Bypassing the OS:** Instead of relying on the operating system’s slower standard input/output methods, this driver uses a technique called "memory mapping." This allows the program to open a special system file (`/dev/mem`) and treat the physical control registers of the SPI hardware as if they were simple variables in the program’s memory. This requires special permissions (running as root/sudo).

2.  **Initialization (`init`):** The driver first maps the necessary hardware addresses and sets up the corresponding GPIO pins for SPI use. It configures the SPI chip with parameters like clock speed, polarity, and timing, optimizing the transfer rate for maximum speed.

3.  **Data Transfer (`sendCmd`):** The core function manages the transmission of information. It first sets a specific control pin to signal whether the next bytes are a "command" or actual "data." It then efficiently pushes the information into the hardware's small internal holding areas (called FIFOs), continuously checking status flags to ensure the hardware is ready to receive the next chunk, guaranteeing a smooth and fast transfer process.

In essence, this file provides a fast, low-level bridge for sending precise digital signals directly to specialized electronics.

sha: fd0994141c1a167c4bd035c6cad3848b3ed25886a49aee9a68ecd4d7f04effca 
*/
#pragma once

// // sudo apt-get install libbcm2835-dev

// Need to be root to access /dev/mem
// build with -lbcm2835 -lbcm_host

#include <fcntl.h> // open, O_RDWR, O_SYNC
#include <inttypes.h>
#include <stdio.h> // printf, stderr
#include <sys/mman.h> // mmap, munmap

#include "../helpers/gpio.h"

#include <bcm_host.h> // bcm_host_get_peripheral_address, bcm_host_get_peripheral_size, bcm_host_get_sdram_address

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

volatile void* bcm2835 = 0;

typedef struct SPIRegisterFile {
    uint32_t cs; // SPI Master Control and Status register
    uint32_t fifo; // SPI Master TX and RX FIFOs
    uint32_t clk; // SPI Master Clock Divider
    uint32_t dlen; // SPI Master Number of DMA Bytes to Write
} SPIRegisterFile;
volatile SPIRegisterFile* spi;

class Spi {
protected:
    int fd = -1;

    uint8_t gpioDataControl;

    void WaitForPolledSPITransferToFinish()
    {
        uint32_t cs;
        while (!(((cs = spi->cs) ^ BCM2835_SPI0_CS_TA) & (BCM2835_SPI0_CS_DONE | BCM2835_SPI0_CS_TA))) // While TA=1 and DONE=0
            if ((cs & (BCM2835_SPI0_CS_RXR | BCM2835_SPI0_CS_RXF)))
                spi->cs = BCM2835_SPI0_CS_CLEAR_RX | BCM2835_SPI0_CS_TA | DISPLAY_SPI_DRIVE_SETTINGS;

        if ((cs & BCM2835_SPI0_CS_RXD))
            spi->cs = BCM2835_SPI0_CS_CLEAR_RX | BCM2835_SPI0_CS_TA | DISPLAY_SPI_DRIVE_SETTINGS;
    }

public:
    Spi(uint8_t gpioDataControl)
        : gpioDataControl(gpioDataControl)
    {
    }

    int init()
    {
        // Memory map GPIO and SPI peripherals for direct access
        fd = open("/dev/mem", O_RDWR | O_SYNC);
        if (fd < 0) {
            fprintf(stderr, "can't open /dev/mem (run as sudo)\n");
            return -1;
        }

        printf("bcm_host_get_peripheral_address: %p, bcm_host_get_peripheral_size: %u, bcm_host_get_sdram_address: %p\n", bcm_host_get_peripheral_address(), bcm_host_get_peripheral_size(), bcm_host_get_sdram_address());
        bcm2835 = mmap(NULL, bcm_host_get_peripheral_size(), (PROT_READ | PROT_WRITE), MAP_SHARED, fd, bcm_host_get_peripheral_address());
        if (bcm2835 == MAP_FAILED) {
            fprintf(stderr, "mapping /dev/mem failed\n");
            return -1;
        }
        spi = (volatile SPIRegisterFile*)((uintptr_t)bcm2835 + BCM2835_SPI0_BASE);

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

        return 0;
    }

    void sendCmd(uint8_t cmd, uint8_t* payload, uint32_t payloadSize)
    {
        uint8_t* tStart;
        uint8_t* tEnd;
        uint8_t* tPrefillEnd;
        uint32_t cs;

        spi->cs = BCM2835_SPI0_CS_TA | DISPLAY_SPI_DRIVE_SETTINGS; // Spi begins transfer

        // An SPI transfer to the display always starts with one control (command) byte, followed by N data bytes.
        gpioWrite(gpioDataControl, 0);

        spi->fifo = cmd;
        while (!(spi->cs & (BCM2835_SPI0_CS_RXD | BCM2835_SPI0_CS_DONE))) /*nop*/
            ;

        if (payloadSize > 0) {
            gpioWrite(gpioDataControl, 1);

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

        WaitForPolledSPITransferToFinish();
    }

    void setSpeed(uint32_t speed)
    {
        spi->clk = speed;
        // __sync_synchronize();
    }
};
