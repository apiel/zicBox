#ifndef _VIEW_MANAGER_H
#define _VIEW_MANAGER_H

#include <dlfcn.h>
#include <mutex>
#include <vector>

#include "controllerList.h"
#include "helpers/getFullpath.h"
#include "host.h"
#include "log.h"
#include "plugins/components/componentInterface.h"
#include "plugins/components/view.h"
#include "styles.h"
#include "helpers/getTicks.h"

#ifdef DRAW_ST7789
#ifdef USE_DRAW_WITH_SDL
#include "draw/ST7789/drawWithSDL.h"
#else
#include "draw/ST7789/draw.h"
#endif
#elifdef DRAW_SSD1306
#ifdef USE_DRAW_WITH_SDL
#include "draw/SSD1306/drawWithSDL.h"
#else
#include "draw/SSD1306/draw.h"
#endif
#else
#include "draw/SDL/draw.h"
#endif

class ViewManager {
public:
    struct Plugin {
        string name;
        std::function<ComponentInterface*(ComponentInterface::Props props)> allocator;
    };
    std::vector<Plugin> plugins;

protected:
    struct SharedComponent {
        std::string name;
        ComponentInterface* component;
    };
    std::vector<SharedComponent> sharedComponents;

    View* lastView = NULL;

    std::vector<View*> views;

public:
    View* view = NULL;

    void setView(std::string value)
    {
        printf("set view string to %s\n", value.c_str());
        if (value == "&previous") {
            value = lastView->name;
        }
        for (int i = 0; i < views.size(); i++) {
            if (views[i]->name == value) {
                if (view != views[i]) {
                    if (lastView != view) {
                        lastView = view;
                    }
                    view = views[i];
                    view->setGroup(0);
                    for (int i = 0; i < 256; i++) {
                        view->onShift(i, shift[i]); 
                    }
                    render();
                }
                return;
            }
        }
        logWarn("Unknown view: %s\n", value.c_str());
    }

protected:
    std::mutex m;

    // there should be about 4 to 12 encoders, however with 256 we are sure to not be out of bounds
    unsigned long lastEncoderTick[256] = { 0 };

    static ViewManager* instance;

    ViewManager()
        : draw(styles)
    {
    }

    void loadPlugin(char* value, const char* filename)
    {
        Plugin plugin;
        plugin.name = strtok(value, " ");
        char* path = strtok(NULL, " ");
        void* handle = dlopen(getFullpath(path, filename).c_str(), RTLD_LAZY);
        if (!handle) {
            logError("Cannot open component library %s [%s]: %s\n", path, filename, dlerror());
            return;
        }

        dlerror();
        plugin.allocator = [handle](ComponentInterface::Props props) {
            void* allocator = dlsym(handle, "allocator");
            return ((ComponentInterface * (*)(ComponentInterface::Props props)) allocator)(props);
        };
        const char* dlsym_error = dlerror();
        if (dlsym_error) {
            logError("Cannot load symbol: %s\n", dlsym_error);
            dlclose(handle);
            return;
        }
        plugins.push_back(plugin);
    }

    void addComponent(string name, Point position, Size size)
    {
        ComponentInterface::Props props = {
            NULL,
            position,
            size,
            getPlugin,
            sendAudioEvent,
            getController,
            view,
            [this](uint8_t index, uint8_t value) { shift[index] = value; view->onShift(index, value); }
        };

        // printf("addComponent: %s pos %d %d size %d %d\n", name, position.x, position.y, size.w, size.h);

        for (auto& plugin : plugins) {
            if (plugin.name == name) {
                ComponentInterface* component = plugin.allocator(props);
                addComponent(component);
                return;
            }
        }
        logWarn("Unknown component: %s", name.c_str());
    }

    void addComponent(ComponentInterface* component)
    {
        if (views.size() > 0) {
            View* lastView = views.back();
            component->container = lastView->containers.back();
            lastView->components.push_back(component);
            if (component->jobRendering) {
                lastView->componentsJob.push_back(component);
            }
        } else {
            logError("ERROR: No view to add component to. Create first a view to be able to add components.\n");
        }
    }

public:
#ifdef USE_DRAW_WITH_SDL
    DrawWithSDL draw;
#else
    Draw draw;
#endif

