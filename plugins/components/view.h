/** Description:
This file defines the foundational blueprint for a user interface screen or application page, known as `View`. It is a crucial abstract structure that dictates the rules for how any specific screen in the system must behave.

**Core Purpose:**

The `View` class acts as a high-level manager, combining two major roles: defining the visual structure (how elements are drawn) and handling all forms of user input.

**How it Works (The Blueprint):**

The `View` itself contains no actual running code; instead, it lists all the essential functions that any concrete screen must implement. This ensures consistency across the entire application.

1.  **Dual Responsibility:** It inherits capabilities for drawing elements and handling events, meaning every screen must be able to display items and react to user interactions simultaneously.
2.  **Lifecycle Management:** It defines mandatory steps for screen startup (`init`) and running (`activate`).
3.  **Visual Layout:** It specifies how visual building blocks (called Components and Containers) are added, organized, and drawn onto the screen.
4.  **Input Handling:** It includes specific requirements for reacting to almost every possible user action, such as movements (`onMotion`), button presses (`onKey`), and interactions with rotary dials or knobs (`onEncoder`).

In summary, this `View` header enforces a standardized design, ensuring that developers build their screens (plugins or applications) correctly so they can integrate seamlessly into the overall system architecture.

sha: 1bfab765aff0ff1cf5615bf9c69571699f3cfe01a54efff3456b51c5ad70b5df 
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
    virtual Container* addContainer(std::string& name, Point position, Size size, uint8_t resizeType) = 0;
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
    virtual void resize(float xFactor, float yFactor) override = 0;

    // encoder API
    virtual int8_t getEncoderId(int32_t x, int32_t y, bool isMotion = false) override = 0;
    virtual const std::vector<EventInterface::EncoderPosition> getEncoderPositions() = 0;
};
