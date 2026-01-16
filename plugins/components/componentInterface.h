/** Description:
This code defines the foundational blueprint, or "interface," for every interactive or visual element (called a "Component") within a complex software system, likely an application with a graphical user interface and audio capabilities.

The core purpose of this interface is standardization. It dictates the mandatory data structure and set of behaviors that any component—such as a button, a display, or a slider—must possess to function correctly within the overall framework.

When a component is created, it receives a detailed initialization package (called `Props`). This package provides essential context, including: its unique ID, its physical position and size, configuration data, access to the primary view and drawing engine, and crucial connections needed to communicate with audio plugins and external hardware controllers.

The component stores this vital information (position, size, unique name, and drawing styles).

Crucially, the interface mandates a comprehensive set of actions (virtual functions) that every component must implement. These required actions ensure seamless integration and cover all aspects of component lifecycle, including:
1.  **Rendering:** Routines to draw itself onto the screen.
2.  **Interaction:** Methods to respond to all user inputs (such as mouse movement, key presses, or turning encoder knobs).
3.  **Data Management:** Functions to handle updates to its controlled values.
4.  **Layout:** The ability to resize and adjust its display if the main application window changes.

In essence, this file acts as the rigid rulebook, ensuring that all building blocks of the application are compatible and can communicate efficiently with each other and the underlying audio system.

sha: 9323cf3c4420985e078dcd61bba3b37ef793ee70f8edefe9e68e94278c1e8b2c
*/
#pragma once

#include "./ViewInterface.h"
#include "draw/drawInterface.h"
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
        ControllerInterface* (*getController)(const char* name);
        ViewInterface* view;
        std::function<void(uint8_t index, float value)> setContext;
        AudioPluginHandlerInterface* audioPluginHandler;
    };

    DrawInterface& draw;
    Styles& styles;
    ViewInterface* view;
    ControllerInterface* (*getController)(const char* name);
    std::function<void(uint8_t index, float value)> setContext;
    AudioPluginHandlerInterface* audioPluginHandler;
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
        , getController(props.getController)
        , view(props.view)
        , setContext(props.setContext)
        , audioPluginHandler(props.audioPluginHandler)
        , position(props.position)
        , relativePosition(props.position)
        , size(props.size)
        , sizeOriginal(props.size)
        , track(props.view->track)
    {
        // printf("ComponentInterface: %d x %d\n", props.position.x, props.position.y);
    }

    AudioPlugin& getPlugin(std::string name, int16_t track)
    {
        return audioPluginHandler->getPlugin(name, track);
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
