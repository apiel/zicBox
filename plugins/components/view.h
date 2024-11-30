#ifndef _VIEW_COMPONENT_H_
#define _VIEW_COMPONENT_H_

#include "./componentInterface.h"
#include "./container/ComponentContainer.h"
#include "./container/containers.h"

#include <mutex>

class View : public ComponentContainer {
public:
    std::vector<ComponentInterface*> components = {};
    std::vector<ComponentInterface*> componentsToRender = {};
    std::vector<ComponentInterface*> componentsJob = {};
    std::vector<ComponentContainer*> containers = { this };
    DrawInterface& draw;
    std::mutex m2;

    View(DrawInterface& draw)
        : draw(draw)
    {
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
        m2.lock();
        for (auto& component : components) {
            component->onKey(id, key, state);
        }
        m2.unlock();
    }

    bool config(char* key, char* value)
    {
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
            ComponentContainer* container = newContainer(type, name, position, size);
            if (container != NULL) {
                containers.push_back(container);
            } else {
                logWarn("Unknown container: %s", type.c_str());
            }
            return true;
        }
        return false;
    }

    ComponentContainer* getContainer(string name) {
        for (auto& container : containers) {
            if (container->name == name) {
                return container;
            }
        }
        return NULL;
    }
};

#endif
