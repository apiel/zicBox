#ifndef _VIEW_COMPONENT_H_
#define _VIEW_COMPONENT_H_

#include "componentInterface.h"
#include "drawInterface.h"

class ComponentContainer {
public:
    Point position = { 0, 0 };
};

class View : public ComponentContainer {
public:
    std::string name;
    std::vector<ComponentInterface*> components = {};
    std::vector<ComponentInterface*> componentsToRender = {};
    std::vector<ComponentInterface*> componentsJob = {};

    uint16_t initViewCounter = 0;
    void initActiveComponents()
    {
        for (auto& component : components) {
            component->initView(initViewCounter);
            component->renderNext();
            for (auto* value : component->values) {
                value->onUpdate(
                    [this](float, void* data) { onUpdate((ValueInterface*)data); },
                    value);
            }
        }
        initViewCounter++;
    }

    // TODO could this be optimized by creating mapping values to components?
    void onUpdate(ValueInterface* val)
    {
        for (auto& component : components) {
            for (auto* value : component->values) {
                if (value == val) {
                    component->onUpdate(value);
                }
            }
        }
    }
};

#endif
