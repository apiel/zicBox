#ifndef _HELPER_GPIO_KEY_H_
#define _HELPER_GPIO_KEY_H_

#include "helpers/controller.h"
#include "helpers/gpio.h"

#include <functional>
#include <stdio.h>
#include <thread>
#include <vector>

using namespace std;

#ifdef DEBUG_GPIO_KEY
#define LOG_GPIO_KEY(...) printf(__VA_ARGS__)
#else
#define LOG_GPIO_KEY(...)
#endif

class GpioKey {
public:
    struct Key {
        uint8_t gpio;
        int key;
        uint8_t lastState = 0;
    };

protected:
    std::thread loopThread;
    bool loopRunning = true;

    std::vector<Key> keys;

    std::function<void(Key, uint8_t)> onKey;

public:
    GpioKey(std::vector<Key> keys, std::function<void(Key, uint8_t)> onKey)
        : keys(keys)
        , onKey(onKey)
    {
    }

    void startThread()
    {
        loopThread = std::thread(&GpioKey::loop, this);
    }

    ~GpioKey()
    {
        loopRunning = false;
    }

    int init()
    {
        if (initGpio() == -1) {
            return -1;
        }

        for (auto& key : keys) {
            gpioSetMode(key.gpio, GPIO_INPUT);
            gpioSetPullUp(key.gpio);
            key.lastState = gpioRead(key.gpio);
        }
        return 0;
    }

    void loop()
    {

        while (loopRunning) {
            for (auto& key : keys) {
                uint8_t state = gpioRead(key.gpio);
                if (state != key.lastState) {
                    key.lastState = state;
                    onKey(key, state);
                    // LOG_GPIO_KEY("gpio%d key [%d] state changed %d\n", key.gpio, key.key, state);
                }
                LOG_GPIO_KEY("[%d]=%d ", key.key, state);
            }
            LOG_GPIO_KEY("\n");
            std::this_thread::sleep_for(10ms);
        }
    }
};

#endif