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

    Container* addContainer(std::string& name, Point position) override
    {
        containers.push_back(Container(draw, setView, contextVar, name, position));
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

    void resize(float xFactor, float yFactor) override
    {
        m2.lock();
        for (auto& c : containers)
            c.resize(xFactor, yFactor);
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
