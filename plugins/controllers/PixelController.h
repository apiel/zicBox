#ifndef _PIXEL_CONTROLLER_H_
#define _PIXEL_CONTROLLER_H_

#include "controllerInterface.h"
#include "helpers/GpioEncoder.h"
#include "helpers/GpioKey.h"
#include "helpers/getTicks.h"

class PixelController : public ControllerInterface {
protected:
    GpioKey gpioKey = GpioKey({},
        [this](GpioKey::Key key, uint8_t state) {
            //   printf("gpio%d key [%d] state changed %d\n", key.gpio, key.key, state);
            onKey(id, key.key, state);
        });

    GpioEncoder gpioEncoder = GpioEncoder({},
        [this](GpioEncoder::Encoder enc, int8_t direction) {
            // printf("encoder%d gpioA%d gpioB%d direction %d\n", encoder.id, encoder.gpioA, encoder.gpioB, direction);
            encoder(enc.id, direction, getTicks());
        });

public:
    PixelController(Props& props, uint16_t id)
        : ControllerInterface(props, id)
    {
    }

    bool config(char* key, char* params) override
    {
        if (strcmp(key, "PIXEL_CONTROLLER") == 0) {
            if (strcmp(params, "rpi0_4enc_6btn") == 0) {
                gpioKey.keys = {
                    { 12, getKeyCode("'q'") }, // pin 32 = gpio 12
                    { 24, getKeyCode("'w'") }, // pin 18 = gpio 24
                    { 27, getKeyCode("'e'") }, // pin 13 = gpio 27
                    { 8, getKeyCode("'a'") }, // pin 24 = gpio 8
                    { 25, getKeyCode("'s'") }, // pin 22 = gpio 25
                    { 23, getKeyCode("'d'") }, // pin 16 = gpio 23

                    { 0, getKeyCode("'0'") }, // pin 27 = gpio 0   <-- push encoder 0
                    { 16, getKeyCode("'1'") }, // pin 36 = gpio 16 <-- push encoder 1
                    { 5, getKeyCode("'2'") }, // pin 29 = gpio 5   <-- push encoder 2
                    { 14, getKeyCode("'3'") }, // pin 8 = gpio 14  <-- push encoder 3
                };
                if (gpioKey.init() == 0) {
                    gpioKey.startThread(); // might want to use the same thread for encoder...
                }
                gpioEncoder.encoders = {
                    { 0, 26, 13 }, // pin 37 = gpio 26, pin 33 = gpio 13
                    { 1, 20, 6 }, // pin 38 = gpio 20, pin 31 = gpio 6
                    { 2, 7, 9 }, // pin 26 = gpio 7, pin 21 = gpio 9
                    { 3, 15, 4 }, // pin 10 = gpio 15, pin 7 = gpio 4
                };
                if (gpioEncoder.init() == 0) {
                    gpioEncoder.startThread(); // might want to use the same thread for encoder...
                }
            } else if (strcmp(params, "rpi3A_4enc_11btn") == 0) {
                gpioKey.keys = {
                    { 27, getKeyCode("shift") }, // pin 13 = gpio 27
                    { 4, getKeyCode("up") }, // pin 7 = gpio 4
                    { 14, getKeyCode("down") }, // pin 8 = gpio 14

                    { 16, getKeyCode("'q'") }, // pin 36 = gpio 16
                    { 1, getKeyCode("'w'") }, // pin 28 = gpio 1
                    { 7, getKeyCode("'e'") }, // pin 26 = gpio 7
                    { 15, getKeyCode("'r'") }, // pin 10 = gpio 15

                    { 12, getKeyCode("'a'") }, // pin 32 = gpio 12
                    { 5, getKeyCode("'s'") }, // pin 29 = gpio 5
                    { 0, getKeyCode("'d'") }, // pin 27 = gpio 0
                    { 8, getKeyCode("'f'") }, // pin 24 = gpio 8

                };
                if (gpioKey.init() == 0) {
                    gpioKey.startThread(); // might want to use the same thread for encoder...
                }
                gpioEncoder.encoders = {
                    { 0, 13, 6 }, // gpio 13, gpio 6
                    { 1, 24, 23 }, // gpio 24, gpio 23
                    { 2, 20, 26 }, // gpio 20, gpio 26
                    { 3, 9, 25 }, // gpio 9, gpio 25
                };
                if (gpioEncoder.init() == 0) {
                    gpioEncoder.startThread(); // might want to use the same thread for encoder...
                }
            }
            return true;
        }
        return false;
    }
};

#endif