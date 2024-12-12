// g++ pixelController.cpp -o pixelController -I../.. -pthread && ./pixelController

// g++ pixelController.cpp -o pixelController -I../.. -pthread -lpigpio -lrt -DPIGPIO=1 && ./pixelController

#ifdef PIGPIO
// sudo apt-get install libpigpio-dev
#include <pigpio.h>
#else
#include "helpers/gpio.h"
#endif

#include <stdio.h>
#include <thread>
#include <unistd.h>
#include <vector>

class PixelController {
protected:
    std::thread loopThread;
    bool loopRunning = true;

    struct Key {
        uint8_t pin;
        int key;
        uint8_t lastState = 0;
    };

    std::vector<Key> keys = {
        { 13, 0 }, // pin 33 = gpio 13
        { 7, 1 }, // pin 26 = gpio 7
        { 25, 2 }, // pin 22 = gpio 25
        { 23, 3 }, // pin 16 = gpio 23
        { 14, 4 }, // pin 8 = gpio 14
        { 16, 5 }, // pin 36 = gpio 16
        { 12, 6 }, // pin 32 = gpio 12
        { 8, 7 }, // pin 24 = gpio 8
        { 24, 8 }, // pin 18 = gpio 24
        { 15, 9 }, // pin 10 = gpio 15
    };

public:
    PixelController()
    {
#ifndef PIGPIO
        if (initGpio() == -1) {
            return;
        }
#else
        gpioInitialise();
#endif
        for (auto& key : keys) {
#ifdef PIGPIO
            gpioSetMode(key.pin, PI_INPUT);
            key.lastState = gpioRead(key.pin);
#else
            // gpioSetMode(key.pin, GPIO_INPUT);
            // setGpio(key.pin);
            gpioSetMode(key.pin, GPIO_INPUT);
            key.lastState = gpioRead(key.pin);
#endif
        }

        loopThread = std::thread(&PixelController::loop, this);
    }

    ~PixelController()
    {
        loopRunning = false;
    }

    void loop()
    {
        while (loopRunning) {
            for (auto& key : keys) {
#ifdef PIGPIO
                uint8_t state = gpioRead(key.pin);
#else
                uint8_t state = gpioRead(key.pin);
#endif
                if (state != key.lastState) {
                    key.lastState = state;
                    // controller->onKey(controller->id, key.key, state);
                    // printf("key [%d] state changed %d\n", key.key, state);
                }
                printf("[%d]=%d ", key.key, state);
            }
            printf("\n");
            // std::this_thread::sleep_for(1000ms);
            usleep(100);
        }
    }
};

// class PixelController {
// protected:
//     std::thread loopThread;
//     bool loopRunning = true;

//     struct Key {
//         uint8_t pin;
//         int key;
//         uint8_t lastState = 0;
//     };

//     std::vector<Key> keys = {
//         { 13, 0 }, // pin 33 = gpio 13
//         { 7, 1 }, // pin 26 = gpio 7
//         { 25, 2 }, // pin 22 = gpio 25
//         { 23, 3 }, // pin 16 = gpio 23
//         { 14, 4 }, // pin 8 = gpio 14
//         { 16, 5 }, // pin 36 = gpio 16
//         { 12, 6 }, // pin 32 = gpio 12
//         { 8, 7 }, // pin 24 = gpio 8
//         { 24, 8 }, // pin 18 = gpio 24
//         { 15, 9 }, // pin 10 = gpio 15
//     };

// public:
//     PixelController()
//     {
//         gpioInitialise();
//         for (auto& key : keys) {
//             gpioSetMode(key.pin, PI_INPUT);
//             gpioSetPullUpDown(key.pin, PI_PUD_UP);
//             gpioSetAlertFuncEx(key.pin, pulseEx, this);
//         }
//     }

//     static void pulseEx(int gpio, int level, uint32_t tick, void* user)
//     {
//         // RotaryEncoder* self = (RotaryEncoder*)user;
//         // self->pulse(gpio, level, tick);
//         printf("key [%d] state changed %d\n", gpio, level);
//     }
// };

int main(int argc, char* argv[])
{
    new PixelController();

    while (true) {
        usleep(1);
    }
    return 0;
}