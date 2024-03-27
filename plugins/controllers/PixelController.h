#ifndef _PIXEL_CONTROLLER_H_
#define _PIXEL_CONTROLLER_H_

#include "controllerInterface.h"
#include "pixel/RotaryEncoder.h"
#include "pixel/Button.h"

#include <cstring>
#include <stdio.h>

// TODO: instead to call it pixel controller
// let's call it encoderController
// where encoder can be as well push button
// ROTARY_ENCODER: id gpioA gpioB [gpioBtn]

class PixelController : public ControllerInterface {
protected:
public:
    PixelController(Props& props, uint16_t id)
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
        if (strcmp(key, "ROTARY_ENCODER") == 0) {
            int gpioA = atoi(strtok(params, " "));
            int gpioB = atoi(strtok(NULL, " "));
            int encoderId = atoi(strtok(NULL, " "));
            new RotaryEncoder(this, gpioA, gpioB, encoderId);
        } else if (strcmp(key, "BUTTON") == 0) {
            int gpio = atoi(strtok(params, " "));
            int buttonId = atoi(strtok(NULL, " "));
            new Button(this, gpio, buttonId);
            // } else if (strcmp(key, "MPR121") == 0) {
            //     char* p;
            //     int address = strtoul(strtok(params, " "), &p, 16);
            //     mpr121s.push_back(new Mpr121(address));
        }
        return false;
    }
};

#endif