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
    struct SharedComponent {
        char* name;
        ComponentInterface* component;
    };
    std::vector<SharedComponent> sharedComponents;

    struct View {
        char* name;
        std::vector<ComponentInterface*> view = {};
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
        lastView = currentView;
        currentView = views[index];

        if (!currentView->hidden) {
            // FIXME viewSelector.setFloat(getIndex());
            viewSelector.setString(currentView->name);
        }
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

    UiPlugin& setView(float value)
    {
        viewSelector.setFloat(value); // FIXME this should happem in setView(int index)
        setView((int)viewSelector.get() - 1);

        return *this;
    }

    UiPlugin& setView(char* value)
    {
        for (int i = 0; i < views.size(); i++) {
            if (strcmp(views[i]->name, value) == 0) {
                // printf("................... Set view to %s at index %d\n", value, i);
                viewSelector.set((float)(i + 1));
                break;
            }
        }
        return *this;
    }

    int getViewCount()
    {
        return views.size();
    }

    std::vector<ComponentInterface*>& getView()
    {
        return currentView->view;
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "VIEW") == 0) {
            View* v = new View;
            v->name = new char[strlen(value) + 1];
            strcpy(v->name, value);

            views.push_back(v);

            viewSelector.props().max = views.size();
            setView(1.0f);

            return true;
        }

        if (strcmp(key, "USE_SHARED_COMPONENT") == 0) {
            for (auto& shared : sharedComponents) {
                if (strcmp(shared.name, value) == 0) {
                    views.back()->view.push_back(shared.component);
                    return true;
                }
            }
        }

        if (views.size() > 0 && views.back()->view.size() > 0) {
            if (strcmp(key, "SHARED_COMPONENT") == 0) {
                SharedComponent shared;
                shared.name = new char[strlen(value) + 1];
                strcpy(shared.name, value);
                shared.component = views.back()->view.back();
                sharedComponents.push_back(shared);
            }

            return views.back()->view.back()->baseConfig(key, value);
        }

        return false;
    }

    void addComponent(ComponentInterface* component)
    {
        if (views.size() > 0) {
            views.back()->view.push_back(component);
        } else {
            printf("ERROR: No view to add component to. Create first a view to be able to add components.\n");
        }
    }

    void clearOnUpdate()
    {
        for (auto& component : lastView->view) {
            for (auto* value : component->values) {
                value->onUpdate([](float, void* data) {}, NULL);
            }
        }
    }

    void initActiveComponents(void (*callback)(float, void* data))
    {
        for (auto& component : getView()) {
            component->renderNext();
            for (auto* value : component->values) {
                value->onUpdate(callback, value);
            }
        }
    }
};

UiPlugin* UiPlugin::instance = NULL;

#endif