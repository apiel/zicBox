#ifndef _CONTROLLER_INTERFACE_H_
#define _CONTROLLER_INTERFACE_H_

#include <stdint.h>
#include <unistd.h>
#include <vector>

class ControllerInterface
{
public:
    void (*midi)(std::vector<unsigned char> *message) = NULL;
    void (*encoder)(int id, int8_t direction) = NULL;

    struct Props
    {
        void (*midi)(std::vector<unsigned char> *message) = NULL;
        void (*encoder)(int id, int8_t direction) = NULL;
    };
    ControllerInterface(Props &props)
        : midi(props.midi), encoder(props.encoder)
    {
    }

    virtual bool config(char *key, char *value)
    {
        return false;
    }
};

#endif