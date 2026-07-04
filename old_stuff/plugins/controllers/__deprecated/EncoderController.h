/** Description:
This file serves as the blueprint (header) for an advanced control system focused on managing physical rotary encoders—the common type of input knob used for adjustments and selection, often paired with a push button.

The core functionality is split between two main components:

1.  **The Rotary Encoder Handler:** This part is highly specialized. It constantly monitors the electronic signals coming from the physical knob. Because a rotary encoder outputs a sequence of pulses across two separate pins (A and B), this handler uses precise timing logic to decode these pulses. This allows it to accurately determine both the direction (clockwise or counter-clockwise) and the speed of rotation. It also manages the dedicated input pin for the knob's built-in button, sending an immediate signal whenever it is pressed or released.

2.  **The Encoder Controller:** This acts as the central manager. It handles system startup, ensuring that the necessary hardware communication libraries (likely for a Raspberry Pi) are properly initialized. Its primary role is configuration: it reads setup parameters (like pin numbers and device IDs) and creates new instances of the encoder handler for every knob connected to the system.

In essence, this header defines the digital bridge that translates the physical actions of turning a knob or pressing a button into specific, usable commands for the controlling software.

sha: 910fd55f29d23726a9716d2b6e557de2e18d7592e8e056fab44edeff6a240e24 
*/
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
        // printf("encoder button (%d): %d\n", self->encoderId, level);
    }

    void send(uint8_t direction, uint32_t tick)
    {
        controller->encoder(encoderId, direction, tick);
        // printf("encoder (%d): %d\n", encoderId, direction);
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