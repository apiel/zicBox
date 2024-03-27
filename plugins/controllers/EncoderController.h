#ifndef _ENCODER_CONTROLLER_H_
#define _ENCODER_CONTROLLER_H_

#include "controllerInterface.h"

#ifdef PIGPIO
// sudo apt-get install libpigpio-dev
#include <pigpio.h>
#endif

#include <cstring>
#include <stdio.h>

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

    static void pulseBtnEx(int gpio, int level, uint32_t tick, void* user)
    {
        RotaryEncoder* self = (RotaryEncoder*)user;
        self->controller->onKey(self->controller->id, self->encoderId, level ? 0 : 1);
        printf("encoder button (%d): %d\n", self->encoderId, level);
    }

    void send(uint8_t direction, uint32_t tick)
    {
        controller->encoder(encoderId, direction, tick);
        printf("encoder (%d): %d\n", encoderId, direction);
    }

public:
    int gpioA;
    int gpioB;
    int gpioBtn;
    uint8_t encoderId;
    ControllerInterface* controller;

    RotaryEncoder(ControllerInterface* _controller, uint8_t _encoderId, int _gpioA, int _gpioB, int _gpioBtn)
        : gpioA(_gpioA)
        , gpioB(_gpioB)
        , gpioBtn(_gpioBtn)
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

        if (gpioBtn >= 0) {
            gpioSetMode(gpioBtn, PI_INPUT);
            gpioSetPullUpDown(gpioBtn, PI_PUD_UP);
            gpioSetAlertFuncEx(gpioBtn, pulseBtnEx, this);
        }
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

class EncoderController : public ControllerInterface {
protected:
public:
    EncoderController(Props& props, uint16_t id)
        : ControllerInterface(props, id)
    {
#ifdef PIGPIO
        if (gpioInitialise() < 0) {
            printf("Failed to initialise GPIO\n");
        }
#endif
    }

    bool config(char* key, char* params)
    {
        if (strcmp(key, "ENCODER") == 0) {
            int encoderId = atoi(strtok(params, " "));
            int gpioA = atoi(strtok(NULL, " "));
            int gpioB = atoi(strtok(NULL, " "));
            int gpioBtn = -1;
            char *btn = strtok(NULL, " ");
            if (btn != NULL) {
                gpioBtn = atoi(btn);
            }
            new RotaryEncoder(this, encoderId, gpioA, gpioB, gpioBtn);
        }
        return false;
    }
};

#endif