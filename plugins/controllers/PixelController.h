#ifndef _PIXEL_CONTROLLER_H_
#define _PIXEL_CONTROLLER_H_

#include "controllerInterface.h"
#include "helpers/controller.h"
#include "helpers/gpio.h"

#include <stdio.h>
#include <thread>
#include <vector>

class PixelController : public ControllerInterface {
protected:
    std::thread loopThread;
    bool loopRunning = true;

    struct Key {
        uint8_t gpio;
        int key;
        uint8_t lastState = 0;
    };

    std::vector<Key> keys = {
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
    };

public:
    PixelController(Props& props, uint16_t id)
        : ControllerInterface(props, id)
    {
        loopThread = std::thread(&PixelController::loop, this);
    }

    ~PixelController()
    {
        loopRunning = false;
    }

    void loop()
    {
        if (initGpio() == -1) {
            return;
        }

        for (auto& key : keys) {
            gpioSetMode(key.gpio, GPIO_INPUT);
            gpioSetPullUp(key.gpio);
            key.lastState = gpioRead(key.gpio);
            // printf("[%d] ", key.key);
        }
        // printf("\n");

        while (loopRunning) {
            for (auto& key : keys) {
                uint8_t state = gpioRead(key.gpio);
                if (state != key.lastState) {
                    key.lastState = state;
                    onKey(id, key.key, state);
                    printf("gpio%d key [%d] state changed %d\n", key.gpio, key.key, state);
                }
                // printf("[%d]=%d ", key.key, state);
            }
            // printf("\n");
            std::this_thread::sleep_for(10ms);
        }
    }
};

#endif