/** Description:
This C++ program serves as a real-time interface for a custom hardware controller, designed to read signals from physical buttons and rotary dials connected to a computer’s input/output pins (GPIO).

The code initializes two primary systems to handle these inputs concurrently:

1.  **GpioKey (Button Handler):** This section maps specific physical input pins (like pins 12, 24, and 27) to designated digital actions, primarily mimicking standard keyboard presses ('q', 'w', 'e', 'a', 's', 'd'). It also includes separate mappings for four push-button inputs ('0' through '3'). When a button is pressed, the program immediately triggers a "callback," which in this case involves printing a message detailing which pin and key were activated.

2.  **GpioEncoder (Rotary Knob Handler):** This part monitors pairs of input pins to track movement on four separate rotary dials (encoders). It constantly checks whether a dial is turned clockwise or counter-clockwise. When rotation occurs, the program reports the specific dial ID and the detected direction.

Both systems are set up to run continuously in the background using separate monitoring processes (known as threads). This structure allows the program to remain responsive, ensuring that every physical button press or dial turn is instantly detected and processed as a digital command, effectively turning the physical device into a functional input controller.

sha: 1e143e7f39d3e1e3f066d86de0477f8a752fa0a2630a2df76482fa3c7733fbd8 
*/
// g++ pixelController.cpp -o pixelController -I../.. -pthread && mv ./pixelController ~/pixelController && ~/pixelController

// g++ pixelController.cpp -o pixelControllerPigpio -I../.. -pthread -lpigpio -lrt -DPIGPIO=1 && mv ./pixelControllerPigpio ~/pixelControllerPigpio && sudo ~/pixelControllerPigpio

// could use `pinctrl set 17 pu`

// #define DEBUG_GPIO_KEY 1
// #define DEBUG_GPIO_ENCODER 1

#include "helpers/GpioEncoder.h"
#include "helpers/GpioKey.h"

#include <unistd.h>

int main(int argc, char* argv[])
{
    GpioKey gpioKey({
                        { 12, getKeyCode("'q'") }, // pin 32 = gpio 12
                        { 24, getKeyCode("'w'") }, // pin 18 = gpio 24
                        { 27, getKeyCode("'e'") }, // pin 13 = gpio 27
                        { 8, getKeyCode("'a'") }, // pin 24 = gpio 8
                        { 25, getKeyCode("'s'") }, // pin 22 = gpio 25
                        { 23, getKeyCode("'d'") }, // pin 16 = gpio 23

                        { 0, getKeyCode("'0'") }, // pin 27 = gpio 0   <-- push encoder 1
                        { 5, getKeyCode("'1'") }, // pin 29 = gpio 5   <-- push encoder 2
                        { 16, getKeyCode("'2'") }, // pin 36 = gpio 16 <-- push encoder 3
                        { 14, getKeyCode("'3'") }, // pin 8 = gpio 14  <-- push encoder 4
                    },
        [](GpioKey::Key key, uint8_t state) {
              printf("gpio%d key [%d] state changed %d\n", key.gpio, key.key, state);
        });

    if (gpioKey.init() == -1) {
        return -1;
    }
    gpioKey.startThread();

    GpioEncoder gpioEncoder({
                                { 0, 26, 13 }, // pin 37 = gpio 26, pin 33 = gpio 13
                                { 1, 7, 9 }, // pin 26 = gpio 7, pin 21 = gpio 9
                                { 2, 20, 6 }, // pin 38 = gpio 20, pin 31 = gpio 6
                                { 3, 15, 4 }, // pin 10 = gpio 15, pin 7 = gpio 4
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