// g++ pixelController.cpp -o pixelController -I../.. -pthread && mv ./pixelController ~/pixelController && ~/pixelController

// g++ pixelController.cpp -o pixelControllerPigpio -I../.. -pthread -lpigpio -lrt -DPIGPIO=1 && mv ./pixelControllerPigpio ~/pixelControllerPigpio && sudo ~/pixelControllerPigpio

#include "../../helpers/GpioKey.h"

#include <unistd.h>

int main(int argc, char* argv[])
{
    GpioKey gpioKey({
                        { 13, getKeyCode("'q'") }, // pin 33 = gpio 13
                        { 7, getKeyCode("'w'") }, // pin 26 = gpio 7
                        { 25, getKeyCode("'e'") }, // pin 22 = gpio 25
                        { 23, getKeyCode("'r'") }, // pin 16 = gpio 23
                        { 14, getKeyCode("'t'") }, // pin 8 = gpio 14
                        { 16, getKeyCode("'a'") }, // pin 36 = gpio 16
                        { 12, getKeyCode("'s'") }, // pin 32 = gpio 12
                        { 8, getKeyCode("'d'") }, // pin 24 = gpio 8
                        { 24, getKeyCode("'f'") }, // pin 18 = gpio 24
                        { 15, getKeyCode("'g'") }, // pin 10 = gpio 15
                    },
        [](GpioKey::Key key, uint8_t state) {
            //   printf("gpio%d key [%d] state changed %d\n", key.gpio, key.key, state);
        });

    if (gpioKey.init() == -1) {
        return -1;
    }
    gpioKey.startThread();

    while (true) {
        usleep(1000);
    }
    return 0;
}