/** Description:
This structure defines a sophisticated screen element manager called `ViewMultiContainer`. Its primary purpose is to act as a unified dashboard that organizes and controls multiple independent interface panels, known internally as "Containers."

This manager inherits the basic functionality of a standard View but extends it to handle complex layouts. It maintains a list of all its organized panels and uses a protective lock mechanism to ensure that simultaneous user interactions (like pressing a key and turning a knob) are processed sequentially and safely, preventing errors.

When the application starts, the `ViewMultiContainer` initializes and activates all its sub-panels. It provides methods to dynamically add new panels and place individual interface elements (Components) inside them. It also combines all components from its sub-panels into one list for centralized management.

Crucially, it serves as a central input dispatcher. Any user interaction—such as key presses, motion events, or inputs from rotary knobs (encoders)—is first received by this manager. It then efficiently passes that input down to every relevant panel, allowing them to react accordingly. It includes specific logic for rotary knobs to accurately track movement speed and scaling.

Finally, it manages all drawing operations, ensuring all contained elements are rendered correctly on the screen. If the display size changes, the manager automatically resizes and adjusts the layout of all its contained panels, ensuring a coherent display.

sha: 24ef2e291bd4a621ccc4e2a4243e0b912c1b1c6a8ce2e1b6f210e382eca5401b
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

public:
    ViewMultiContainer(DrawInterface& draw,
        std::function<void(std::string)> setView,
        float* contextVar)
        : View(draw, setView, contextVar)
    {
    }

    void init() override
    {
        for (auto& c : containers)
            c.init();
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
        for (auto& c : containers)
            c.pushToRenderingQueue(component);
    }

    void renderComponents(unsigned long now) override
    {
        for (auto& c : containers)
            c.renderComponents(now);

        draw.triggerRendering();
    }

    void onContext(uint8_t index, float value) override
    {
        for (auto& c : containers)
            c.onContext(index, value);
    }

    void onMotion(MotionInterface& motion) override
    {
        for (auto& c : containers)
            c.onMotion(motion);
    }

    void onMotionRelease(MotionInterface& motion) override
    {
        for (auto& c : containers)
            c.onMotionRelease(motion);
    }

    void onEncoder(int8_t id, int8_t direction, uint64_t tick) override
    {
        m2.lock();
        if (tick > 0) {
            direction = encGetScaledDirection(direction, tick, lastEncoderTick[id]);
        }
        lastEncoderTick[id] = tick;

        for (auto& container : containers)
            container.onEncoder(id, direction, tick);

        m2.unlock();
    }

    void onKey(uint16_t id, int key, int8_t state) override
    {
        unsigned long now = getTicks();
        m2.lock();
        for (auto& c : containers)
            c.onKey(id, key, state, now);
        m2.unlock();
    }

    void resize() override
    {
        m2.lock();

        int totalFixed = 0;
        float totalPercent = 0.f;
        for (auto& c : containers) {
            if (c.fixedHeight > 0) {
                totalFixed += c.fixedHeight;
            } else {
                totalPercent += c.percentHeight;
            }
        }

        int screenHeight = draw.getScreenSize().h;
        int available = screenHeight - totalFixed;

        float xFactor = draw.getxFactor();
        int currentY = 0;
        for (auto& c : containers) {
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
