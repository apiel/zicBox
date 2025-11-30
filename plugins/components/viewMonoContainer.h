/** Description:
This header file defines a specialized class called `ViewMonoContainer`, which serves as a single, centralized control panel or screen interface within a larger application.

The core function of this class is to wrap and manage one single, powerful internal element—a standard `Container`. This container holds all the actual buttons, gauges, and interface components that the user sees and interacts with. The `ViewMonoContainer` acts mostly as a delegate, passing almost every command (like initializing, activating, rendering, or resizing) directly down to its internal container.

To ensure stability and safety, especially when handling user input or screen updates, the class incorporates a digital locking mechanism called a "mutex." This lock prevents the system from becoming corrupted if multiple processes (like a key press and a resize operation) try to change the interface state at the exact same moment.

A unique feature is its detailed handling of physical input knobs (encoders). It tracks the timing of each turn to intelligently adjust the sensitivity of the input. If the user turns a knob quickly, the system accelerates the input value, allowing for faster adjustments; if turned slowly, the precision is maintained.

In summary, this class provides a secure and organized structure where a complex group of user interface components can be safely initialized, rendered, and efficiently manipulated via various forms of input, including keys and motion events.

sha: f2f842392b4c1b1c82e182b71492f1e0d5062029f7a0a48ad1c7be32ea285a64 
*/
#pragma once

#include "helpers/getTicks.h"
#include "plugins/components/container.h"
#include "plugins/components/view.h"

#include <mutex>
#include <string>
#include <vector>

class ViewMonoContainer : public View {
protected:
    Container container;
    std::mutex m2;

    // there should be about 4 to 12 encoders, however with 256 we are sure to not be out of bounds
    uint64_t lastEncoderTick[256] = { 0 };

public:
    ViewMonoContainer(DrawInterface& draw, std::function<void(std::string name)> setView, float* contextVar)
        : View(draw, setView, contextVar)
        , container(draw, setView, contextVar)
    {
    }

    void init() override
    {
        container.init();
    }

    void activate() override
    {
        container.activate();
    }

    std::vector<ComponentInterface*>& getComponents() override { return container.components; }

    void pushToRenderingQueue(void* component) override
    {
        container.pushToRenderingQueue(component);
    }

    void onContext(uint8_t index, float value) override
    {
        container.onContext(index, value);
    }

    Container* addContainer(std::string& name, Point position, std::string height) override { return &container; }

    void addComponent(ComponentInterface* component, Container*) override
    {
        container.addComponent(component);
    }

    void renderComponents(unsigned long now) override
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

    void resize() override
    {
        m2.lock();
        container.resize(draw.getxFactor(), draw.getyFactor());
        m2.unlock();
        draw.renderNext();
    }

    const std::vector<EventInterface::EncoderPosition> getEncoderPositions() override
    {
        return container.getEncoderPositions();
    }

    int8_t getEncoderId(int32_t x, int32_t y, bool isMotion = false) override
    {
        return container.getEncoderId(x, y, isMotion);
    }
};
