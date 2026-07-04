/** Description:
This program is a utility designed to interact directly with the physical hardware pins, known as General Purpose Input/Output (GPIO), on an embedded system or computer. Think of these GPIO pins as digital switches that can either report a status (Input) or be used to control an external device (Output).

The program has two main modes of operation, determined by the information provided when it is launched from the command line:

### 1. Reading Hardware Status (Input Mode)

If the user provides only the number of the specific hardware pin they are interested in, the program switches that pin into "Input" mode. It then reads the current electrical voltage on that pin to determine its status—whether it is electrically active ("ON," usually represented by 1) or inactive ("OFF," represented by 0). This is useful for monitoring sensors or detecting external signals.

### 2. Controlling Hardware (Output Mode)

If the user provides both the pin number and a desired state (0 for OFF or 1 for ON), the program switches the pin into "Output" mode. It then forcibly sets the electrical state of that pin to the requested value. This action is used to control connected devices, such as turning an LED light on or off.

In summary, this small C++ application acts as a straightforward terminal interface, allowing users to either monitor the status of a specific hardware pin or actively control its electrical state based on the arguments supplied at runtime.

sha: 063fe46e676a5b48b60817dbd1ed798602ccfbb4ce18739a416d8a566905b6b6 
*/
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
