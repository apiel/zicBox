#ifndef _UI_PLUGIN_H_
#define _UI_PLUGIN_H_

// #include "plugins/valueInterface.h"
#include "plugins/audio/mapping.h"
#include "plugins/components/componentInterface.h"

#include <cstdio> // printf

/**
 * Simulate audio plugin component to be usable within the UI component
 */
class UiPlugin : public Mapping {
protected:
    void (*onUpdatePtr)() = []() {};

    uint16_t initViewCounter = 0;

    struct SharedComponent {
        std::string name;
        ComponentInterface* component;
    };
    std::vector<SharedComponent> sharedComponents;

    struct View {
        std::string name;
        std::vector<ComponentInterface*> components = {};
        std::vector<ComponentInterface*> componentsToRender = {};
        std::vector<ComponentInterface*> componentsJob = {};
        bool hidden = false;
    };

    View* lastView = NULL;

    std::vector<View*> views;

    static UiPlugin* instance;
    AudioPlugin::Props props = { NULL, 0, 0, NULL, 0, NULL };
    UiPlugin()
        : Mapping(props, (char*)"UI")
    {
    }

    void setView(int index)
    {
        if (index < 0 || index >= views.size()) {
            return;
        }

        if (view != views[index]) {
            if (lastView != view) {
                lastView = view;
            }
            view = views[index];

            if (!view->hidden) {
                viewSelector.setString(view->name);

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
    }

public:
    View* view = NULL;

    Val& viewSelector = val(1.0f, "VIEW", { "View", VALUE_STRING, .min = 1.0 }, [&](auto p) { setView(p.value); });

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

    UiPlugin& setView(std::string value)
    {
        if (value == "&previous") {
            value = lastView->name;
        }
        for (int i = 0; i < views.size(); i++) {
            if (views[i]->name == value) {
                setView(i);
                return *this;
            }
        }
        printf("Unknown view: %s\n", value.c_str());
        return *this;
    }

    int getViewCount()
    {
        return views.size();
    }

    void pushToRenderingQueue(ComponentInterface* component)
    {
        view->componentsToRender.push_back(component);
    }

    bool config(char* key, char* value)
    {
/*md
### VIEW

The user interface is composed of multiple views that contain the components. A view, represent a full screen layout. Use `VIEW: name_of_the_veiw` to create a view. All the following `COMPONENT: ` will be assign to this view, till the next view.

```coffee
# VIEW: ViewName

VIEW: Main

# some components...

VIEW: Mixer

# some components...
# ...
```

In some case, we need to create some hidden view. Those hidden views can be useful when defining a layout that is re-used in multiple view. It might also be useful, when a view have multiple state (e.g. shifted view...). In all those case, we do not want those view to be iterable. To define a hidden view, set `HIDDEN` flag after the view name.

```coffee
VIEW: Layout HIDDEN

# some components...
```
*/
        if (strcmp(key, "VIEW") == 0) {
            View* v = new View;
            v->name = strtok(value, " ");

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

            printf("Set VIEW: %s hidden=%s (max %d)\n", v->name.c_str(), v->hidden ? "true" : "false", max);

            setView(1.0f);

            return true;
        }

/*md
### STARTUP_VIEW

`STARTUP_VIEW` can be used to load a specific view on startup. This command should only be call at the end of the configuration file, once all the view has been initialised.

```coffee
#STARTUP_VIEW: ViewName

STARTUP_VIEW: Mixer
```

If `STARTUP_VIEW` is not defined, the first defined view (not `HIDDEN`) will be displayed.
*/
        if (strcmp(key, "STARTUP_VIEW") == 0) {
            setView(value);
        }

        if (strcmp(key, "USE_SHARED_COMPONENT") == 0) {
            for (auto& shared : sharedComponents) {
                if (shared.name == value) {
                    addComponent(shared.component);
                    return true;
                }
            }
            printf("ERROR: Shared component not found: %s\n", value);
        }

        if (views.size() > 0 && views.back()->components.size() > 0) {
            if (strcmp(key, "SHARED_COMPONENT") == 0) {
                SharedComponent shared;
                shared.name = value;
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
            if (component->jobRendering) {
                views.back()->componentsJob.push_back(component);
            }
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
        for (auto& component : view->components) {
            component->initView(initViewCounter);
            component->renderNext();
            for (auto* value : component->values) {
                value->onUpdate(callback, value);
            }
        }
        initViewCounter++;
    }
};

UiPlugin* UiPlugin::instance = NULL;

#endif