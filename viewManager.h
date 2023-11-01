#ifndef _VIEW_MANAGER_H
#define _VIEW_MANAGER_H

#include <dlfcn.h>
#include <mutex>
#include <vector>

#include "UiPlugin.h"
#include "draw.h"
#include "helpers/getFullpath.h"
#include "host.h"
#include "plugins/components/componentInterface.h"
#include "state.h"
#include "styles.h"

class ViewManager {
protected:
    std::mutex m;
    std::mutex m2;

    UiPlugin& ui = UiPlugin::get();
    int8_t lastGroup = -100;

    static ViewManager* instance;

    ViewManager()
        : draw(styles)
    {
    }

    struct Plugin {
        char name[64];
        ComponentInterface* (*allocator)(ComponentInterface::Props& props);
    };
    std::vector<Plugin> plugins;

    void loadPlugin(char* value, const char* filename)
    {
        Plugin plugin;
        strcpy(plugin.name, strtok(value, " "));
        char* path = strtok(NULL, " ");

        char fullpath[512];
        getFullpath(path, filename, fullpath);
        void* handle = dlopen(fullpath, RTLD_LAZY);

        if (!handle) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot open library: %s\n", dlerror());
            return;
        }

        dlerror();
        plugin.allocator = (ComponentInterface * (*)(ComponentInterface::Props & props)) dlsym(handle, "allocator");
        const char* dlsym_error = dlerror();
        if (dlsym_error) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot load symbol: %s\n", dlsym_error);
            dlclose(handle);
            return;
        }
        plugins.push_back(plugin);
    }

    void addComponent(const char* name, Point position, Size size)
    {
        ComponentInterface::Props props = { position, size, draw, getPlugin, setGroup, [](char* name) { UiPlugin::get().setView(name); } };

        for (auto& plugin : plugins) {
            if (strcmp(plugin.name, name) == 0) {
                ComponentInterface* component = plugin.allocator(props);
                ui.addComponent(component);
                return;
            }
        }
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unknown component: %s", name);
    }

    void changeGroup()
    {
        lastGroup = group;
        for (auto& component : ui.getView()) {
            component->onGroupChanged(group);
        }
    }

public:
    Draw draw;

    static ViewManager& get()
    {
        if (!instance) {
            instance = new ViewManager();
        }
        return *instance;
    }

    void init()
    {
        ui.setUpdateCallback([]() { ViewManager::get().render(); });
    }

    // TODO could this be optimized by creating mapping values to components?
    void onUpdate(ValueInterface* val)
    {
        for (auto& component : ui.getView()) {
            for (auto* value : component->values) {
                if (value == val) {
                    component->onUpdate(value);
                }
            }
        }
    }

    bool render(bool forceReRender = false)
    {
        m.lock();
        if (!ui.getViewCount()) {
            return false;
        }

        draw.clear();
        draw.next();

        changeGroup();
        ui.clearOnUpdate();
        ui.initActiveComponents([](float, void* data) { ViewManager::get().onUpdate((ValueInterface*)data); });

        m.unlock();

        renderComponents();

        return true;
    }

    void renderComponents()
    {
        m.lock();
        if (group != lastGroup) {
            changeGroup();
        }
        for (auto& component : ui.getView()) {
            component->triggerRenderer();
        }
        draw.triggerRender();
        m.unlock();
    }

    void onMotion(MotionInterface& motion)
    {
        for (auto& component : ui.getView()) {
            component->handleMotion(motion);
        }
    }

    void onMotionRelease(MotionInterface& motion)
    {
        for (auto& component : ui.getView()) {
            component->handleMotionRelease(motion);
        }
    }

    void onEncoder(int id, int8_t direction)
    {
        m2.lock();
        for (auto& component : ui.getView()) {
            component->onEncoder(id, direction);
        }
        m2.unlock();
    }

    bool config(char* key, char* value, const char* filename)
    {
        printf("------ Config: '%s' = '%s'\n", key, value);
        if (strcmp(key, "PLUGIN_COMPONENT") == 0) {
            loadPlugin(value, filename);
            return true;
        }
        if (strcmp(key, "COMPONENT") == 0) {
            char* name = strtok(value, " ");
            Point position = { atoi(strtok(NULL, " ")), atoi(strtok(NULL, " ")) };
            Size size = { atoi(strtok(NULL, " ")), atoi(strtok(NULL, " ")) };
            addComponent(name, position, size);
            return true;
        }
        return ui.config(key, value);
    }

    void config(const char* key, const char* value)
    {
        config((char*)key, (char*)value);
    }
};

ViewManager* ViewManager::instance = NULL;

#endif