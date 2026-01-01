/** Description:
This C++ header file defines the foundational blueprint for a generic interactive screen element, referred to here as a "Component." Every interactive item in the system (such as a button, a display, or a control knob) will inherit its core behavior from this structure.

**Core Functionality:**

1.  **Interface Integration:** The Component ensures it adheres to several required system standards, allowing it to interface seamlessly with the drawing system, motion/touch handlers, and data tracking services.

2.  **Input and Interaction Management:** It is primarily responsible for receiving and processing user input. This includes handling physical key presses or keypad entries, detecting motion (like touch input), and responding to events from rotary encoders (knobs). Critically, it checks its visibility before processing any input.

3.  **Appearance and Visibility:** The Component manages its own visibility, determining whether it should be drawn on the screen based on internal rules or external context changes. It also tracks the specific color and styling properties assigned to it.

4.  **Data Monitoring (Watching):** Components can be set up to "watch" external data sources. If the underlying data changes, the Component automatically triggers a visual update to reflect the new state.

5.  **Rendering and Layout:** It contains methods to draw itself and to request the application’s rendering engine to update its display. It also includes complex logic to adjust its size and relative position accurately whenever the overall screen layout is resized or rearranged.

6.  **Configuration Safety:** Utility functions are provided to safely read and extract specific settings (like text strings or numbers) from the configuration files used during its initialization, ensuring the component is set up correctly and preventing errors from missing or improperly formatted data.

sha: d72b42285b6c8d074ee1979b784b8f3d3bc42ce82ba357b29268cb8e7fc7a32f
*/
#pragma once

#include "base/KeypadLayout.h"
#include "componentInterface.h"
#include "drawInterface.h"
#include "motionInterface.h"
#include "plugins/components/utils/VisibilityContext.h"
// #include "utils/VisibilityGroup.h" // TODO
#include "plugins/components/base/ControllerColor.h"
#include "plugins/components/utils/resize.h"
#include "valueInterface.h"

#include <stdlib.h>

class Component : public ComponentInterface {
protected:
    VisibilityContext visibilityContext;
    // VisibilityGroup visibilityGroup; // TODO

    int8_t extendEncoderIdArea = -1;

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
        extendEncoderIdArea = config.value("extendEncoderIdArea", extendEncoderIdArea);
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
        if (extendEncoderIdArea < 0) {
            return {};
        }

        return {
            { extendEncoderIdArea, size, relativePosition },
        };
    }
};
