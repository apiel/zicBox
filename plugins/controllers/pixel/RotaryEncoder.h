#ifndef _ROTARY_ENCODER_H
#define _ROTARY_ENCODER_H

#ifdef PIGPIO
// sudo apt-get install libpigpio-dev
#include <pigpio.h>
#endif
#include <stdint.h>
#include <stdio.h>

#include "../controllerInterface.h"

class RotaryEncoder {
protected:
    int levelA = 0;
    int levelB = 0;
    int lastGpio = -1;

    static void pulseEx(int gpio, int level, uint32_t tick, void* user)
    {
        RotaryEncoder* self = (RotaryEncoder*)user;
        self->pulse(gpio, level, tick);
    }

    void send(uint8_t direction, uint32_t tick)
    {
        controller->encoder(encoderId, direction, tick);
    }

public:
    int gpioA;
    int gpioB;
    uint8_t encoderId;
    ControllerInterface* controller;

    RotaryEncoder(ControllerInterface * _controller, int _gpioA, int _gpioB, uint8_t _encoderId)
        : gpioA(_gpioA)
        , gpioB(_gpioB)
        , encoderId(_encoderId)
        , controller(_controller)
    {
        printf("[encoderId: %d] gpioA: %d, gpioB: %d\n", encoderId, gpioA, gpioB);

#ifdef PIGPIO
        gpioSetMode(gpioA, PI_INPUT);
        gpioSetMode(gpioB, PI_INPUT);
        gpioSetPullUpDown(gpioA, PI_PUD_UP);
        gpioSetPullUpDown(gpioB, PI_PUD_UP);
        gpioSetAlertFuncEx(gpioA, pulseEx, this);
        gpioSetAlertFuncEx(gpioB, pulseEx, this);
#endif
    }

    void pulse(int gpio, int level, uint32_t tick)
    {
        if (gpio == gpioA) {
            levelA = level;
        } else {
            levelB = level;
        }

        if (gpio != lastGpio) /* debounce */
        {
            lastGpio = gpio;

            if ((gpio == gpioA) && (level == 1)) {
                if (levelB) {
                    send(-1, tick);
                }
            } else if ((gpio == gpioB) && (level == 1)) {
                if (levelA) {
                    send(1, tick);
                }
            }
        }
    }
};

#endif
