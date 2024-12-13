#ifndef _PIXEL_CONTROLLER_H_
#define _PIXEL_CONTROLLER_H_

#include "controllerInterface.h"
#include "helpers/GpioKey.h"

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
                              },
        [this](GpioKey::Key key, uint8_t state) {
            //   printf("gpio%d key [%d] state changed %d\n", key.gpio, key.key, state);
            onKey(id, key.key, state);
        });

public:
    PixelController(Props& props, uint16_t id)
        : ControllerInterface(props, id)
    {
        gpioKey.init();
        gpioKey.startThread(); // might want to use the same thread for encoder...
    }
};

#endif