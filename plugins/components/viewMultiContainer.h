#pragma once

#include "helpers/getTicks.h"
#include "plugins/components/EventInterface.h"
#include "plugins/components/container.h"

#include <mutex>
#include <string>
#include <vector>

class View : public ViewInterface, public EventInterface {
protected:
    Container container;
    std::mutex m2;

    // there should be about 4 to 12 encoders, however with 256 we are sure to not be out of bounds
    uint64_t lastEncoderTick[256] = { 0 };

public:
    std::string name;

    View(DrawInterface& draw, std::function<void(std::string name)> setView, float* contextVar)
        : ViewInterface(draw, setView, contextVar)
        , container(draw, setView, contextVar)
    {
    }

    void init()
    {
        container.init();
    }

    void activate()
    {
        container.activate();
    }

    std::vector<ComponentInterface*>& getComponents() { return container.components; }

    void pushToRenderingQueue(void* component)
    {
        container.pushToRenderingQueue(component);
    }

    void onContext(uint8_t index, float value)
    {
        container.onContext(index, value);
    }

    void addComponent(ComponentInterface* component)
    {
        container.addComponent(component);
    }

    void renderComponents(unsigned long now)
    {
        container.renderComponents(now);
        draw.triggerRendering();
    }

    void onMotion(MotionInterface& motion) override
    {
        container.onMotion(motion);
    }

    void onMotionRelease(MotionInterface& motion) override
    {
        container.onMotionRelease(motion);
    }

    void onEncoder(int8_t id, int8_t direction, uint64_t tick) override
    {
        m2.lock();
        if (tick > 0) {
            direction = encGetScaledDirection(direction, tick, lastEncoderTick[id]);
        }
        lastEncoderTick[id] = tick;
        container.onEncoder(id, direction, tick);
        m2.unlock();
    }

    void onKey(uint16_t id, int key, int8_t state) override
    {
        unsigned long now = getTicks();
        m2.lock();
        container.onKey(id, key, state, now);
        m2.unlock();
    }

    void resize(float xFactor, float yFactor) override
    {
        m2.lock();
        container.resize(xFactor, yFactor);
        m2.unlock();
        draw.renderNext();
    }

    const std::vector<EventInterface::EncoderPosition> getEncoderPositions()
    {
        return container.getEncoderPositions();
    }

    int8_t getEncoderId(int32_t x, int32_t y, bool isMotion = false) override
    {
        return container.getEncoderId(x, y, isMotion);
    }
};
