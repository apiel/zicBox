#pragma once

#include <cstdint>
#include <vector>

#include "plugins/components/motionInterface.h"

class EventInterface {
public:
    virtual void onMotion(MotionInterface& motion) = 0;
    virtual void onMotionRelease(MotionInterface& motion) = 0;
    virtual void onEncoder(int id, int8_t direction, uint64_t tick) = 0;
    virtual void onKey(uint16_t id, int key, int8_t state) = 0;
    virtual void resize(float xFactor, float yFactor) = 0;

    struct EncoderPosition {
        uint8_t id;
        Size size;
        Point pos;
    };
    virtual const std::vector<EventInterface::EncoderPosition> getEncoderPositions() = 0;
};
