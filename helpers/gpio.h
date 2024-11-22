#ifndef _HELPER_GPIO_H_
#define _HELPER_GPIO_H_

#include <fcntl.h> // open, O_RDWR, O_SYNC
#include <inttypes.h>
#include <stdio.h> // printf, stderr
#include <string.h> // memset
#include <sys/ioctl.h>
#include <sys/mman.h> // mmap, munmap

#define BLOCK_SIZE (4 * 1024)

#define GPIO_BASE 0x00200000 // 0_00200000
// #define GPIO_BASE 0x200000

#define GPIO_INPUT 0x00
#define GPIO_OUTPUT 0x01
#define GPIO_ALT0 0x04

typedef struct GPIORegisterFile {
    uint32_t gpfsel[6], reserved0; // GPIO Function Select registers, 3 bits per pin, 10 pins in an uint32_t
    uint32_t gpset[2], reserved1; // GPIO Pin Output Set registers, write a 1 to bit at index I to set the pin at index I high
    uint32_t gpclr[2], reserved2; // GPIO Pin Output Clear registers, write a 1 to bit at index I to set the pin at index I low
    uint32_t gplev[2];
} GPIORegisterFile;
volatile GPIORegisterFile* memgpio = 0;

void setGpioMode(uint8_t pin, uint8_t mode)
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

void setGpio(uint8_t pin, uint8_t value) { value ? setGpio(pin) : clearGpio(pin); }

uint8_t getGpio(uint8_t pin) { return memgpio->gplev[0] & (1 << (pin)); }

int initGpio()
{
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
    return 0;
}

#endif