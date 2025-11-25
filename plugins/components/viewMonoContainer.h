/** Description:
This C++ header defines a foundational class called `ViewMonoContainer`, which acts as a specialized manager for a single user interface screen or panel.

The core purpose of this class is to wrap and manage one central organizational unit, known internally as a `Container`. This container is responsible for holding and drawing all the individual elements (components) visible on the screen.

**How it Works:**

The `ViewMonoContainer` functions primarily as a delegate. For most standard screen functions—such as initialization, activation, rendering components, and handling touch or mouse input (motion)—it simply forwards the request directly to its internal `Container`. It provides the standardized public interface while letting the container handle the heavy lifting of component organization.

**Key Responsibilities and Safety Features:**

1.  **Thread Safety:** The class includes mechanisms (using a "mutex") to ensure that critical operations, such as resizing the display or processing keyboard input, cannot be interrupted or corrupted if multiple background tasks try to access the screen simultaneously. This ensures stability.
2.  **Advanced Input Processing:** It handles input from rotary encoders (physical knobs or dials). Before passing the movement data to the components, the container tracks the timing of the input. This allows it to calculate if the user is turning the knob quickly or slowly, enabling features like accelerated scrolling or value changes.

sha: 6c9c433e17a741b6160692478bd5eda2823d20a4338cf910704026ee0b927e20 
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

    Container* addContainer(std::string& name, Point position) override { return &container; }

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

    void resize(float xFactor, float yFactor) override
    {
        m2.lock();
        container.resize(xFactor, yFactor);
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