    uint8_t shift[256] = { 0 };

    static ViewManager& get()
    {
        if (!instance) {
            instance = new ViewManager();
        }
        return *instance;
    }

    void init()
    {
        draw.init();

        view = views[0];

        for (auto& v : views) {
            v->init();
        }
    }

    bool render()
    {
        m.lock();
        if (!views.size()) {
            return false;
        }

        draw.clear();

        if (lastView != NULL) {
            for (auto& component : lastView->components) {
                for (auto* value : component->values) {
                    value->onUpdate([](float, void* data) {}, NULL);
                }
            }
        }

        view->initActiveComponents();

        m.unlock();

        renderComponents();

        return true;
    }

    void renderComponents(unsigned long now = getTicks())
    {
        m.lock();
        view->renderComponents(now);
        m.unlock();
    }

    bool config(char* key, char* value, const char* filename)
    {
        /*md
        ### PLUGIN_COMPONENT

        A component must be load from a shared library (those `.so` files). To load those plugin components, use `PLUGIN_COMPONENT: given_name_to_component ../path/of/the/component.so`.

        ```coffee
        PLUGIN_COMPONENT: Encoder ../plugins/build/libzic_EncoderComponent.so
        ```

        In this example, we load the shared library `../plugins/build/libzic_EncoderComponent.so` and we give it the name of `Encoder`. The `Encoder` name will be used later to place the components in the view.
        */
        if (strcmp(key, "PLUGIN_COMPONENT") == 0) {
            loadPlugin(value, filename);
            return true;
        }
        /*md
        ### COMPONENT

        To place previously loaded components inside a view, use `COMPONENT: given_name_to_component x y w h`.

        ```coffee
        COMPONENT: Encoder 100 0 100 50
        ENCODER_ID: 1
        VALUE: MultiModeFilter RESONANCE
        ```

        A component can get extra configuration settings and any `KEY: VALUE` following `COMPONENT: ` will be forwarded to the component.
        In this example, we assign the hardware encoder id 1 to this component and we assign it to the resonance value from the multi mode filter audio plugin.
        */
        if (strcmp(key, "COMPONENT") == 0) {
            char* name = strtok(value, " ");
            Point position = { atoi(strtok(NULL, " ")), atoi(strtok(NULL, " ")) };
            char* w = strtok(NULL, " ");
            char* h = strtok(NULL, " ");
            Size size = { w ? atoi(w) : styles.screen.w, h ? atoi(h) : styles.screen.h };
            addComponent(name, position, size);
            return true;
        }

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
*/
        if (strcmp(key, "VIEW") == 0) {
            View* v = new View(draw, [&](std::string name) { setView(name); }, shift);
            v->name = value;

            views.push_back(v);
            setView(v->name);

            return true;
        }

        /*md
        ### STARTUP_VIEW

        `STARTUP_VIEW` can be used to load a specific view on startup. This command should only be call at the end of the configuration file, once all the view has been initialised.

        ```coffee
        #STARTUP_VIEW: ViewName

        STARTUP_VIEW: Mixer
        ```

        If `STARTUP_VIEW` is not defined, the first defined view will be displayed.
        */
        if (strcmp(key, "STARTUP_VIEW") == 0) {
            setView(value);
            return true;
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

        if (draw.config(key, value)) {
            return true;
        }

        if (views.size() > 0) {
            if (views.back()->config(key, value)) {
                return true;
            }
            if (views.back()->components.size() > 0) {
                if (strcmp(key, "SHARED_COMPONENT") == 0) {
                    SharedComponent shared;
                    shared.name = value;
                    shared.component = views.back()->components.back();
                    sharedComponents.push_back(shared);
                }
                return views.back()->components.back()->baseConfig(key, value);
            }
        }

        return false;
    }

    void config(const char* key, const char* value)
    {
        config((char*)key, (char*)value);
    }
};

ViewManager* ViewManager::instance = NULL;

#endif