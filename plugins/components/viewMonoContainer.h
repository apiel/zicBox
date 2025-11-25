/** Description:
This code defines a central structural element called `View`, which typically represents an entire screen or main panel within a graphical application or control system.

The `View` is responsible for two main duties: organizing the visual elements and handling all user interactions.

1.  **Organization and Drawing:** The `View` uses an internal structure called a `Container`. This container holds all the smaller visual components (like buttons, sliders, or graphs) that make up the screen. The `View` simply delegates most drawing and setup tasks to this central container.

2.  **Input Management:** The `View` is the primary receiver of all user input:
    *   It handles button presses (`onKey`).
    *   It manages touch inputs or mouse movements (`onMotion`).
    *   It processes physical rotary encoders (knobs). For these knobs, it tracks the timing of rotations to calculate the speed, allowing the system to adjust sensitivity (scaling the input) dynamically—a fast turn moves the value more quickly.

3.  **Safety and Stability:** To ensure the system remains stable when inputs arrive simultaneously (e.g., a button press and a knob turn), the code uses a locking mechanism (a "mutex"). This forces operations like handling input or resizing the screen to happen one at a time, preventing data corruption or crashes.

In summary, the `View` acts as a robust, thread-safe intermediary that collects all user actions, interprets them (especially for speed-sensitive controls like encoders), and routes them efficiently to the correct underlying visual components.

sha: 140df081f12eb909d7983315c145ed6e0ff7a29e67bfd6706929c19e53ee8b78
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

    void addContainer(std::string& name) override { }

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
