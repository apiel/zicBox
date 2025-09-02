#include "helpers/gpio.h"

// ../zicOs/buildroot/output/host/bin/aarch64-buildroot-linux-gnu-g++ -g -fms-extensions -o build/arm/screenOff screenOff.cpp -fpermissive -I.

#define GPIO_TFT_BACKLIGHT 3

int main(int argc, char* argv[])
{
    initGpio();

    logDebug("Switch Off on GPIO pin %d\n", GPIO_TFT_BACKLIGHT);
    gpioSetMode(GPIO_TFT_BACKLIGHT, 1);
    gpioWrite(GPIO_TFT_BACKLIGHT, 0);
    return 0;
}
