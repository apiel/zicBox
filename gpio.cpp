#include "helpers/gpio.h"

// ../zicOs/buildroot/output/host/bin/aarch64-buildroot-linux-gnu-g++ -g -fms-extensions -o build/arm/screenOff screenOff.cpp -fpermissive -I.

int main(int argc, char* argv[])
{
    initGpio();

    if (argc < 2) {
        logDebug("Usage read: %s <gpio number>", argv[0]);
        logDebug("       set: %s <gpio number> <gpio state: 0 or 1>", argv[0]);
        return 0;
    }

    int gpioNumber = atoi(argv[1]);

    if (argc < 3) {
        gpioSetMode(gpioNumber, GPIO_INPUT);
        uint8_t value = gpioRead(gpioNumber);
        logDebug("GPIO %d is %s (%d)", gpioNumber, value ? "ON" : "OFF", value);
    } else {
        int gpioState = atoi(argv[2]);
        gpioSetMode(gpioNumber, GPIO_OUTPUT);
        gpioWrite(gpioNumber, gpioState);
    }
    return 0;
}
