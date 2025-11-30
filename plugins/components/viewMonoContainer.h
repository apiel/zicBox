/** Description:
This C++ header defines a core component called `ViewMonoContainer`, which serves as a specialized, self-contained screen or layout manager within a larger graphical application.

**Purpose and Structure**
The class acts as a dedicated display view (the `View` part) designed to host and manage only a single, primary layout area (the `MonoContainer` part). It essentially takes all user interface elements—buttons, displays, etc.—and places them within one main organizational box, or "container."

**Key Functions**
1.  **Input Routing:** This component is the central hub for user interaction. It receives input events like key presses, screen touches, and signals from physical rotary knobs (encoders). It then securely passes these events down to the elements inside its managed container.
2.  **Encoder Scaling:** A unique feature is its handling of rotary knobs. It tracks the speed at which a user turns a knob, allowing the system to scale the input. For instance, a quick turn might adjust a volume setting in large steps, while a slow turn allows for fine, precise adjustments.
3.  **System Safety:** The component utilizes a "lock" mechanism (`mutex`) during critical operations (like handling user input, resizing the display, or updating its layout). This safety measure ensures that simultaneous actions do not conflict or cause data corruption, leading to a stable and reliable interface.
4.  **Management:** It manages the life cycle of its contained elements, handling initial setup, rendering updates, and resizing adjustments whenever the application window changes.

In essence, `ViewMonoContainer` is a secure, specialized dashboard panel designed to manage one main layout perfectly and efficiently respond to all forms of user input.

sha: 8073dcefcb366368e950ed6c01e3048adbbe5f95c79c84e12b27110893a31e33 
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
