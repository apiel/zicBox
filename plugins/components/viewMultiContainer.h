/** Description:
This code defines a fundamental component for building complex user interfaces, specifically a class named `ViewMultiContainer`. Its main job is to manage and display multiple, independent screen sections or panels, referred to as "Containers," within a single view.

Think of this component as a layout manager for a dashboard.

**Core Functionality:**

1.  **Container Management:** It holds a collection of these sub-panels and is responsible for their initialization and activation. It can dynamically add new containers and the various display elements (components) that belong inside them.
2.  **Smart Layout:** A critical function is handling screen resizing. The component calculates how much vertical space is available and intelligently divides it among the managed containers. Some containers can specify a fixed height (e.g., 50 pixels), while others take a percentage of the remaining screen space, ensuring the entire view is utilized efficiently regardless of screen dimensions.
3.  **Input Routing:** This class acts as a central hub for all user interactions, including motion (touch or gestures), key presses, and rotary encoder turns (dials). It intercepts these events and forwards them to the specific container or element that needs to respond.
4.  **Encoder Enhancement:** For physical rotary dials, the code includes a special feature: it tracks the speed of the dial turn (using timing "ticks") to scale the input. This means a quick spin results in a larger response than a slow nudge.
5.  **Thread Safety:** The code uses an internal mechanism ("mutex") to lock data during critical operations, such as handling input or resizing. This prevents errors if multiple processes try to update the screen or handle user input simultaneously.

In summary, the `ViewMultiContainer` provides the necessary structure to stack multiple interactive widgets or screens together, handle all incoming user events safely, and automatically manage the flexible positioning and size of these elements.

sha: 0dc65b4480bbe5f77c056ef38107259d6ac99b66cb3f1909718e692f267c119f
*/
#pragma once

#include "helpers/getTicks.h"
#include "plugins/components/container.h"
#include "plugins/components/view.h"

#include <mutex>
#include <vector>

class ViewMultiContainer : public View {
protected:
    std::vector<Container> containers;
    std::mutex m2;

    // there should be about 4 to 12 encoders, however with 256 we are sure to not be out of bounds
    uint64_t lastEncoderTick[256] = { 0 };

    int totalFixed = 0;
    float totalPercent = 0.f;

public:
    ViewMultiContainer(DrawInterface& draw,
        std::function<void(std::string)> setView,
        float* contextVar)
        : View(draw, setView, contextVar)
    {
    }

    void init() override
    {
        for (auto& c : containers) {
            c.init();

            if (c.fixedHeight > 0) {
                totalFixed += c.fixedHeight;
            } else {
                totalPercent += c.percentHeight;
            }
        }
    }

    void activate() override
    {
        for (auto& c : containers)
            c.activate();
    }

    std::vector<ComponentInterface*>& getComponents() override
    {
        static std::vector<ComponentInterface*> merged;
        merged.clear();

        for (auto& c : containers) {
            if (!c.isVisible()) continue; // <-------------- ??
            for (auto* comp : c.components)
                merged.push_back(comp);
        }
        return merged;
    }

    Container* addContainer(std::string& name, Point position, std::string height) override
    {
        containers.push_back(Container(draw, setView, contextVar, name, position, height));
        return &containers.back();
    }

    void addComponent(ComponentInterface* component, Container* container) override
    {
        if (container) {
            container->addComponent(component);
        }
    }

    void pushToRenderingQueue(void* component) override
    {
        for (auto& c : containers) {
            c.pushToRenderingQueue(component);
        }
    }

    void renderComponents(unsigned long now) override
    {
        for (auto& c : containers) {
            c.renderComponents(now);
        }

        draw.triggerRendering();
    }

    void onContext(uint8_t index, float value) override
    {
        for (auto& c : containers) {
            c.onContext(index, value);
        }
    }

    void onMotion(MotionInterface& motion) override
    {
        for (auto& c : containers) {
            c.onMotion(motion);
        }
    }

    void onMotionRelease(MotionInterface& motion) override
    {
        for (auto& c : containers) {
            c.onMotionRelease(motion);
        }
    }

    void onEncoder(int8_t id, int8_t direction, uint64_t tick) override
    {
        m2.lock();
        if (tick > 0) {
            direction = encGetScaledDirection(direction, tick, lastEncoderTick[id]);
        }
        lastEncoderTick[id] = tick;

        for (auto& c : containers) {
            c.onEncoder(id, direction, tick);
        }

        m2.unlock();
    }

    void onKey(uint16_t id, int key, int8_t state) override
    {
        unsigned long now = getTicks();
        m2.lock();
        for (auto& c : containers) {
            c.onKey(id, key, state, now);
        }
        m2.unlock();
    }

    void resize() override
    {
        m2.lock();

        int available = draw.getScreenSize().h - totalFixed;
        float xFactor = draw.getxFactor();
        int currentY = 0;
        for (auto& c : containers) {
            if (!c.isVisible()) continue; // <----- TODO need to fix this, actually even if not visible, should still be resized
            int h = c.fixedHeight;
            if (c.percentHeight > 0.f) {
                if (totalPercent > 0)
                    h = int((c.percentHeight / totalPercent) * available);
                else
                    h = 0;
            }

            c.position.y = currentY;
            if (c.originalHeight == 0) c.originalHeight = h; // First time, initialize original height for responsive height using percent
            float yFactor = h / (float)c.originalHeight;
            c.resize(xFactor, yFactor);
            currentY += h;
        }

        m2.unlock();
        draw.renderNext();
    }

    const std::vector<EventInterface::EncoderPosition> getEncoderPositions() override
    {
        static std::vector<EventInterface::EncoderPosition> merged;
        merged.clear();

        for (auto& c : containers) {
            if (!c.isVisible()) continue; // <-------------- ?
            auto list = c.getEncoderPositions();
            merged.insert(merged.end(), list.begin(), list.end());
        }
        return merged;
    }

    int8_t getEncoderId(int32_t x, int32_t y, bool isMotion = false) override
    {
        for (auto& c : containers) {
            int8_t id = c.getEncoderId(x, y, isMotion);
            if (id > -2)
                return id;
        }
        return -2;
    }
};
