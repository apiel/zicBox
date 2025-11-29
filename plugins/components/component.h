/** Description:
This C++ header file defines the blueprint for the fundamental building block of the application, a class simply called `Component`. This component acts as the standardized base layer for all interactive elements, screen widgets, or functional modules within the system.

**Role and Setup:**
The `Component` handles its initial configuration, reading necessary settings (often from a structured data format like JSON) upon creation. It manages essential properties like tracking external values and dynamically resizing when the screen layout changes. A critical feature is the `VisibilityContext`, which determines whether the component is currently active and visible to the user.

**Interaction and Input:**
A core function of the `Component` is managing diverse user interactions. It integrates specialized managers for:
1.  **Key Input:** Using a `KeypadLayout` to interpret and respond to specific key presses or keyboard commands.
2.  **Motion/Touch:** Handling touch screen input, mouse movements, or pointer interactions, ensuring the component only responds if the input occurs within its boundaries.
3.  **Encoders:** Processing input from rotary knobs often found on specialized control hardware.

**Visual Management and Updates:**
This base class dictates when and how the element is drawn. It manages its display state, including utilizing a `ControllerColor` module to ensure it displays the correct colors. When its status or an external value changes, the component schedules itself to be redrawn (rendered) in the next update cycle, maintaining a responsive visual interface.

In summary, the `Component` class provides the reusable infrastructure—handling configuration, visibility, various input types, and rendering logic—that allows developers to quickly build complex, interactive elements for the application.

sha: fd735d80b6fecd9ed81eab6c4c8b83acb1a369014a155f41fcc70d4acc7f6f90
*/
#pragma once

#include "base/KeypadLayout.h"
#include "componentInterface.h"
#include "drawInterface.h"
#include "motionInterface.h"
#include "utils/VisibilityContext.h"
// #include "utils/VisibilityGroup.h" // TODO
#include "plugins/components/base/ControllerColor.h"
#include "plugins/components/utils/resize.h"
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
        if (!motion.originIn({ relativePosition, size })) {
            return;
        }

        if (motion.in({ relativePosition, size })) {
            onMotion(motion);
        }
    }

    virtual void onMotionRelease(MotionInterface& motion) override
    {
    }

    virtual void handleMotionRelease(MotionInterface& motion) override
    {
        if (!motion.originIn({ relativePosition, size })) {
            return;
        }
        onMotionRelease(motion);
    }

    virtual void onEncoder(int8_t id, int8_t direction) override
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

    uint8_t resizeType = RESIZE_NONE;
    virtual void resize(float xFactor, float yFactor, Point containerPosistion) override
    {
        clear();
        resizeOriginToRelative(resizeType, xFactor, yFactor, position, sizeOriginal, relativePosition, size);
        relativePosition.x += containerPosistion.x;
        relativePosition.y += containerPosistion.y;
        renderNext();
        resize();
    }

    virtual const std::vector<EventInterface::EncoderPosition> getEncoderPositions() override
    {
        return {};
    }
};
