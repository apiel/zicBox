#ifndef _HELPER_GPIO_H_
#define _HELPER_GPIO_H_

#include <fcntl.h> // open, O_RDWR, O_SYNC
#include <inttypes.h>
#include <stdio.h> // printf, stderr
#include <string.h> // memset
#include <sys/ioctl.h>
#include <sys/mman.h> // mmap, munmap
#include <unistd.h> // usleep

#define BLOCK_SIZE (4 * 1024)

#define GPIO_BASE 0x00200000 // 0_00200000
// #define GPIO_BASE 0x200000

// read
#define GPIO_INPUT 0x00
// write
#define GPIO_OUTPUT 0x01
// alt
#define GPIO_ALT0 0x04

// typedef struct GPIORegisterFile {
//     uint32_t gpfsel[6], reserved0; // GPIO Function Select registers, 3 bits per pin, 10 pins in an uint32_t
//     uint32_t gpset[2], reserved1; // GPIO Pin Output Set registers, write a 1 to bit at index I to set the pin at index I high
//     uint32_t gpclr[2], reserved2; // GPIO Pin Output Clear registers, write a 1 to bit at index I to set the pin at index I low
//     uint32_t gplev[2];
// } GPIORegisterFile;

#define GPIO_PUD_OFF 0x0  // No pull-up or pull-down
#define GPIO_PUD_DOWN 0x1 // Pull-down
#define GPIO_PUD_UP 0x2   // Pull-up

typedef struct GPIORegisterFile {
    uint32_t gpfsel[6], reserved0;
    uint32_t gpset[2], reserved1;
    uint32_t gpclr[2], reserved2;
    uint32_t gplev[2];
    uint32_t gpeds[2];
    uint32_t gpren[2];
    uint32_t gpfen[2];
    uint32_t gphen[2];
    uint32_t gplen[2];
    uint32_t gparen[2];
    uint32_t gpafen[2];
    uint32_t gppud;       // GPIO Pull-up/down Register
    uint32_t gppudclk[2]; // GPIO Pull-up/down Clock Register
} GPIORegisterFile;
volatile GPIORegisterFile* memgpio = 0;

void gpioSetMode(uint8_t pin, uint8_t mode)
{
    memgpio->gpfsel[(pin) / 10] = (memgpio->gpfsel[(pin) / 10] & ~(0x7 << ((pin) % 10) * 3)) | ((mode) << ((pin) % 10) * 3);
}

void setGpio(uint8_t pin)
{
    memgpio->gpset[0] = 1 << (pin); // Pin must be (0-31)
}

void clearGpio(uint8_t pin)
{
    memgpio->gpclr[0] = 1 << (pin); // Pin must be (0-31)
}

void gpioWrite(uint8_t pin, uint8_t value) { value ? setGpio(pin) : clearGpio(pin); }

uint8_t gpioRead(uint8_t gpio)
{
    if ((memgpio->gplev[gpio >> 5] & 1 << (gpio & 0x1F)) != 0)
        return 0;
    else
        return 1;
}

int initGpio()
{
    if (memgpio == 0) {
        int gpiomem_fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
        if (gpiomem_fd < 0) {
            // https://raspberrypi.stackexchange.com/questions/40105/access-gpio-pins-without-root-no-access-to-dev-mem-try-running-as-root
            fprintf(stderr, "can't open /dev/gpiomem\n");
            return -1;
        }

        memgpio = (volatile GPIORegisterFile*)mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, gpiomem_fd, GPIO_BASE);
        if (memgpio == MAP_FAILED) {
            fprintf(stderr, "mmap (GPIO) failed\n");
            return -1;
        }
    }
    return 0;
}

void gpioSetPullUp(uint8_t gpio)
{
    // Step 1: Write to GPPUD to set the desired control signal (pull-up)
    memgpio->gppud = GPIO_PUD_UP;

    // Step 2: Wait 150 cycles for control signal to set up (arbitrary delay)
    usleep(1); // At least 150 cycles on Raspberry Pi hardware

    // Step 3: Write to GPPUDCLK to clock the control signal into the specified GPIO pin
    memgpio->gppudclk[gpio >> 5] = 1 << (gpio & 0x1F);

    // Step 4: Wait 150 cycles to ensure clock signal is registered
    usleep(1);

    // Step 5: Clear the GPPUD and GPPUDCLK to remove the signal
    memgpio->gppud = GPIO_PUD_OFF;
    memgpio->gppudclk[gpio >> 5] = 0;
}

#endif