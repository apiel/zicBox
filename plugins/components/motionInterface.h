#pragma once

#include "baseInterface.h"

class MotionInterface
{
public:
    int id = -1;
    int8_t encoderId = -1;
    Point position = {0, 0};
    Point origin = {0, 0};
    unsigned long counter = 0;

    virtual void setId(int id) = 0;
    virtual void setEncoderId(int8_t id) = 0;
    virtual void init(int id, int x, int y) = 0;
    virtual void move(int x, int y) = 0;
    virtual bool isStarting() = 0;
    virtual bool in(Rect rect) = 0;
    virtual bool originIn(Rect rect) = 0;
};
