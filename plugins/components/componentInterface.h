#ifndef _UI_COMPONENT_INTERFACE_H_
#define _UI_COMPONENT_INTERFACE_H_

#include "./ViewInterface.h"
#include "./container/ComponentContainer.h"
#include "./drawInterface.h"
#include "./motionInterface.h"
#include "./valueInterface.h"
#include "plugins/controllers/controllerInterface.h"

#include <functional>
#include <string.h>
#include <vector>

class ComponentInterface {
public:
    struct Props {
        ComponentContainer* container;
        Point position;
        Size size;
        AudioPlugin& (*getPlugin)(const char* name, int16_t track);
        void (*sendAudioEvent)(AudioEventType event);
        ControllerInterface* (*getController)(const char* name);
        ViewInterface* view;
        std::function<void(uint8_t index, uint8_t value)> setShift;
    };

protected:
    DrawInterface& draw;
    Styles& styles;

    void (*sendAudioEvent)(AudioEventType event);
    ControllerInterface* (*getController)(const char* name);
    ViewInterface* view;
    std::function<void(uint8_t index, uint8_t value)> setShift;
    Point relativePosition = { 0, 0 };

    Props getNewPropsRect(Props props, Rect rect)
    {
        props.position = rect.position;
        props.size = rect.size;
        return props;
    }

    bool updatePosition()
    {
        return container->updateCompontentPosition(position, size, relativePosition);
    }

public:
    ComponentContainer* container;
    AudioPlugin& (*getPlugin)(const char* name, int16_t track);
    std::vector<ValueInterface*> values;
    Point position;
    Size size;
    int16_t track = -1;
    int8_t group = -1;
    bool active = true;
    std::string id = "";

    std::function<void(unsigned long now)> jobRendering;

    ComponentInterface(Props props)
        : draw(props.view->draw)
        , styles(props.view->draw.styles)
        , container(props.container)
        , getPlugin(props.getPlugin)
        , sendAudioEvent(props.sendAudioEvent)
        , getController(props.getController)
        , view(props.view)
        , setShift(props.setShift)
        , position(props.position)
        , size(props.size)
    {
        // printf("ComponentInterface: %d x %d\n", props.position.x, props.position.y);
    }

    virtual void clear() = 0;
    virtual void render() = 0;
    virtual void renderNext() = 0;
    virtual void initView(uint16_t counter) = 0;
    virtual void onMotion(MotionInterface& motion) = 0;
    virtual void handleMotion(MotionInterface& motion) = 0;
    virtual void onMotionRelease(MotionInterface& motion) = 0;
    virtual void handleMotionRelease(MotionInterface& motion) = 0;
    virtual void onEncoder(int id, int8_t direction) = 0;
    virtual void onKey(uint16_t id, int key, int8_t state) = 0;
    virtual bool config(char* key, char* value) = 0;
    virtual bool baseConfig(char* key, char* value) = 0;
    virtual void onGroupChanged(int8_t index) = 0;
    virtual void onShift(uint8_t index, uint8_t value) = 0;
    virtual void onUpdate(ValueInterface* value) = 0;
    virtual void* data(int id, void* userdata = NULL)
    {
        return NULL;
    }
};

#endif
