/** Description:
This foundational code serves as the abstract blueprint, or "standard template," for every interactive item—like a button, dial, or display area—within a complex graphical user interface (GUI) or real-time system.

It establishes a universal contract (`ComponentInterface`) that all visual elements must adhere to. This ensures they can all be managed and interacted with uniformly by the system.

Each component defined by this template is equipped with essential properties: tracking its physical location, size, and unique identification. It holds the necessary connections to system resources for rendering itself (drawing features), managing data values, and integrating with external hardware.

Crucially, it defines standardized methods for handling all user interactions and system events. These include refreshing the display (`render`), responding to movement (touch/mouse input), reacting to key presses, processing input from rotary dials (encoders), and receiving updates from connected audio or control systems. This highly modular approach ensures predictable behavior across the entire application.

sha: dc57677c2b2c069eb51192f640ec9a16921c34790cc8660c3b6e7f807a3bd778 
*/
#pragma once

#include "./ComponentContainer.h"
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
    virtual void resize(float xFactor, float yFactor) = 0;
    virtual void* data(int id, void* userdata = NULL)
    {
        return NULL;
    }

    virtual const std::vector<EventInterface::EncoderPosition> getEncoderPositions() = 0;
};
