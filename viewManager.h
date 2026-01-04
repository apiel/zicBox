/** Description:
This code defines the `ViewManager`, which acts as the central control system for managing a user interface, particularly suitable for embedded or modular display applications. It coordinates everything the user sees on the screen.

### Core Functionality

The `ViewManager` manages multiple "Views," which are essentially different screen layouts or pages. Each View is composed of individual interactive elements called "Components" (like buttons, gauges, or text areas).

1.  **Centralized Control:** It operates as a globally accessible entity (a Singleton pattern), ensuring that all parts of the application refer to the same instance for managing the display.
2.  **Dynamic Modularity (Plugins):** A key feature is the ability to load Components dynamically. It uses a plugin architecture to load component code from external files. This means new display elements can be added or updated without needing to recompile the main system.
3.  **Rendering Abstraction:** The Manager supports multiple drawing backends (like Framebuffer, specialized display drivers like ST7789, or desktop libraries like SDL/SFML). It selects the correct rendering method during initialization to draw the active View and its Components.
4.  **Navigation and State:** It controls which View is currently active via the `setView` function, handling navigation and even temporary tagging of Views for easy recall. It also maintains a set of "context variables" to pass real-time data (like sensor readings or settings) to the active Components.
5.  **Configuration:** It reads detailed configurations (usually from a JSON structure) to set up screen parameters, select the appropriate renderer, and define the layout and properties of all Views and their Components upon startup.

In essence, the `ViewManager` is responsible for loading the layout, handling screen transitions, feeding data to the visual elements, and executing the actual drawing process on the device screen.

Tags: C++, Header, Class, Singleton, ViewManager, ViewManagement, ComponentManagement, PluginArchitecture, DynamicLoading, SharedLibraries, GUI, Rendering, Graphics, Configuration, JSON, nlohmann/json, EmbeddedSystems, Framebuffer, CrossPlatform, Drawing, Concurrency, Synchronization, StateManagement, FactoryPattern, UI/UX, Logging.
sha: 937ad9bc528b510f4d7a6a6c092adea8eb8d998ba79bf1f3f8142d0f35d3277f
*/
#pragma once

#include "libs/nlohmann/json.hpp"
#include <dlfcn.h>
#include <mutex>
#include <vector>

#include <cstdio> // for std::remove
#include <fstream> // for std::ifstream
#include <string>
#include <unistd.h> // for access()

#include "controllerList.h"
#include "helpers/getExecutableDirectory.h"
#include "helpers/getTicks.h"
#include "host.h"
#include "log.h"
#include "plugins/components/componentInterface.h"
#include "plugins/components/viewMonoContainer.h"
#include "plugins/components/viewMultiContainer.h"
#include "styles.h"

#ifdef DRAW_SDL
#include "draw/drawWithSDL.h"
#endif

#ifdef DRAW_SMFL
#include "draw/drawWithSFML.h"
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

    View* previousView = NULL;

    std::vector<View*> views;

    std::unordered_map<std::string, std::string> taggedViews;

public:
    View* view = NULL;

    void setView(std::string value, bool force = false)
    {
        logTrace("set view string to %s", value.c_str());
        // logDebug("set view string to %s", value.c_str());
        if (value == "&previous" && previousView != NULL) {
            value = previousView->name;
        }
        // use # to save a view to a variable a re-use later
        // e.g.: setView("viewName#viewNameVar");
        //       setView("#viewNameVar"); <--- will then use the view stored value under "viewNameVar"
        size_t hashPos = value.find('#');
        if (hashPos != std::string::npos) {
            std::string tagName = value.substr(hashPos + 1);
            value = value.substr(0, hashPos);
            if (!tagName.empty()) {
                auto it = taggedViews.find(tagName);
                if (it != taggedViews.end()) {
                    if (hashPos == 0) {
                        value = it->second;
                    } else {
                        it->second = value;
                    }
                } else if (hashPos != 0) {
                    taggedViews[tagName] = value;
                } else {
                    logWarn("[%d] No view stored under tag: %s", hashPos, tagName.c_str());
                    return;
                }
            }
        }
        for (int i = 0; i < views.size(); i++) {
            if (views[i]->name == value) {
                if (view != views[i] || force) {
                    if (view->saveForPrevious && previousView != view) {
                        previousView = view;
                    }
                    view = views[i];
                    for (int i = 0; i < 256; i++) {
                        view->onContext(i, contextVar[i]);
                    }
                    render();
                }
                return;
            }
        }
        logWarn("Unknown view: %s", value.c_str());
    }

protected:
    std::mutex m;

    // there should be about 4 to 12 encoders, however with 256 we are sure to not be out of bounds
    unsigned long lastEncoderTick[256] = { 0 };

    static ViewManager* instance;

    ViewManager()
        : drawFB(styles)
        , drawST7789(styles)
#ifdef DRAW_SMFL
        , drawSMFL(styles)
        , draw(&drawSMFL)
#elif defined(DRAW_SDL)
        , drawSDL(styles)
        , draw(&drawSDL)
