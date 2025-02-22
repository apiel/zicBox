#pragma once

#include "libs/nlohmann/json.hpp"
#include <dlfcn.h>
#include <mutex>
#include <vector>

#include "controllerList.h"
#include "helpers/getFullpath.h"
#include "helpers/getTicks.h"
#include "host.h"
#include "log.h"
#include "plugins/components/componentInterface.h"
#include "plugins/components/view.h"
#include "styles.h"

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
        std::string name;
        std::function<ComponentInterface*(ComponentInterface::Props props)> allocator;
    };
    std::vector<Plugin> plugins;

protected:
    struct SharedComponent {
        std::string name;
        ComponentInterface* component;
    };

    View* lastView = NULL;

    std::vector<View*> views;

public:
    View* view = NULL;

    void setView(std::string value, bool force = false)
    {
        printf("set view string to %s\n", value.c_str());
        if (value == "&previous") {
            value = lastView->name;
        }
        for (int i = 0; i < views.size(); i++) {
            if (views[i]->name == value) {
                if (view != views[i] || force) {
                    if (lastView != view) {
                        lastView = view;
                    }
                    view = views[i];
                    view->setGroup(view->resetGroupOnSetView ? 0 : view->activeGroup);
                    for (int i = 0; i < 256; i++) {
                        view->onContext(i, contextVar[i]);
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

    void setContext(uint8_t index, float value)
    {
        contextVar[index] = value;
        view->onContext(index, value);
    }

    Plugin& loadPlugin(std::string name, nlohmann::json& config)
    {
        for (auto& plugin : plugins) {
            if (plugin.name == name) {
                return plugin;
            }
        }

        Plugin plugin;
        plugin.name = name;
#ifdef IS_RPI
        std::string path = "./plugins/components/Pixel/build/arm/libzic_" + name + "Component.so";
#else
        std::string path = "./plugins/components/Pixel/build/x86/libzic_" + name + "Component.so";
#endif
        if (config.contains("pluginPath")) {
            path = config["pluginPath"].get<std::string>();
        }

        void* handle = dlopen(path.c_str(), RTLD_LAZY);
        if (!handle) {
            throw std::runtime_error("Cannot open component library " + path + ": " + dlerror());
        }

        dlerror();
        plugin.allocator = [handle](ComponentInterface::Props props) {
            void* allocator = dlsym(handle, "allocator");
            return ((ComponentInterface * (*)(ComponentInterface::Props props)) allocator)(props);
        };
        const char* dlsym_error = dlerror();
        if (dlsym_error) {
            dlclose(handle);
            throw std::runtime_error("Cannot load symbol: " + std::string(dlsym_error));
        }
        plugins.push_back(plugin);
        return plugins.back();
    }

    void addComponent(nlohmann::json& config, View* targetView)
    {
        try {
            if (!config.contains("componentName")) {
                logWarn("Missing componentName in component config.");
                return;
            }
            if (!config.contains("bounds") || !config["bounds"].is_array() || config["bounds"].size() < 2) {
                logWarn("Missing bounds in component config.");
                return;
            }
            std::string name = config["componentName"].get<std::string>();
            // printf("Adding component %s %s\n", name.c_str(), config.dump().c_str());
            Point position = { config["bounds"][0].get<int>(), config["bounds"][1].get<int>() };

            // Check if width and height exist, otherwise use default values
            int w = config.contains("bounds") && config["bounds"].size() > 2 ? config["bounds"][2].get<int>() : styles.screen.w;
            int h = config.contains("bounds") && config["bounds"].size() > 3 ? config["bounds"][3].get<int>() : styles.screen.h;
            Size size = { w, h };

            ComponentInterface::Props props = {
                targetView->name + "_" + name + "_x" + std::to_string(position.x) + "_y" + std::to_string(position.y),
                config,
                position,
                size,
                getPlugin,
                sendAudioEvent,
                getController,
                targetView,
                [this](uint8_t index, float value) { setContext(index, value); }
            };
            Plugin& plugin = loadPlugin(name, config);
            ComponentInterface* component = plugin.allocator(props);
            targetView->components.push_back(component);
            if (component->jobRendering) {
                targetView->componentsJob.push_back(component);
            }
        } catch (const std::exception& e) {
            logError("Error adding component: %s", e.what());
        }
    }

public:
#ifdef USE_DRAW_WITH_SDL
    DrawWithSDL draw;
#else
    Draw draw;
#endif

    float contextVar[256] = { 0 };

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
        setView(view->name, true);

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
                    value->setOnUpdateCallback([](float, void* data) { }, NULL);
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
        if (strcmp(key, "COMPONENT") == 0) {
            try {
                nlohmann::json config = nlohmann::json::parse(value);
                addComponent(config, views.back());
                return true;
            } catch (const std::exception& e) {
                logError("COMPONENT: JSON Parsing Error: %s", e.what());
            }
        }

        /*#md
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
            View* v = new View(draw, [&](std::string name) { setView(name); }, contextVar);
            v->name = value;

            views.push_back(v);
            setView(v->name);

            return true;
        }

        if (draw.config(key, value)) {
            return true;
        }

        if (views.size() > 0) {
            if (views.back()->config(key, value)) {
                return true;
            }
        }

        return false;
    }

    void config(const char* key, const char* value)
    {
        config((char*)key, (char*)value);
    }

    void config(nlohmann::json& config)
    {
        if (config.is_array()) {
            for (auto& v : config) {
                // TODO Might want to move all this in view!!!
                if (v.contains("name") && v.contains("components") && v["components"].is_array()) {
                    View* newView = new View(draw, [&](std::string name) { setView(name); }, contextVar);
                    newView->name = v["name"];
                    try {
                        // TODO how to handle extra config?
                        views.push_back(newView);
                        for (auto& component : v["components"]) {
                            addComponent(component, newView);
                        }
                    } catch (const std::exception& e) {
                        logError("view %s config: %s", newView->name.c_str(), e.what());
                    }
                }
            }
        }
    }
};

ViewManager* ViewManager::instance = NULL;
