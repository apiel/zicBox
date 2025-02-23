#pragma once

#include <stdint.h>
#include <string>
#include <unistd.h>
#include <vector>
#include "libs/nlohmann/json.hpp"

class ControllerInterface {
public:
    uint16_t id;
    void (*encoder)(int id, int8_t direction, uint32_t tick) = NULL;
    void (*onKey)(uint16_t id, int key, int8_t state) = NULL;

    struct Props {
        void (*encoder)(int id, int8_t direction, uint32_t tick) = NULL;
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

    // virtual void* data(int id, void* userdata = NULL)
    // {
    //     return NULL;
    // }
};
