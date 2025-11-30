/** Description:
This C++ header defines the blueprint for a crucial software element called `Container`. This class acts as a central manager or a panel that groups, organizes, and controls various interactive or visual elements, referred to internally as "Components."

### Core Purpose and Role

The primary function of the `Container` is to aggregate individual widgets (like buttons, sliders, or text fields) and manage their combined presentation and behavior within a larger display environment. It handles essential tasks such as drawing, layout management, and user interaction routing.

### Key Responsibilities

1.  **Drawing Coordination:** The Container works closely with the system's drawing mechanism to ensure all its components are rendered correctly onto the screen. It manages rendering queues, distinguishing between components that need immediate updates and those running continuous background tasks.
2.  **Layout and Resizing:** It determines how components are positioned and sized, supporting configuration based on fixed pixel values or proportional percentages. If the overall screen size changes (e.g., resizing a window), the Container automatically recalculates and adjusts the layout of its contents to maintain visual consistency.
3.  **Input Handling:** The Container acts as an event dispatcher. It receives all forms of user input from the system, including screen touches/movements, key presses, and input from rotary dials (encoders). It checks which internal component is currently visible and responsible, and forwards the relevant event only to that component for handling.
4.  **Component Lifecycle Management:** It manages the initialization and activation sequence for all its components, ensuring they are properly set up before they start interacting with the user or rendering content. It also allows components to communicate data updates back to the Container, triggering necessary visual changes.

In essence, the `Container` serves as the foundational structure that allows multiple specialized interface elements to function cohesively within a modern graphical application.

sha: 96c0626fee3e49c24ef57f4be0c0ee401a351a5cfd0ccd1180b432c2b68e1308 
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

    Size lastScreenSize = { 0, 0 };
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
    std::string name = "";
    Point position = { 0, 0 };
    int originalHeight = 0;
    int fixedHeight = 0; // for px
    float percentHeight = 0.0f; // for %

    std::vector<ComponentInterface*> components = {};

    Container(DrawInterface& draw, std::function<void(std::string name)> setView, float* contextVar)
        : draw(draw)
        , setView(setView)
        , contextVar(contextVar)
    {
    }

    Container(DrawInterface& draw, std::function<void(std::string name)> setView, float* contextVar, std::string name, Point position, std::string& height)
        : draw(draw)
        , setView(setView)
        , contextVar(contextVar)
        , name(name)
        , position(position)
    {
        if (height.find("px") != std::string::npos) {
            fixedHeight = std::stoi(height);
            originalHeight = fixedHeight;
        } else if (height.find("%") != std::string::npos) {
            percentHeight = std::stof(height) / 100.f;
        }
    }

    void init()
    {
        encoderPositions = getEncoderPositions();
    }

    void activate()
    {
        // logDebug("activate container");
#ifdef DRAW_DESKTOP
        Size screenSize = draw.getScreenSize();
        if (lastScreenSize.w != screenSize.w || lastScreenSize.h != screenSize.h) {
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
        lastScreenSize = draw.getScreenSize();

        for (auto& component : components) {
            // component->resize(xFactor, yFactor, relativePosition);
            component->resize(xFactor, yFactor, position);
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
