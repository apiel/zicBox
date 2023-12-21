#ifndef _KEYPAD_INTERFACE_H_
#define _KEYPAD_INTERFACE_H_

#include "controllerInterface.h"

class KeypadInterface: public ControllerInterface{
public:
    
    KeypadInterface(ControllerInterface::Props& props)
        : ControllerInterface(props)
    {
    }

    virtual void setKeyColor(int id, uint8_t color) = 0;
    virtual void setButton(int id, uint8_t color) = 0;
};

#endif