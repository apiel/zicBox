#ifndef _VIEW_INTERFACE_COMPONENT_H_
#define _VIEW_INTERFACE_COMPONENT_H_

#include <string>
#include <vector>
#include <functional>

#include "./drawInterface.h"

class ViewInterface {
public:
    DrawInterface& draw;
    std::function<void(std::string name)> setView;
    float *contextVar;
    int8_t activeGroup = 0;
    int8_t track = -1;

    ViewInterface(DrawInterface& draw, std::function<void(std::string name)> setView, float *contextVar)
        : draw(draw)
        , setView(setView)
        , contextVar(contextVar)
    {
    }

    virtual void setGroup(int8_t index) = 0;
    virtual void* getContainer(std::string name) = 0;
    virtual void pushToRenderingQueue(void* component) = 0;
    virtual std::vector<void*>* getComponents() = 0;
    virtual void renderAllNext(void* container) = 0;
};

#endif
