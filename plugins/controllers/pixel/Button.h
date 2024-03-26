#ifndef _BUTTON_H
#define _BUTTON_H

#ifdef PIGPIO
// sudo apt-get install libpigpio-dev
#include <pigpio.h>
#endif
#include <stdint.h>
#include <stdio.h>

#include "../controllerInterface.h"

class Button {
protected:
    static void pulseEx(int gpio, int level, uint32_t tick, void* user)
    {
        Button* self = (Button*)user;
        self->pulse(gpio, level, tick);
    }

    void send(uint8_t direction, uint32_t tick)
    {
        // if (debug) {
        //     printf("[buttomId %d] send: %d\n", buttomId, direction);
        // }
        // lo_send(oscClient, "/button", "iii", buttomId, direction, tick);

        // controller->button(buttomId, direction, tick);
        printf("button %d: %d\n", buttomId, direction);
    }

public:
    int gpio;
    uint8_t buttomId;
    ControllerInterface* controller;

    Button(ControllerInterface * _controller, int _gpio, uint8_t _buttomId)
        : gpio(_gpio)
        , buttomId(_buttomId)
        , controller(_controller)
    {
        printf("[buttomId %d] gpio: %d\n", buttomId, gpio);

#ifdef PIGPIO
        gpioSetMode(gpio, PI_INPUT);
        gpioSetPullUpDown(gpio, PI_PUD_UP);
        gpioSetAlertFuncEx(gpio, pulseEx, this);
#endif
    }

    void pulse(int gpio, int level, uint32_t tick)
    {
        send(level, tick);
    }
};


#endif
