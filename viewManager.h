#pragma once

#include "libs/nlohmann/json.hpp"
#include <dlfcn.h>
#include <mutex>
#include <vector>

#include "controllerList.h"
#include "helpers/getExecutableDirectory.h"
#include "helpers/getTicks.h"
#include "host.h"
#include "log.h"
#include "plugins/components/componentInterface.h"
#include "plugins/components/view.h"
#include "styles.h"

#ifdef DRAW_SDL
#include "draw/drawWithSDL.h"
#endif

#include "draw/drawWithFB.h"
#include "draw/drawWithST7789.h"

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
        logTrace("set view string to %s\n", value.c_str());
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
        : drawFB(styles)
        , drawST7789(styles)
#ifdef DRAW_SDL
        , drawSDL(styles)
        , draw(&drawSDL)
#else
        // By default use ST7789
        , draw(&drawST7789)
#endif
    {
    }

    void setContext(uint8_t index, float value)
    {
        // printf("set context %d to %f\n", index, value);
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
        std::string path = getExecutableDirectory() + "/libs/components/libzic_" + name + "Component.so";
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
            std::string name = config["componentName"].get<std::string>();
            logTrace("Adding component %s", name.c_str());
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
            logError("Error adding component: %s in %s", e.what(), config.dump().c_str());
        }
    }

public:
#ifdef DRAW_SDL
    DrawWithSDL drawSDL;
#endif

    DrawWithFB drawFB;
    DrawWithST7789 drawST7789;

    Draw* draw = NULL;

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
        if (draw == NULL) {
            throw std::runtime_error("No renderer was initialized");
        }
        draw->init();

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

        draw->clear();

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

    void config(nlohmann::json& config)
    {
#ifndef DRAW_SDL
        if (config.contains("renderer")) {
            std::string renderer = config["renderer"].get<std::string>();
            if (renderer == "FB") {
                draw = &drawFB;
            } else if (renderer == "ST7789") {
                draw = &drawST7789;
            }
        }
#endif

        // Should happen before views
        if (config.contains("screen")) {
            logInfo("----------- init screen / draw -------------");
            draw->config(config["screen"]);
        }

        if (config.contains("views")) {
            logInfo("----------- init views -------------");
            nlohmann::json& viewsConfig = config["views"];
            if (viewsConfig.is_array()) {
                for (auto& v : viewsConfig) {
                    // TODO Might want to move all this in view!!!
                    if (v.contains("name") && v.contains("components") && v["components"].is_array()) {
                        logTrace("Loading view %s", v["name"].get<std::string>().c_str());
                        View* newView = new View(*draw, [&](std::string name) { setView(name); }, contextVar);
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
    }
};

ViewManager* ViewManager::instance = NULL;
