// cp ../remoteZicBox/st7789_2.cpp . && g++ -o test23 st7789_2.cpp -lbcm2835 -lrt -DBCM2835=1 -fpermissive -lbcm_host && ./test23

#include <fcntl.h> // open, O_RDWR, O_SYNC
#include <inttypes.h>
#include <stdio.h> // printf, stderr
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

#define	BLOCK_SIZE		(4*1024)

#define GPIO_BASE 0x00200000 // 0_00200000 

int main(int argc, char* argv[])
{
    printf("Start spi display test.\n");
    int gpiomem_fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (gpiomem_fd < 0) {
        fprintf(stderr, "can't open /dev/gpiomem (fix permissions: https://raspberrypi.stackexchange.com/questions/40105/access-gpio-pins-without-root-no-access-to-dev-mem-try-running-as-root)\n");
        return -1;
    }

    printf("Nmap gpiomem...\n");
    // // bcm2835 = mmap(NULL, bcm_host_get_peripheral_size(), (PROT_READ | PROT_WRITE), MAP_SHARED, gpiomem_fd, bcm_host_get_peripheral_address());
    // // bcm2835 = mmap(NULL, 180, (PROT_READ | PROT_WRITE), MAP_SHARED, gpiomem_fd, 0);
    // bcm2835 = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, gpiomem_fd, 0x00200000);
    // if (bcm2835 == MAP_FAILED) {
    //     fprintf(stderr, "mapping /dev/mem failed\n");
    //     return -1;
    // }
    // // spi = (volatile SPIRegisterFile*)((uintptr_t)bcm2835 + BCM2835_SPI0_BASE);
    // gpio = (volatile GPIORegisterFile*)((uintptr_t)bcm2835 + BCM2835_GPIO_BASE);

    gpio = (volatile GPIORegisterFile*)mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, gpiomem_fd, GPIO_BASE);
    if (gpio == MAP_FAILED) {
        fprintf(stderr, "mmap (GPIO) failed\n");
        return -1;
    }

    printf("Test stuff\n");
    setGpioMode(GPIO_TFT_BACKLIGHT, 1);
    setGpio(GPIO_TFT_BACKLIGHT);
    usleep(100 * 1000);
    clearGpio(GPIO_TFT_BACKLIGHT);
    usleep(100 * 1000);
    setGpio(GPIO_TFT_BACKLIGHT);

    return 0;
}
