/** Description:
This C++ header file defines the blueprint for a standardized hardware interface, known as `ControllerInterface`. Its primary purpose is to establish a common way for an application to manage and receive input from various physical devices, such as specialized keypads, rotary encoders, or external control panels.

This interface ensures the main program can interact with any compatible device uniformly.

Each controller is assigned a unique identifier (`id`). The core functionality relies on "hooks," which are specialized pointers that instantly notify the application when a user interaction occurs.

1.  **Input Handling:**
    *   If a rotary dial is turned, the `encoder` hook is triggered, reporting the direction and duration of the turn.
    *   If a button or key is pressed or released, the `onKey` hook is activated, relaying the specific key and its current state (on or off).

2.  **Configuration and Output:**
    *   The `config` function allows the device to load necessary settings, often received in the industry-standard JSON data format.
    *   The `setColor` function provides a way for the application to send commands back to the device, typically used to change the color of components like LEDs or display segments.

By defining these standard hooks and functions, the application logic remains separate and clean, reacting consistently regardless of the specific hardware connected.

sha: c7c1d321a19718520daa6fbc5b596989168ea0755e21e2911f7434a1aca70eaf 
*/
#pragma once

#include "libs/nlohmann/json.hpp"
#include "draw/baseInterface.h" // for Color

#include <stdint.h>
#include <string>
#include <unistd.h>
#include <vector>

class ControllerInterface {
public:
    uint16_t id;
    void (*encoder)(int8_t id, int8_t direction, uint32_t tick) = NULL;
    void (*onKey)(uint16_t id, int key, int8_t state) = NULL;

    struct Props {
        void (*encoder)(int8_t id, int8_t direction, uint32_t tick) = NULL;
        void (*onKey)(uint16_t id, int key, int8_t state) = NULL;
    };
    ControllerInterface(Props& props, uint16_t id)
        : id(id)
        , encoder(props.encoder)
        , onKey(props.onKey)
    {
    }

    virtual void config(nlohmann::json& config)
    {
    }

    virtual void setColor(int id, Color color)
    {
    }

    // virtual void* data(int id, void* userdata = NULL)
    // {
    //     return NULL;
    // }
};
