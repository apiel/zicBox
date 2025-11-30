/** Description:
This document defines the essential blueprint for a single, interactive screen or menu within a larger system. This structure is called a "View."

The core purpose of this View is twofold: it manages the visual display elements, and it acts as the primary receiver for all user interactions.

When a View is created, it requires essential tools: a mechanism for drawing graphics, a function to navigate and switch to other screens, and access to shared system variables.

**Its key responsibilities are defined by its lifecycle and management duties:**

1.  **Lifecycle Management:** It specifies clear setup procedures for when the screen first loads (`init`) and when it becomes the active, visible screen (`activate`).
2.  **UI Component Management:** The View is responsible for organizing all the smaller visual elements (called Components) that reside within it, such as buttons or sliders. It manages dedicated grouping areas (Containers) and ensures all these internal components are drawn correctly and updated over time.
3.  **User Input Handling:** As the central event processor, the View is designed to immediately react to all forms of user interaction. This includes touch gestures or mouse movements, the rotation of physical input dials (encoders), and keyboard presses. It also handles system events like screen resizing.

In essence, this blueprint ensures that any functional screen built upon it strictly follows rules for managing its contents and responding dynamically to the user.

sha: 8f859293ca6b297cdec2f7e9da94aadee998fae1ee817a8b8e25bcf2d021c254 
*/
#pragma once

#include <functional>
#include <string>
#include <vector>

#include "plugins/components/EventInterface.h"
#include "plugins/components/ViewInterface.h"
#include "plugins/components/componentInterface.h"
#include "plugins/components/container.h"

class View : public ViewInterface, public EventInterface {
public:
    std::string name;

    View(DrawInterface& draw,
        std::function<void(std::string)> setView,
        float* contextVar)
        : ViewInterface(draw, setView, contextVar)
    {
    }

    // lifecycle
    virtual void init() = 0;
    virtual void activate() = 0;

    // components
    virtual std::vector<ComponentInterface*>& getComponents() = 0;
    virtual Container* addContainer(std::string& name, Point position, std::string height) = 0;
    virtual void addComponent(ComponentInterface* component, Container *container) = 0;
    virtual void renderComponents(unsigned long now) = 0;

    // UI events
    virtual void onContext(uint8_t index, float value) = 0;

    // override pure virtual from ViewInterface
    virtual void pushToRenderingQueue(void* component) override = 0;

    // override pure virtual from EventInterface
    virtual void onMotion(MotionInterface& motion) override = 0;
    virtual void onMotionRelease(MotionInterface& motion) override = 0;
    virtual void onEncoder(int8_t id, int8_t direction, uint64_t tick) override = 0;
    virtual void onKey(uint16_t id, int key, int8_t state) override = 0;
    virtual void resize() override = 0;

    // encoder API
    virtual int8_t getEncoderId(int32_t x, int32_t y, bool isMotion = false) override = 0;
    virtual const std::vector<EventInterface::EncoderPosition> getEncoderPositions() = 0;
};
