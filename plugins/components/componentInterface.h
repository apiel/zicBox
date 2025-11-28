/** Description:
This code defines the `ComponentInterface`, which is the fundamental blueprint or contract for creating any interactive element—such as a button, slider, or visual display—within a complex software system, likely an application dealing with audio, visuals, or control hardware.

**Basic Idea:**
This interface ensures that every single interactive component, regardless of its specific function, conforms to a standard set of requirements and has access to essential system tools. This standardization allows the entire application to manage complex screen elements reliably.

**How it Works:**

1.  **Setup (Properties):** When a component is created, it is initialized using a set of "Properties" (`Props`). These properties define the component's unique name, its size and location on the screen, and critical links back to the main system (like access to audio processing units, hardware controllers, and configuration data).

2.  **Tools and Connections:** Once built, the component holds direct references to vital system resources. It has access to drawing tools, styling rules, and essential callbacks to communicate with external hardware. For instance, it can request an external hardware controller or send a command to trigger an audio event.

3.  **Required Behavior:** The most critical part of this blueprint is a list of standardized actions. These actions are mandatory for every component and define how it interacts with the user and the system:
    *   **Rendering:** Functions like `render()` dictate how the component draws itself on the screen.
    *   **Input Handling:** Functions like `onMotion()` (for touch/mouse input), `onKey()` (for keyboard), and `onEncoder()` (for physical knobs) define how the element reacts when a user interacts with it.
    *   **Lifecycle Management:** Functions like `resize()` ensure the component can adapt if the view changes size.

In essence, the `ComponentInterface` provides the necessary structure and tools for any UI element to be a functional, independent part of the larger framework.

sha: cefeaa72ba1e7d7d52e40a3617144d61c1c19e6ae9330b859a57c7a723df7f67 
*/
#pragma once

#include "./ViewInterface.h"
#include "./drawInterface.h"
#include "./motionInterface.h"
#include "./valueInterface.h"
#include "plugins/controllers/controllerInterface.h"

#include "libs/nlohmann/json.hpp"
#include <functional>
#include <string.h>
#include <vector>

class ComponentInterface {
public:
    struct Props {
        std::string nameUID;
        nlohmann::json& config;
        Point position;
        Size size;
        AudioPlugin& (*getPlugin)(std::string name, int16_t track);
        void (*sendAudioEvent)(AudioEventType event, int16_t track);
        ControllerInterface* (*getController)(const char* name);
        ViewInterface* view;
        std::function<void(uint8_t index, float value)> setContext;
    };

    DrawInterface& draw;
    Styles& styles;
    ViewInterface* view;
    AudioPlugin& (*getPlugin)(std::string name, int16_t track);
    ControllerInterface* (*getController)(const char* name);
    void (*sendAudioEvent)(AudioEventType event, int16_t track);
    std::function<void(uint8_t index, float value)> setContext;
    std::vector<ValueInterface*> values;
    Point position;
    Point relativePosition = { 0, 0 };
    Size size;
    Size sizeOriginal;
    int16_t track = 0;
    std::string nameUID;

    std::function<void(unsigned long now)> jobRendering;

    ComponentInterface(Props props)
        : draw(props.view->draw)
        , styles(props.view->draw.styles)
        , nameUID(props.nameUID)
        , getPlugin(props.getPlugin)
        , sendAudioEvent(props.sendAudioEvent)
        , getController(props.getController)
        , view(props.view)
        , setContext(props.setContext)
        , position(props.position)
        , relativePosition(props.position)
        , size(props.size)
        , sizeOriginal(props.size)
        , track(props.view->track)
    {
        // printf("ComponentInterface: %d x %d\n", props.position.x, props.position.y);
    }

    virtual void clear() = 0;
    virtual void render() = 0;
    virtual void renderNext() = 0;
    virtual void initView(uint16_t counter) = 0;
    virtual void onMotion(MotionInterface& motion) = 0;
    virtual void handleMotion(MotionInterface& motion) = 0; // <--- should this go away?
    virtual void onMotionRelease(MotionInterface& motion) = 0;
    virtual void handleMotionRelease(MotionInterface& motion) = 0; // <--- should this go away?
    virtual void onEncoder(int8_t id, int8_t direction) = 0;
    virtual bool onKey(uint16_t id, int key, int8_t state, unsigned long now) = 0;
    virtual void onContext(uint8_t index, float value) = 0;
    virtual void onUpdate(ValueInterface* value) = 0;
    virtual bool isVisible() = 0;
    virtual void resize() = 0;
    virtual void resize(float xFactor, float yFactor, Point containerPosistion) = 0;
    virtual void* data(int id, void* userdata = NULL)
    {
        return NULL;
    }

    virtual const std::vector<EventInterface::EncoderPosition> getEncoderPositions() = 0;
};
