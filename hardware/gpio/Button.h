#ifndef _BUTTON_H
#define _BUTTON_H

#include "def.h"

class Button {
protected:
    static void pulseEx(int gpio, int level, uint32_t tick, void* user)
    {
        Button* self = (Button*)user;
        self->pulse(gpio, level, tick);
    }

    void send(uint8_t direction, uint32_t tick)
    {
        if (debug) {
            printf("[buttomId %d] send: %d\n", buttomId, direction);
        }
        lo_send(oscClient, "/button", "iii", buttomId, direction, tick);
    }

public:
    int gpio;
    uint8_t buttomId;

    Button(int _gpio, uint8_t _buttomId)
        : gpio(_gpio)
        , buttomId(_buttomId)
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
