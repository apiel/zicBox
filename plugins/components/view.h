#ifndef _VIEW_COMPONENT_H_
#define _VIEW_COMPONENT_H_

#include "drawInterface.h"
#include "componentInterface.h"

class ComponentContainer {
public:
    Point position = {0, 0};
};

class View : public ComponentContainer {
public:
    std::string name;
    std::vector<ComponentInterface*> components = {};
    std::vector<ComponentInterface*> componentsToRender = {};
    std::vector<ComponentInterface*> componentsJob = {};
};

#endif
