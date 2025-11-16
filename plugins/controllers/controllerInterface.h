#pragma once

#include "libs/nlohmann/json.hpp"
#include "plugins/components/baseInterface.h" // for Color

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