#else
        // By default use FB
        , draw(&drawFB)
#endif
    {
    }

    void setContext(uint8_t index, float value)
    {
        contextVar[index] = value;
        if (view != NULL) {
            view->onContext(index, value);
        }
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

    void addComponent(nlohmann::json& config, View* targetView, Container* container)
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
            targetView->addComponent(component, container);
        } catch (const std::exception& e) {
            logError("Error adding component: %s in %s", e.what(), config.dump().c_str());
        }
    }

public:
#ifdef DRAW_SMFL
    DrawWithSFML drawSMFL;
#elif defined(DRAW_SDL)
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
        if (!views.size()) {
            return false;
        }
        m.lock();

        // #ifdef DRAW_DESKTOP
        draw->clear(); // <---- was slow, is it still slow with the new fix?
        // #endif

        if (previousView != NULL) {
            for (auto& component : previousView->getComponents()) {
                for (auto* value : component->values) {
                    value->setOnUpdateCallback([](float, void* data) { }, NULL);
                }
            }
        }
        view->activate();
        m.unlock();

        // renderComponents(); // <------ is this one necessary?
        drawMessage();
        return true;
    }

    void drawMessage()
    {
        if (access("message.txt", F_OK) != 0) {
            return;
        }

        std::ifstream infile("message.txt");
        std::string text;
        if (infile) {
            std::getline(infile, text); // reads a single line, no '\n'
        }
        infile.close();
        if (std::remove("message.txt") != 0) {
            perror("Failed to delete message.txt");
        }

        if (text.empty()) {
            return;
        }

        if (text.rfind("setView:") == 0) {
            std::string viewName = text.substr(8);
            setView(viewName);
            return;
        }

        Color color = styles.colors.white;
        color.a = 220;
        void* font = draw->getFont("PoppinsLight_8");
        draw->filledRect({ 0, 0 }, { (int)(styles.screen.w * 0.5), 10 }, { .color = color });
        draw->text({ 4, 1 }, text, 8, { .color = { 0, 0, 0 }, .font = font });
        draw->render();
    }

    void renderComponents(unsigned long now = getTicks())
    {
        m.lock();
        view->renderComponents(now);
        m.unlock();
    }

    void config(nlohmann::json& config)
    {
#ifndef DRAW_DESKTOP
        if (config.contains("renderer")) {
            std::string renderer = config["renderer"].get<std::string>();
            if (renderer == "FB") {
                draw = &drawFB;
            } else if (renderer == "ST7789") {
                draw = &drawST7789;
            }
        }
#endif

        if (config.contains("taggedViews") && config["taggedViews"].is_object()) {
            taggedViews = config["taggedViews"];
        }

        // Should happen before views
        if (config.contains("screen")) {
            logInfo("----------- init screen / draw -------------");
            draw->config(config["screen"]);
            logDebug("init screen / draw done.");
        }

        if (config.contains("views")) {
            logInfo("----------- init views -------------");
            nlohmann::json& viewsConfig = config["views"];
            if (viewsConfig.is_array()) {
                logDebug("Loading %d views", viewsConfig.size());
                for (auto& v : viewsConfig) {
                    if (v.contains("name") && (v.contains("components") || v.contains("containers"))) {
                        logTrace("Loading view %s", v["name"].get<std::string>().c_str());

                        View* newView = nullptr;
                        if (v.contains("components") && v["components"].is_array())
                            newView = new ViewMonoContainer(*draw, [&](std::string name) { setView(name); }, contextVar);
                        else
                            newView = new ViewMultiContainer(*draw, [&](std::string name) { setView(name); }, contextVar);

                        newView->name = v["name"];
                        if (v.contains("noPrevious")) {
                            logTrace("view %s noPrevious", newView->name.c_str());
                            newView->saveForPrevious = !v["noPrevious"];
                        }
                        views.push_back(newView);
                        if (v.contains("components") && v["components"].is_array())
                            componentConfig(v, newView, NULL);
                        else if (v.contains("containers") && v["containers"].is_array()) {
                            for (auto& c : v["containers"]) {
                                if (c.contains("components") && c["components"].is_array()
                                    && c.contains("name") && c.contains("position") && c["position"].is_array() && c["position"].size() == 2) {
                                    std::string name = c["name"].get<std::string>();
                                    Point position = { c["position"][0].get<int>(), c["position"][1].get<int>() };
                                    std::string height = c.value("height", "100%");
                                    Container* container = newView->addContainer(name, position, height);
                                    container->config(c);
                                    componentConfig(c, newView, container);
                                }
                            }
                        }
                    }
                }
            }
            logDebug("init views done.");
        }
    }

    void componentConfig(nlohmann::json& config, View* newView, Container* container)
    {
        if (config.contains("components") && config["components"].is_array()) {
            try {
                for (auto& component : config["components"]) {
                    addComponent(component, newView, container);
                }
            } catch (const std::exception& e) {
                logError("view %s config: %s", newView->name.c_str(), e.what());
            }
        }
    }
};

ViewManager* ViewManager::instance = NULL;
