/** Description:
This specialized module, defined within a C++ header, provides an essential background service for monitoring physical buttons or switches connected to an embedded system's input/output pins (GPIO).

**Core Functionality**

The main component is the `GpioKey` class, which acts as a dedicated hardware input manager. It simplifies the process of reading user input from physical controls.

1.  **Setup and Initialization:** The module is configured with a list of specific pins (GPIOs) that correspond to buttons. During startup, it sets up these pins to reliably read external input.

2.  **Continuous Monitoring:** It launches a dedicated, small background process (a thread) that runs continuously. This process repeatedly checks the physical state of every registered button, performing a scan approximately every 10 milliseconds.

3.  **State Change Notification:** If the background process detects that a button's state has changed—meaning it was either pressed down or released—it immediately triggers a custom notification mechanism. This instantly alerts the main program, providing the key's unique identifier and its new state.

**How It Works**

This design prevents the main application from having to pause and check the buttons itself. Instead, the `GpioKey` module handles the complex, rapid polling in the background, ensuring immediate response to user actions while keeping the main application streamlined and efficient.

sha: d0fe95b1242ccf45324d65c7f4e898f70ded1ae5f81989e3e681b638dba95352 
*/
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
    std::vector<Key> keys;

protected:
    std::thread loopThread;
    bool loopRunning = true;
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
        pthread_setname_np(loopThread.native_handle(), "gpioKey");
    }

    ~GpioKey()
    {
        loopRunning = false;
    }

    void initKeys() {
        for (auto& key : keys) {
            gpioSetMode(key.gpio, GPIO_INPUT);
            gpioSetPullUp(key.gpio);
            key.lastState = gpioRead(key.gpio);
        }
    }

    int init()
    {
        if (initGpio() == -1) {
            return -1;
        }
        initKeys();
        return 0;
    }

    void loop()
    {
        while (loopRunning) {
            handler();
            std::this_thread::sleep_for(10ms);
        }
    }

    void handler()
    {
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
    }
};

#endif