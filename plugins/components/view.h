/** Description:
This C++ header file defines an abstract blueprint, or template, for creating specific user interface screens, named `View`. It does not contain executable logic itself, but rather establishes a rigid set of rules that all screens in the system must follow.

The primary purpose of this blueprint is to manage two key roles simultaneously: Display Management and User Interaction.

This `View` acts as a central control panel that every functioning screen must implement:

1.  **Visual Layout:** It handles the organization and drawing of smaller sub-elements called "Components" and "Containers," ensuring they are rendered correctly to the display. It dictates the process for adding, managing, and drawing these constituent parts.
2.  **System Lifecycle:** It defines necessary stages for the screen's existence, such as specific functions for initial setup and activation (when the screen becomes visible).
3.  **Input Handling:** Crucially, the `View` is the primary listener for all user inputs. It contains dedicated functions to react to various actions, including touch gestures, physical buttons, rotary dials, and contextual changes within the application.

By using this detailed blueprint, developers ensure every new screen added to the system adheres to a consistent structure, guaranteeing proper communication with the rendering engine and reliable response to all input devices.

sha: ed1749907c9cdee8acf18445bff57620b014d6bac4c2b8f76ff53d344291d1f7 
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
    virtual Container* addContainer(std::string& name, Point position) = 0;
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
