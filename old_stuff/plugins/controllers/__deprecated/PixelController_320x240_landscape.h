/** Description:
This C++ header file defines the `PixelController`, a specialized component designed to bridge physical hardware interactions with the application's software logic.

The core function of this controller is to manage and interpret user inputs coming from physical buttons and rotary dials (encoders) connected directly to the device’s input/output pins (GPIOs). It builds upon a standard blueprint provided by the `ControllerInterface`.

Internally, the `PixelController` relies on two dedicated input management systems:

1.  **Button Manager (`GpioKey`):** This system monitors an array of physical buttons. It defines which specific hardware pin corresponds to a functional input (like pressing a keyboard key, such as 'q' or 'a'). When a button is pressed or released, this manager immediately triggers a specific routine (a "callback") to translate that physical event into a standardized software message.
2.  **Encoder Manager (`GpioEncoder`):** This system monitors four independent rotary dials. It tracks the rotation of these dials and reports the direction of movement (clockwise or counter-clockwise) along with the precise time the action occurred.

When the `PixelController` is initialized, it sets up both the button and encoder systems. Crucially, it starts dedicated background processes, or "threads," for each manager. This ensures that both the buttons and the rotary dials are continuously monitored in real-time, allowing the application to react instantly and reliably to any user input from the physical hardware.

sha: 1bc83c2ecafc7e7e990f0bbd299e72f489fe4286c30672fb2cdfe3c744417c40 
*/
#ifndef _PIXEL_CONTROLLER_H_
#define _PIXEL_CONTROLLER_H_

#include "controllerInterface.h"
#include "helpers/GpioEncoder.h"
#include "helpers/GpioKey.h"
#include "helpers/getTicks.h"

class PixelController : public ControllerInterface {
protected:
    GpioKey gpioKey = GpioKey({
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
        [this](GpioKey::Key key, uint8_t state) {
            //   printf("gpio%d key [%d] state changed %d\n", key.gpio, key.key, state);
            onKey(id, key.key, state);
        });

    GpioEncoder gpioEncoder = GpioEncoder({
                                              { 0, 20, 26 }, // pin 38 = gpio 20, pin 37 = gpio 26
                                              { 1, 6, 5 }, // pin 31 = gpio 6, pin 29 = gpio 5
                                              { 2, 0, 1 }, // pin 27 = gpio 0, pin 28 = gpio 1
                                              { 3, 4, 27 }, // pin 7 = gpio 4, pin 13 = gpio 27
                                          },
        [this](GpioEncoder::Encoder enc, int8_t direction) {
            // printf("encoder%d gpioA%d gpioB%d direction %d\n", encoder.id, encoder.gpioA, encoder.gpioB, direction);
            encoder(enc.id, direction, getTicks());
        });

public:
    PixelController(Props& props, uint16_t id)
        : ControllerInterface(props, id)
    {
        if (gpioKey.init() == 0) {
            gpioKey.startThread(); // might want to use the same thread for encoder...
        }
        if (gpioEncoder.init() == 0) {
            gpioEncoder.startThread(); // might want to use the same thread for encoder...
        }
    }
};

#endif