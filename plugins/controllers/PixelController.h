#ifndef _PIXEL_CONTROLLER_H_
#define _PIXEL_CONTROLLER_H_

#include "controllerInterface.h"

#include <stdio.h>

class PixelController : public ControllerInterface
{
protected:

public:
    PixelController(Props &props, uint16_t id) : ControllerInterface(props, id)
    {
    }
};

#endif