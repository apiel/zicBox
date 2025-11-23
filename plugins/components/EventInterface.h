#pragma once

#include <cstdint>
#include <vector>

#include "plugins/components/motionInterface.h"

class EventInterface {
public:
    virtual void onMotion(MotionInterface& motion) = 0;
    virtual void onMotionRelease(MotionInterface& motion) = 0;
    virtual void onEncoder(int8_t id, int8_t direction, uint64_t tick) = 0;
    virtual void onKey(uint16_t id, int key, int8_t state) = 0;
    virtual void resize(float xFactor, float yFactor) = 0;

    struct EncoderPosition {
        int8_t id;
        Size size;
        Point pos;
        bool allowMotionScroll = true;
    };
    virtual const std::vector<EventInterface::EncoderPosition> getEncoderPositions() = 0; // might not even expose this in event interface.
    virtual int8_t getEncoderId(int32_t x, int32_t y, bool isMotion = false) = 0;
};
