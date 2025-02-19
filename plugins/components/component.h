#pragma once

#include "base/KeypadLayout.h"
#include "componentInterface.h"
#include "drawInterface.h"
#include "motionInterface.h"
#include "utils/VisibilityContext.h"
// #include "utils/VisibilityGroup.h" // TODO
#include "valueInterface.h"

#include <optional>
#include <stdlib.h>

class Component : public ComponentInterface {
protected:
    VisibilityContext visibilityContext;
    // VisibilityGroup visibilityGroup; // TODO

public:
    KeypadLayout keypadLayout;

    Component(
        ComponentInterface::Props props,
        std::function<std::function<void(KeypadLayout::KeyMap& keymap)>(std::string action)> keypadCustomAction = nullptr)
        : ComponentInterface(props)
        , keypadLayout(this, keypadCustomAction)
    {
        nlohmann::json config = props.config;
        track = config.value("track", track);
        group = config.value("group", group);
        visibilityContext.init(config);
        keypadLayout.init(config);
    }

    ValueInterface* watch(ValueInterface* value)
    {
        if (value != NULL) {
            values.push_back(value);
        }
        return value;
    }

    virtual void clear() override
    {
        draw.filledRect(position, size, { draw.styles.colors.background });
    };

    virtual void render() override { }
    virtual void initView(uint16_t counter) override { }

    virtual void renderNext() override
    {
        if (isVisible()) {
            view->pushToRenderingQueue(this);
        }
    }

    virtual bool isVisible() override { return visibilityContext.visible; }

    virtual void onUpdate(ValueInterface* value) override
    {
        renderNext();
    }

    virtual void onMotion(MotionInterface& motion) override
    {
    }

    virtual void handleMotion(MotionInterface& motion) override
    {
        if (!motion.originIn({ position, size })) {
            return;
        }

        if (motion.in({ position, size })) {
            if (motion.isStarting() && group != -1) {
                view->setGroup(group);
            }
            onMotion(motion);
        }
    }

    virtual void onMotionRelease(MotionInterface& motion) override
    {
    }

    virtual void handleMotionRelease(MotionInterface& motion) override
    {
        onMotionRelease(motion);
    }

    virtual void onEncoder(int id, int8_t direction) override
    {
    }

    virtual void onKey(uint16_t id, int key, int8_t state, unsigned long now) override
    {
        // printf("[%s]\n", this->id.c_str());
        keypadLayout.onKey(id, key, state, now);
    }

    virtual void resize() override
    {
    }

    virtual void onGroupChanged(int8_t index) override
    {
    }

    virtual void onContext(uint8_t index, float value) override
    {
        if (visibilityContext.onContext(index, value)) {
            renderNext();
        }
    }

    // // Get a parameter from the JSON config, throwing an exception if it's missing
    // std::string getParam(const nlohmann::json& config, const std::string& parameterKey, const char* errorDescription = nullptr) const
    // {
    //     if (!config.contains(parameterKey)) {
    //         if (errorDescription) {
    //             throw std::runtime_error(errorDescription);
    //         }
    //         throw std::runtime_error(
    //             nameUID + ": Component is missing " + parameterKey + " parameter.");
    //     }
    //     return config[parameterKey].get<std::string>();
    // }

    // // Get a plugin pointer, propagating any exceptions from getParam
    // AudioPlugin* getPluginPtr(const nlohmann::json& config, const std::string& parameterKey, int16_t track, const char* errorDescription = nullptr) const
    // {
    //     std::string pluginName = getParam(config, parameterKey, errorDescription);
    //     return &getPlugin(pluginName.c_str(), track);
    // }

    // Generic getParam function that can return any type
    template <typename T>
    T getParam(const nlohmann::json& config, const std::string& parameterKey, const char* errorDescription = nullptr) const
    {
        if (!config.contains(parameterKey)) {
            if (errorDescription) {
                throw std::runtime_error(errorDescription);
            }
            throw std::runtime_error(
                nameUID + ": Component is missing " + parameterKey + " parameter.");
        }
        try {
            return config[parameterKey].get<T>();
        } catch (const nlohmann::json::exception& e) {
            throw std::runtime_error(
                nameUID + ": Invalid type for parameter " + parameterKey + ". Expected type: " + typeid(T).name() + ". Error: " + e.what());
        }
    }

    // Example usage with getPluginPtr
    AudioPlugin* getPluginPtr(const nlohmann::json& config, const std::string& parameterKey, int16_t track, const char* errorDescription = nullptr) const
    {
        std::string pluginName = getParam<std::string>(config, parameterKey, errorDescription);
        return &getPlugin(pluginName.c_str(), track);
    }
};
