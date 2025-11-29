/** Description:
This C++ header defines a central architectural element called `Container`. Its primary role is to act as a manager for a group of interactive visual elements, often referred to as "components" (like buttons, sliders, or graphs), within a larger application view.

### Summary of Functionality

The `Container` is essentially a panel or section of the user interface that organizes and controls other smaller elements. It performs four main functions:

1.  **Component Management:** It stores a list of elements it needs to manage and provides methods to add new ones. It keeps track of which components need immediate rendering and which require background processing ("job rendering").
2.  **Event Dispatcher:** The container acts as a receptionist for user input. When events occur (such as mouse movements, key presses, or encoder rotations), the container receives them first and then automatically routes these signals to the correct, currently visible component for processing.
3.  **Drawing and Lifecycle Control:** It coordinates when components are initialized, activated, and drawn to the screen using a provided `DrawInterface`. It manages a rendering queue to ensure efficient screen updates.
4.  **Layout Adaptation:** The container is designed to be responsive. When the overall application or display area changes size, the `resize` function calculates new relative positions and scales all contained components to ensure they fit correctly and maintain proportion.

In essence, the `Container` provides the structure necessary to build complex, responsive user interfaces by grouping individual interactive elements and efficiently routing all necessary data and user commands to them.

sha: 4e86e391a1fe9cd98cf37310012ed884489eeaea040aa9f42b06c32ff8db578c 
*/
#pragma once

#include "helpers/enc.h"
#include "log.h"
#include "plugins/components/ViewInterface.h"
#include "plugins/components/componentInterface.h"
#include "plugins/components/drawInterface.h"
#include "plugins/components/utils/inRect.h"
#include "plugins/components/utils/resize.h"

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

    Point relativePosition = { 0, 0 };
    Size relativeSize = { 0, 0 };
    uint8_t resizeType = RESIZE_ALL;

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
    std::string name = "";
    Point position = { 0, 0 };
    Size size = { 0, 0 };

    std::vector<ComponentInterface*> components = {};

    Container(DrawInterface& draw, std::function<void(std::string name)> setView, float* contextVar)
        : draw(draw)
        , setView(setView)
        , contextVar(contextVar)
    {
    }

    Container(DrawInterface& draw, std::function<void(std::string name)> setView, float* contextVar, std::string name, Point position, Size size, uint8_t resizeType)
        : draw(draw)
        , setView(setView)
        , contextVar(contextVar)
        , name(name)
        , position(position)
        , size(size)
        , resizeType(resizeType)
    {
    }

    void init()
    {
        encoderPositions = getEncoderPositions();
    }

    void activate()
    {
        logDebug("activate container");
#ifdef DRAW_DESKTOP
        if (lastxFactor != draw.getxFactor() || lastyFactor != draw.getyFactor()) {
            resize(draw.getxFactor(), draw.getyFactor());
        }
#endif
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

        resizeOriginToRelative(resizeType, xFactor, yFactor, position, size, relativePosition, relativeSize);
        float xFactorComponent = size.w ? relativeSize.w / float(size.w) : 1.0f;
        float yFactorComponent = size.h ? relativeSize.h / float(size.h) : 1.0f;

        for (auto& component : components) {
            component->resize(xFactorComponent, yFactorComponent, relativePosition);
        }
#ifdef DRAW_DESKTOP
        encoderPositions = getEncoderPositions();
#endif
    }

    const std::vector<EventInterface::EncoderPosition> getEncoderPositions()
    {
        std::vector<EventInterface::EncoderPosition> positions;
#ifdef DRAW_DESKTOP
        for (auto& component : components) {
            auto encoderPositions = component->getEncoderPositions();
            positions.insert(positions.end(), encoderPositions.begin(), encoderPositions.end());
        }
#endif
        return positions;
    }

    int8_t getEncoderId(int32_t x, int32_t y, bool isMotion = false)
    {
#ifdef DRAW_DESKTOP
        for (auto& encoderPosition : encoderPositions) {
            if (isMotion && !encoderPosition.allowMotionScroll)
                continue;

            if (inRect({ .position = encoderPosition.pos, .size = encoderPosition.size }, { x, y })) {
                return encoderPosition.id;
            }
        }
#endif
        return -2;
    }
};
