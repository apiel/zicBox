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