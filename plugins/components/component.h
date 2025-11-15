#pragma once

#include "base/KeypadLayout.h"
#include "componentInterface.h"
#include "drawInterface.h"
#include "motionInterface.h"
#include "utils/VisibilityContext.h"
// #include "utils/VisibilityGroup.h" // TODO
#include "plugins/components/base/ControllerColor.h"
#include "valueInterface.h"

#include <stdlib.h>

class Component : public ComponentInterface {
protected:
    VisibilityContext visibilityContext;
    // VisibilityGroup visibilityGroup; // TODO

public:
    KeypadLayout keypadLayout;
    ControllerColor controllerColor;

    Component(
        ComponentInterface::Props props,
        std::function<std::function<void(KeypadLayout::KeyMap& keymap)>(std::string action)> keypadCustomAction = nullptr, bool skipInitKeypad = false)
        : ComponentInterface(props)
        , keypadLayout(this, keypadCustomAction)
        , controllerColor(this)
    {
        nlohmann::json& config = props.config;
        track = config.value("track", track);
        group = config.value("group", group);
        resizeType = config.value("resizeType", resizeType);
        visibilityContext.init(config);
        if (!skipInitKeypad) {
            keypadLayout.init(config);
        }
        controllerColor.init(config);
    }

    ValueInterface* watch(ValueInterface* value)
    {
        if (value != NULL) {
            values.push_back(value);
        }
        return value;
    }

    virtual void render() override { }
    virtual void initView(uint16_t counter) override
    {
        if (isVisible()) {
            controllerColor.render();
        }
    }

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

    virtual bool onKey(uint16_t id, int key, int8_t state, unsigned long now) override
    {
        if (isVisible()) {
            // printf("[%s]\n", this->nameUID.c_str());
            return keypadLayout.onKey(id, key, state, now);
        }
        return false;
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
            if (isVisible()) {
                controllerColor.render();
            }
        }
    }

    // Generic getConfig function that can return any type
    template <typename T = std::string>
    T getConfig(const nlohmann::json& config, const std::string& parameterKey, const char* errorDescription = nullptr) const
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
        std::string pluginName = getConfig<std::string>(config, parameterKey, errorDescription);
        return &getPlugin(pluginName, track);
    }

    virtual void clear() override
    {
        draw.filledRect(relativePosition, size, { styles.colors.background });
    }

    enum ResizeType {
        RESIZE_NONE = 1 << 0, // 1
        RESIZE_X = 1 << 1, // 2
        RESIZE_Y = 1 << 2, // 4
        RESIZE_W = 1 << 3, // 8
        RESIZE_H = 1 << 4, // 16
    } resizeType
        = RESIZE_NONE;

    virtual void resize(float xFactor, float yFactor) override
    {
        clear();
        if (resizeType & RESIZE_W) size.w = sizeOriginal.w * xFactor;
        if (resizeType & RESIZE_H) size.h = sizeOriginal.h * yFactor;
        if (resizeType & RESIZE_X) relativePosition.x = position.x * xFactor;
        if (resizeType & RESIZE_Y) relativePosition.y = position.y * yFactor;
        renderNext();
        resize();
    }
};
