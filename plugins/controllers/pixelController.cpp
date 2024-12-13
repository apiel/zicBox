// g++ pixelController.cpp -o pixelController -I../.. -pthread && mv ./pixelController ~/pixelController && ~/pixelController

// g++ pixelController.cpp -o pixelControllerPigpio -I../.. -pthread -lpigpio -lrt -DPIGPIO=1 && mv ./pixelControllerPigpio ~/pixelControllerPigpio && sudo ~/pixelControllerPigpio

// could use `pinctrl set 17 pu`

// #define DEBUG_GPIO_KEY 1
// #define DEBUG_GPIO_ENCODER 1

#include "../../helpers/GpioEncoder.h"
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

                        { 9, getKeyCode("'3'") }, // pin 21 = gpio 9 <-- push encoder 3
                    },
        [](GpioKey::Key key, uint8_t state) {
              printf("gpio%d key [%d] state changed %d\n", key.gpio, key.key, state);
        });

    if (gpioKey.init() == -1) {
        return -1;
    }
    gpioKey.startThread();

    GpioEncoder gpioEncoder({
                                { 0, 20, 26 }, // pin 38 = gpio 20, pin 37 = gpio 26
                                { 1, 6, 5 }, // pin 31 = gpio 6, pin 29 = gpio 5
                                { 2, 0, 1 }, // pin 27 = gpio 0, pin 28 = gpio 1
                                { 3, 4, 27 }, // pin 7 = gpio 4, pin 13 = gpio 27
                            },
        [](GpioEncoder::Encoder encoder, int8_t direction) {
            printf("encoder%d gpioA%d gpioB%d direction %d\n", encoder.id, encoder.gpioA, encoder.gpioB, direction);
        });

    if (gpioEncoder.init() == -1) {
        return -1;
    }
    gpioEncoder.startThread();

    while (true) {
        usleep(1000);
    }
    return 0;
}