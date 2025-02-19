#pragma once

#include "./ViewInterface.h"
#include "./ComponentContainer.h"
#include "./drawInterface.h"
#include "./motionInterface.h"
#include "./valueInterface.h"
#include "plugins/controllers/controllerInterface.h"

#include <functional>
#include "libs/nlohmann/json.hpp"
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
        void (*sendAudioEvent)(AudioEventType event);
        ControllerInterface* (*getController)(const char* name);
        ViewInterface* view;
        std::function<void(uint8_t index, float value)> setContext;
    };

protected:
    DrawInterface& draw;
    Styles& styles;

public:
    ViewInterface* view;
    AudioPlugin& (*getPlugin)(std::string name, int16_t track);
    ControllerInterface* (*getController)(const char* name);
    void (*sendAudioEvent)(AudioEventType event);
    std::function<void(uint8_t index, float value)> setContext;
    std::vector<ValueInterface*> values;
    Point position;
    Point relativePosition = { 0, 0 };
    Size size;
    int16_t track = 0;
    int8_t group = -1;
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
        , track(props.view->track)
    {
        // printf("ComponentInterface: %d x %d\n", props.position.x, props.position.y);
    }

    virtual void clear() = 0; // Doesnt seems to be used... to be deprecated ?
    virtual void render() = 0;
    virtual void renderNext() = 0;
    virtual void initView(uint16_t counter) = 0;
    virtual void onMotion(MotionInterface& motion) = 0;
    virtual void handleMotion(MotionInterface& motion) = 0; // <--- should this go away?
    virtual void onMotionRelease(MotionInterface& motion) = 0;
    virtual void handleMotionRelease(MotionInterface& motion) = 0; // <--- should this go away?
    virtual void onEncoder(int id, int8_t direction) = 0;
    virtual void onKey(uint16_t id, int key, int8_t state, unsigned long now) = 0;
    virtual void onGroupChanged(int8_t index) = 0;
    virtual void onContext(uint8_t index, float value) = 0;
    virtual void onUpdate(ValueInterface* value) = 0;
    virtual bool isVisible() = 0;
    virtual void resize() = 0;
    virtual void* data(int id, void* userdata = NULL)
    {
        return NULL;
    }
};
