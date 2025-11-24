#pragma once

#include "helpers/enc.h"
#include "plugins/components/ViewInterface.h"
#include "plugins/components/componentInterface.h"
#include "plugins/components/drawInterface.h"
#include "plugins/components/utils/inRect.h"

#include <string>
#include <vector>

class Container {
protected:
    DrawInterface& draw;
    std::function<void(std::string name)> setView;
    float* contextVar;

    float lastxFactor = 1.0f, lastyFactor = 1.0f;
    std::vector<EventInterface::EncoderPosition> encoderPositions;

    std::vector<ComponentInterface*> componentsJob = {};
    std::vector<ComponentInterface*> componentsToRender = {};

    uint16_t initCounter = 0;

    void onUpdate(ValueInterface* val)
    {
        for (auto& component : components) {
            if (component->isVisible()) {
                for (auto* value : component->values) {
                    if (value == val) {
                        component->onUpdate(value);
                    }
                }
            }
        }
    }

public:
    std::vector<ComponentInterface*> components = {};

    Container(DrawInterface& draw, std::function<void(std::string name)> setView, float* contextVar)
        : draw(draw)
        , setView(setView)
        , contextVar(contextVar)
    {
    }

    void init()
    {
        encoderPositions = getEncoderPositions();
    }

    void activate()
    {
        if (lastxFactor != draw.getxFactor() || lastyFactor != draw.getyFactor()) {
            resize(draw.getxFactor(), draw.getyFactor());
        }

        for (auto& component : components) {
            component->initView(initCounter);
            component->renderNext();
            for (auto* value : component->values) {
                value->setOnUpdateCallback(
                    [this](float, void* data) { onUpdate((ValueInterface*)data); },
                    value);
            }
        }
        initCounter++;
    }

    void pushToRenderingQueue(void* component)
    {
        componentsToRender.push_back((ComponentInterface*)component);
    }

    void onContext(uint8_t index, float value)
    {
        for (auto& component : components) {
            component->onContext(index, value);
        }
    }

    void addComponent(ComponentInterface* component)
    {
        components.push_back(component);
        if (component->jobRendering) {
            componentsJob.push_back(component);
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
    }

    void onMotion(MotionInterface& motion)
    {
        for (auto& component : components) {
            if (component->isVisible()) {
                component->handleMotion(motion);
            }
        }
    }

    void onMotionRelease(MotionInterface& motion)
    {
        for (auto& component : components) {
            if (component->isVisible()) {
                component->handleMotionRelease(motion);
            }
        }
    }

    void onEncoder(int8_t id, int8_t direction, uint64_t tick)
    {
        for (auto& component : components) {
            if (component->isVisible()) {
                component->onEncoder(id, direction);
            }
        }
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        for (auto& component : components) {
            if (component->isVisible()) {
                if (component->onKey(id, key, state, now)) { // exit as soon as action happen, do not support multiple action for different component
                    break;
                }
            }
        }
    }

    void resize(float xFactor, float yFactor)
    {
        lastxFactor = xFactor;
        lastyFactor = yFactor;
        for (auto& component : components) {
            component->resize(xFactor, yFactor);
        }
        encoderPositions = getEncoderPositions();
    }

    const std::vector<EventInterface::EncoderPosition> getEncoderPositions()
    {
        std::vector<EventInterface::EncoderPosition> positions;
        for (auto& component : components) {
            auto encoderPositions = component->getEncoderPositions();
            positions.insert(positions.end(), encoderPositions.begin(), encoderPositions.end());
        }
        return positions;
    }

    int8_t getEncoderId(int32_t x, int32_t y, bool isMotion = false)
    {
        for (auto& encoderPosition : encoderPositions) {
            if (isMotion && !encoderPosition.allowMotionScroll)
                continue;

            if (inRect({ .position = encoderPosition.pos, .size = encoderPosition.size }, { x, y })) {
                return encoderPosition.id;
            }
        }
        return -2;
    }
};
