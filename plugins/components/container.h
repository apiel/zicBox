/** Description:
This C++ structure defines a crucial architectural element called a **Container**. Its primary purpose is to act as an organized panel or manager for groups of smaller, specialized graphical parts, referred to as "components," within a user interface (UI).

The Container handles essential functions related to screen layout, visibility, and user interaction routing:

1.  **Organization and Layout:** It defines its own position and size on the screen, which can be specified either using fixed pixel values or as a percentage of the total screen area. It manages a list of all components inside it.
2.  **Lifecycle and Activation:** When the application starts or when the Container becomes visible, it initializes its internal components and prepares them for drawing. It also sets up automatic update mechanisms, so if any data linked to a component changes, the component knows to refresh itself immediately.
3.  **Interaction Routing:** The Container acts as a central hub for user input. If you press a button, move a mouse, or turn a digital knob (encoder), the input first goes to the Container. It efficiently checks if it is visible, and if so, passes that specific input event down to the relevant visible component for processing.
4.  **Rendering Management:** It organizes the drawing process. Components that require constant background calculations or maintenance are handled separately from standard components that are just drawn on the screen, ensuring smooth performance.
5.  **Adaptability:** It contains logic to handle screen changes (resizing). If the main application window size shifts, the Container automatically adjusts the size and position of all its internal components to maintain the intended look and feel.

In essence, the Container is the robust structure that groups related UI elements, controls their collective visibility, and ensures they respond correctly to user actions and layout changes.

Tags: C++, Header File, Class Definition, Object-Oriented Programming, Container Pattern, Component Management, User Interface, GUI, Rendering Flow, Event Handling, Input Processing, Plugin System, Interface Based Architecture, Layout Management, Visibility Context, Responsiveness, Resize Logic, Job Queue, Encoder Input, Runtime Configuration
sha: 1855682992204270d2d58fa606a5179faed4e788c52525488c7f91d0be7bbbcd 
*/
#pragma once

#include "helpers/enc.h"
#include "log.h"
#include "plugins/components/ViewInterface.h"
#include "plugins/components/componentInterface.h"
#include "draw/drawInterface.h"
#include "plugins/components/utils/VisibilityContext.h"
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
    VisibilityContext visibilityContext;

    std::string name = "";
    Point position = { 0, 0 };
    int originalHeight = 0;
    int fixedHeight = 0; // for px
    float percentHeight = 0.0f; // for %

    bool needResize = false;

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

    bool isVisible() { return visibilityContext.visible; }
    void setVisible(bool visible) { visibilityContext.visible = visible; }

    void config(nlohmann::json& config)
    {
        // logDebug("config container %s", config.dump().c_str());
        visibilityContext.init(config);
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
                    [this](float, void* data) { 
                        if (isVisible()) onUpdate((ValueInterface*)data); 
                    },
                    value);
            }
        }
        initCounter++;
    }

    void pushToRenderingQueue(void* component)
    {
        if (!isVisible()) return;
        componentsToRender.push_back((ComponentInterface*)component);
    }

    bool onContext(uint8_t index, float value)
    {
        bool res = visibilityContext.onContext(index, value);
        if (!isVisible()) return false;
        for (auto& component : components) {
            component->onContext(index, value);
            if (res) pushToRenderingQueue(component);
        }
        return res;
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
        if (!isVisible()) return;

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
        if (!isVisible()) return;
        for (auto& component : components) {
            if (component->isVisible()) {
                component->handleMotion(motion);
            }
        }
    }

    void onMotionRelease(MotionInterface& motion)
    {
        if (!isVisible()) return;
        for (auto& component : components) {
            if (component->isVisible()) {
                component->handleMotionRelease(motion);
            }
        }
    }

    void onEncoder(int8_t id, int8_t direction, uint64_t tick)
    {
        if (!isVisible()) return;
        for (auto& component : components) {
            if (component->isVisible()) {
                component->onEncoder(id, direction);
            }
        }
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        if (!isVisible()) return;
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
        if (!isVisible()) return -2;
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
