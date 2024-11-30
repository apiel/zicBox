#ifndef _VIEW_INTERFACE_COMPONENT_H_
#define _VIEW_INTERFACE_COMPONENT_H_

#include "./componentInterface.h"
#include "./container/ComponentContainer.h"
#include "./container/containers.h"

#include <string>

class ViewInterface : public ComponentContainer {
public:
    std::vector<ComponentInterface*> components = {};
    std::vector<ComponentInterface*> componentsToRender = {};
    std::vector<ComponentInterface*> componentsJob = {};
    std::vector<ComponentContainer*> containers = { this };
    DrawInterface& draw;

    ViewInterface(DrawInterface& draw)
        : draw(draw)
    {
    }

    virtual void setGroup(int8_t index) = 0;
    virtual ComponentContainer* getContainer(std::string name) = 0;
};

#endif
