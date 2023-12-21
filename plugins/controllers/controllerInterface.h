#ifndef _CONTROLLER_INTERFACE_H_
#define _CONTROLLER_INTERFACE_H_

#include <stdint.h>
#include <string>
#include <unistd.h>
#include <vector>

class ControllerInterface {
public:
    void (*midi)(std::vector<unsigned char>* message) = NULL;
    void (*encoder)(int id, int8_t direction) = NULL;
    void (*keypad)(int id, int8_t state) = NULL;

    struct Props {
        void (*midi)(std::vector<unsigned char>* message) = NULL;
        void (*encoder)(int id, int8_t direction) = NULL;
        void (*keypad)(int id, int8_t state) = NULL;
    };
    ControllerInterface(Props& props)
        : midi(props.midi)
        , encoder(props.encoder)
        , keypad(props.keypad)
    {
    }

    virtual bool config(char* key, char* params)
    {
        return false;
    }

    // virtual void* data(int id, void* userdata = NULL)
    // {
    //     return NULL;
    // }
};

#endif