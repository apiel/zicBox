#ifndef _VIEW_COMPONENT_H_
#define _VIEW_COMPONENT_H_

#include "./ViewInterface.h"
#include "./container/containers.h"
#include "helpers/getTicks.h"

#include <mutex>

class View : public ViewInterface, public ComponentContainer {
public:
    std::vector<ComponentContainer*> containers = { this };
    std::vector<ComponentInterface*> components = {};
    std::vector<ComponentInterface*> componentsToRender = {};
    std::vector<ComponentInterface*> componentsJob = {};
    std::mutex m2;

    View(DrawInterface& draw, std::function<void(std::string name)> setView, float* contextVar)
        : ViewInterface(draw, setView, contextVar)
    {
    }

    int8_t lastGroupId = 0;
    void init()
    {
        for (auto& container : containers) {
            container->initContainer();
        }

        for (auto& component : components) {
            if (component->group > lastGroupId) {
                lastGroupId = component->group;
            }
        }
    }

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

    void pushToRenderingQueue(void* component)
    {
        componentsToRender.push_back((ComponentInterface*)component);
    }

    std::vector<void*>* getComponents() override
    {
        return (std::vector<void*>*)&components;
    }

    bool groupLoop = true;
    void setGroup(int8_t index) override
    {
        int8_t group;
        if (groupLoop) {
            group = index == -1 ? lastGroupId : index;
            if (group > lastGroupId) {
                group = 0;
            }
        } else {
            group = index == -1 ? 0 : index;
            if (group > lastGroupId) {
                group = lastGroupId;
            }
        }
        activeGroup = group;
        for (auto& container : containers) {
            container->onGroupChanged(group);
        }

        for (auto& component : components) {
            component->onGroupChanged(group);
        }
    }

    void onContext(uint8_t index, float value)
    {
        for (auto& container : containers) {
            if (container != this) {
                container->onContext(index, value);
            }
        }

        for (auto& component : components) {
            component->onContext(index, value);
        }
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

    void renderComponents(unsigned long now)
    {
        if (componentsJob.size()) {
            for (auto& component : componentsJob) {
                if (component->active) {
                    component->jobRendering(now);
                }
            }
        }

        if (componentsToRender.size()) {
            for (auto& component : componentsToRender) {
                if (component->active) {
                    component->render();
                }
            }
            componentsToRender.clear();
            draw.renderNext();
        }
        draw.triggerRendering();
    }

    void onMotion(MotionInterface& motion)
    {
        for (auto& component : components) {
            component->handleMotion(motion);
        }
    }

    void onMotionRelease(MotionInterface& motion)
    {
        for (auto& component : components) {
            component->handleMotionRelease(motion);
        }
    }

    // there should be about 4 to 12 encoders, however with 256 we are sure to not be out of bounds
    unsigned long lastEncoderTick[256] = { 0 };
    void onEncoder(int id, int8_t direction, uint32_t tick)
    {
        m2.lock();
        if (tick - lastEncoderTick[id] < 25) {
            direction = direction * 5;
        }
        lastEncoderTick[id] = tick;
        for (auto& component : components) {
            component->onEncoder(id, direction);
        }
        m2.unlock();
    }

    void onKey(uint16_t id, int key, int8_t state)
    {
        unsigned long now = getTicks();
        m2.lock();
        for (auto& component : components) {
            component->onKey(id, key, state, now);
        }
        m2.unlock();
    }

    void* getContainer(string name) override
    {
        for (auto& container : containers) {
            // printf("Container: %s == %s ?\n", container->name.c_str(), name.c_str());
            if (container->name == name) {
                return container;
            }
        }
        return NULL;
    }

    bool config(char* key, char* value)
    {
        // set default track value for all components contained in the view
        if (strcmp(key, "COMPONENTS_TRACK") == 0) {
            track = atoi(value);
            return true;
        }

        if (strcmp(key, "CONTAINER") == 0) {
            string type = strtok(value, " ");
            string name = strtok(NULL, " ");
            Point position = {
                atoi(strtok(NULL, " ")),
                atoi(strtok(NULL, " ")),
            };
            Size size = { 0, 0 };
            char* w = strtok(NULL, " ");
            if (w != NULL) {
                size.w = atoi(w);
                char* h = strtok(NULL, " ");
                if (h != NULL) {
                    size.h = atoi(h);
                }
            }
            ComponentContainer* container = newContainer(type, this, name, position, size);
            if (container != NULL) {
                logInfo("Adding container (%s): %s %dx%d %dx%d", type.c_str(), name.c_str(), position.x, position.y, size.w, size.h);
                containers.push_back(container);
            } else {
                logWarn("Unknown container: %s", type.c_str());
            }
            return true;
        }

        if (strcmp(key, "GROUP_LOOP") == 0) {
            groupLoop = strcmp(value, "true") == 0;
            return true;
        }

        if (containers.size() > 1 && containers.back()->config(key, value)) {
            return true;
        }

        return false;
    }
};

#endif
