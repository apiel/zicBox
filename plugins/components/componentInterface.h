#ifndef _UI_COMPONENT_INTERFACE_H_
#define _UI_COMPONENT_INTERFACE_H_

#include "../controllers/controllerInterface.h"
#include "drawInterface.h"
#include "motionInterface.h"
#include "valueInterface.h"

#include <functional>
#include <string.h>
#include <vector>

class ComponentInterface {
public:
    struct Props {
        Point position;
        Size size;
        DrawInterface& draw;
        AudioPlugin& (*getPlugin)(const char* name, int16_t track);
        ControllerInterface* (*getController)(const char* name);
        std::vector<ComponentInterface*> (*getViewComponents)();
        void (*setGroup)(int8_t index);
        void (*setVisibility)(int8_t index);
        void (*setView)(std::string name);
        void (*pushToRenderingQueue)(ComponentInterface* component);
    };

protected:
    DrawInterface& draw;
    Styles& styles;

    AudioPlugin& (*getPlugin)(const char* name, int16_t track);
    ControllerInterface* (*getController)(const char* name);
    std::vector<ComponentInterface*> (*getViewComponents)();
    void (*setGroup)(int8_t index);
    void (*setVisibility)(int8_t index);
    void (*setView)(std::string name);
    void (*pushToRenderingQueue)(ComponentInterface* component);

    Props getNewPropsRect(Props props, Rect rect)
    {
        props.position = rect.position;
        props.size = rect.size;
        return props;
    }

public:
    std::vector<ValueInterface*> values;
    Point position;
    Size size;
    int16_t track = -1;
    int8_t group = -1;
    int8_t visibility = -1;
    bool active = true;
    std::string id = "";

    std::function<void(unsigned long now)> jobRendering;

    ComponentInterface(Props props)
        : draw(props.draw)
        , styles(props.draw.styles)
        , getPlugin(props.getPlugin)
        , getController(props.getController)
        , getViewComponents(props.getViewComponents)
        , setGroup(props.setGroup)
        , setVisibility(props.setVisibility)
        , setView(props.setView)
        , pushToRenderingQueue(props.pushToRenderingQueue)
        , position(props.position)
        , size(props.size)
    {
        // printf("ComponentInterface: %d x %d\n", props.position.x, props.position.y);
    }

    virtual void render() = 0;
    virtual void renderNext() = 0;
    virtual void initView(uint16_t counter) = 0;
    virtual void onMotion(MotionInterface& motion) = 0;
    virtual void handleMotion(MotionInterface& motion) = 0;
    virtual void onMotionRelease(MotionInterface& motion) = 0;
    virtual void handleMotionRelease(MotionInterface& motion) = 0;
    virtual void onEncoder(int id, int8_t direction) = 0;
    virtual void onKeyPad(int id, int8_t state) = 0;
    virtual bool config(char* key, char* value) = 0;
    virtual bool baseConfig(char* key, char* value) = 0;
    virtual void onGroupChanged(int8_t index) = 0;
    virtual void onUpdate(ValueInterface* value) = 0;
    virtual void onVisibilityChanged(int8_t index) = 0;
    virtual void* data(int id, void* userdata = NULL)
    {
        return NULL;
    }
};

#endif
