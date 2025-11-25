#pragma once

#include <functional>
#include <string>
#include <vector>

#include "plugins/components/EventInterface.h"
#include "plugins/components/ViewInterface.h"
#include "plugins/components/componentInterface.h"
#include "plugins/components/container.h"

class View : public ViewInterface, public EventInterface {
public:
    std::string name;

    View(DrawInterface& draw,
        std::function<void(std::string)> setView,
        float* contextVar)
        : ViewInterface(draw, setView, contextVar)
    {
    }

    // lifecycle
    virtual void init() = 0;
    virtual void activate() = 0;

    // components
    virtual std::vector<ComponentInterface*>& getComponents() = 0;
    virtual void addContainer(std::string& name, Point position, Size size) = 0;
    virtual void addComponent(ComponentInterface* component, Container *container) = 0;
    virtual void renderComponents(unsigned long now) = 0;

    // UI events
    virtual void onContext(uint8_t index, float value) = 0;

    // override pure virtual from ViewInterface
    virtual void pushToRenderingQueue(void* component) override = 0;

    // override pure virtual from EventInterface
    virtual void onMotion(MotionInterface& motion) override = 0;
    virtual void onMotionRelease(MotionInterface& motion) override = 0;
    virtual void onEncoder(int8_t id, int8_t direction, uint64_t tick) override = 0;
    virtual void onKey(uint16_t id, int key, int8_t state) override = 0;
    virtual void resize(float xFactor, float yFactor) override = 0;

    // encoder API
    virtual int8_t getEncoderId(int32_t x, int32_t y, bool isMotion = false) override = 0;
    virtual const std::vector<EventInterface::EncoderPosition> getEncoderPositions() = 0;
};
