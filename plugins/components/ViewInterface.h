#ifndef _VIEW_INTERFACE_COMPONENT_H_
#define _VIEW_INTERFACE_COMPONENT_H_

#include "./container/ComponentContainer.h"
#include "./container/containers.h"

#include <string>
#include <vector>
// #include <iterator>

class ViewInterface : public ComponentContainer {
public:
    std::vector<ComponentContainer*> containers = { this };
    DrawInterface& draw;

    ViewInterface(DrawInterface& draw)
        : draw(draw)
    {
    }

    virtual void setGroup(int8_t index) = 0;
    virtual ComponentContainer* getContainer(std::string name) = 0;
    virtual void pushToRenderingQueue(void* component) = 0;
    virtual std::vector<void*>* getComponents() = 0;
};

#endif
