#ifndef _UI_PLUGIN_H_
#define _UI_PLUGIN_H_

// #include "plugins/valueInterface.h"
#include "plugins/audio/mapping.h"
#include "plugins/components/componentInterface.h"

#include <cstdio> // printf

/**
 * Simulate audio plugin component to be usable within the UI component
 */
class UiPlugin : public Mapping<UiPlugin> {
protected:
    void (*onUpdatePtr)() = []() {};

    struct SharedComponent {
        char* name;
        ComponentInterface* component;
    };
    std::vector<SharedComponent> sharedComponents;

    struct View {
        char* name;
        std::vector<ComponentInterface*> components = {};
        std::vector<ComponentInterface*> componentsToRender = {};
        bool hidden = false;
    };

    View* lastView = NULL;
    View* currentView = NULL;

    std::vector<View*> views;

    static UiPlugin* instance;
    AudioPlugin::Props props = { NULL, 0, 0, NULL, 0 };
    UiPlugin()
        : Mapping(props, (char*)"UI")
    {
    }

    void setView(int index)
    {
        if (index < 0 || index >= views.size()) {
            return;
        }

        lastView = currentView;
        currentView = views[index];

        if (!currentView->hidden) {
            viewSelector.setString(currentView->name);

            int value = 1;
            for (int i = 0; i < index; i++) {
                if (!views[i]->hidden) {
                    value++;
                }
            }
            viewSelector.setFloat((float)value);
        }

        onUpdatePtr();
    }

public:
    Val<UiPlugin>& viewSelector = val(this, 1.0f, "VIEW", &UiPlugin::setView, { "View", VALUE_STRING, .min = 1.0 });

    static UiPlugin& get()
    {
        if (!instance) {
            instance = new UiPlugin();
        }
        return *instance;
    }

    void sample(float* buf) { }

    void setUpdateCallback(void (*callback)())
    {
        onUpdatePtr = callback;
    }

    UiPlugin& setView(float value)
    {
        if (value < viewSelector.props().min || value > viewSelector.props().max) {
            return *this;
        }
        int visible = 0;
        for (int i = 0; i < views.size(); i++) {
            if (!views[i]->hidden) {
                visible++;
            }
            if (visible == (int)value) {
                setView(i);
                break;
            }
        }
        return *this;
    }

    UiPlugin& setView(char* value)
    {
        if (strcmp(value, "&previous") == 0) {
            value = lastView->name;
        }
        for (int i = 0; i < views.size(); i++) {
            if (strcmp(views[i]->name, value) == 0) {
                setView(i);
                break;
            }
        }
        return *this;
    }

    int getViewCount()
    {
        return views.size();
    }

    std::vector<ComponentInterface*>& getComponents()
    {
        return currentView->components;
    }

    std::vector<ComponentInterface*>& getComponentsToRender()
    {
        return currentView->componentsToRender;
    }

    void pushToRenderingQueue(ComponentInterface* component)
    {
        currentView->componentsToRender.push_back(component);
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "VIEW") == 0) {
            View* v = new View;

            char* name = strtok(value, " ");
            v->name = new char[strlen(name) + 1];
            strcpy(v->name, name);

            char* hidden = strtok(NULL, " ");
            if (hidden != NULL && strcmp(hidden, "HIDDEN") == 0) {
                v->hidden = true;
            }

            views.push_back(v);

            uint16_t max = 0;
            for (auto& view : views) {
                if (!view->hidden) {
                    max++;
                }
            }
            viewSelector.props().max = (float)max;

            printf("Set VIEW: %s hidden=%s (max %d)\n", v->name, v->hidden ? "true" : "false", max);

            setView(1.0f);

            return true;
        }

        if (strcmp(key, "USE_SHARED_COMPONENT") == 0) {
            for (auto& shared : sharedComponents) {
                if (strcmp(shared.name, value) == 0) {
                    views.back()->components.push_back(shared.component);
                    return true;
                }
            }
        }

        if (views.size() > 0 && views.back()->components.size() > 0) {
            if (strcmp(key, "SHARED_COMPONENT") == 0) {
                SharedComponent shared;
                shared.name = new char[strlen(value) + 1];
                strcpy(shared.name, value);
                shared.component = views.back()->components.back();
                sharedComponents.push_back(shared);
            }

            return views.back()->components.back()->baseConfig(key, value);
        }

        return false;
    }

    void addComponent(ComponentInterface* component)
    {
        if (views.size() > 0) {
            views.back()->components.push_back(component);
        } else {
            printf("ERROR: No view to add component to. Create first a view to be able to add components.\n");
        }
    }

    void clearOnUpdate()
    {
        if (lastView != NULL) {
            for (auto& component : lastView->components) {
                for (auto* value : component->values) {
                    value->onUpdate([](float, void* data) {}, NULL);
                }
            }
        }
    }

    void initActiveComponents(void (*callback)(float, void* data))
    {
        for (auto& component : getComponents()) {
            component->renderNext();
            for (auto* value : component->values) {
                value->onUpdate(callback, value);
            }
        }
    }
};

UiPlugin* UiPlugin::instance = NULL;

#endif