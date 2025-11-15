#pragma once

#include "./ViewInterface.h"
#include "EventInterface.h"
#include "helpers/enc.h"
#include "helpers/getTicks.h"
#include "log.h"

#include <mutex>
#include <string>
#include <vector>

class View : public ViewInterface, public ComponentContainer, public EventInterface {
protected:
    float lastxFactor = 1.0f, lastyFactor = 1.0f;

public:
    std::vector<ComponentInterface*> components = {};
    std::vector<ComponentInterface*> componentsToRender = {};
    std::vector<ComponentInterface*> componentsJob = {};
    std::mutex m2;

    View(DrawInterface& draw, std::function<void(std::string name)> setView, float* contextVar)
        : ViewInterface(draw, setView, contextVar)
    {
    }

    void init()
    {
    }

    uint16_t initViewCounter = 0;
    void initActiveComponents()
    {
        for (auto& component : components) {
            component->initView(initViewCounter);
            component->renderNext();
            for (auto* value : component->values) {
                value->setOnUpdateCallback(
                    [this](float, void* data) { onUpdate((ValueInterface*)data); },
                    value);
            }
        }
        initViewCounter++;
    }

    void activate()
    {
        if (lastxFactor != draw.getxFactor() || lastyFactor != draw.getyFactor()) {
            resize(draw.getxFactor(), draw.getyFactor());
        }

        initActiveComponents();
    }

    void pushToRenderingQueue(void* component)
    {
        componentsToRender.push_back((ComponentInterface*)component);
    }

    std::vector<void*>* getComponents() override
    {
        return (std::vector<void*>*)&components;
    }

    void onContext(uint8_t index, float value)
    {
        // printf("on context %d to %f\n", index, value);
        for (auto& component : components) {
            component->onContext(index, value);
        }
    }

    void onUpdate(ValueInterface* val)
    {
        for (auto& component : components) {
            if (isVisible(component)) {
                for (auto* value : component->values) {
                    if (value == val) {
                        component->onUpdate(value);
                    }
                }
            }
        }
    }

    void renderComponents(unsigned long now)
    {
        if (componentsJob.size()) {
            for (auto& component : componentsJob) {
                if (component->isVisible()) {
                    component->jobRendering(now);
                }
            }
        }

        if (componentsToRender.size()) {
            for (auto& component : componentsToRender) {
                if (component->isVisible()) {
                    component->render();
                }
            }
            componentsToRender.clear();
            draw.renderNext();
        }
        draw.triggerRendering();
    }

    // Might want to rework this
    bool isVisible(ComponentInterface* component)
    {
        return component->isVisible();
    }

    void onMotion(MotionInterface& motion) override
    {
        for (auto& component : components) {
            if (isVisible(component)) {
                component->handleMotion(motion);
            }
        }
    }

    void onMotionRelease(MotionInterface& motion) override
    {
        for (auto& component : components) {
            if (isVisible(component)) {
                component->handleMotionRelease(motion);
            }
        }
    }

    // there should be about 4 to 12 encoders, however with 256 we are sure to not be out of bounds
    uint64_t lastEncoderTick[256] = { 0 };
    void onEncoder(int id, int8_t direction, uint64_t tick) override
    {
        // printf("onEncoder %d %d %d\n", id, direction, tick);
        m2.lock();
        // if (tick - lastEncoderTick[id] < 25) {
        //     direction = direction * 5;
        // }
        direction = encGetScaledDirection(direction, tick, lastEncoderTick[id]);
        lastEncoderTick[id] = tick;
        for (auto& component : components) {
            if (isVisible(component)) {
                component->onEncoder(id, direction);
                // printf("onEncoder %d %d component %s\n", id, direction, component->id.c_str());
            }
        }
        m2.unlock();
    }

    void onKey(uint16_t id, int key, int8_t state) override
    {
        // logDebug("onKey id %d key %d state %d", id, key, state);
        unsigned long now = getTicks();
        m2.lock();
        for (auto& component : components) {
            if (isVisible(component)) {
                if (component->onKey(id, key, state, now)) { // exit as soon as action happen, do not support multiple action for different component
                    break;
                }
            }
        }
        m2.unlock();
    }

    void resize(float xFactor, float yFactor) override
    {
        m2.lock();
        lastxFactor = xFactor;
        lastyFactor = yFactor;
        for (auto& component : components) {
            component->resize(xFactor, yFactor);
        }
        m2.unlock();
        draw.renderNext();
    }
};
