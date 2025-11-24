/** Description:
This code defines the blueprint for a specialized user interface element called a **Workspace Knob Component**. Its primary role is to provide a visual indicator and a control mechanism for switching between different saved states or configurations, known as "workspaces," within an audio application or control system.

### How It Works

1.  **Display and Settings:** The component manages its own appearance, including specific colors for the background, the main text (the workspace number), a descriptive label ("Workspace"), and a badge indicating if the currently displayed workspace is active. It supports customized fonts and font sizes, all dynamically loaded from external settings upon initialization.
2.  **Navigation (The Knob):** The component is designed to be linked to a physical rotary encoder (a digital knob). When a user turns this knob, the component responds by incrementing or decrementing the displayed workspace number, cycling through W01 up to W99.
3.  **Interaction (Loading):** When a user activates a corresponding action (usually pressing a button associated with the knob), the component initiates communication with an "Audio Plugin." This instructs the plugin to execute the task of loading the selected workspace configuration, effectively changing the current setup of the controlled software or hardware.
4.  **Active Status:** The component constantly checks with the plugin to determine which workspace is currently active. If the number being displayed matches the active workspace, it renders a visual "active" badge.

sha: bee96926387b8e80f4803f6c1008903f27ad1cfedda085e194881ae455099252 
*/
#pragma once

#include "helpers/clamp.h"
#include "log.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

/*md
## WorkspaceKnob

A knob to switch between workspaces
*/

class WorkspaceKnobComponent : public Component {
    Color bgColor;
    Color color;
    Color labelColor;
    Color badgeColor;

    int fontSize = 32;
    void* font = NULL;
    int labelfontSize = 12;
    void* labelfont = NULL;
    void* activefont = NULL;

    int workspace = 1;

    int8_t encoderId = -1;
    AudioPlugin* plugin = NULL;
    std::string* currentWorkspaceName = NULL;

    std::string getWorkspaceName()
    {
        std::string prefix = workspace < 10 ? "0" : "";
        return "W" + prefix + std::to_string(workspace);
    }

public:
    WorkspaceKnobComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".load") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (currentWorkspaceName != NULL && getWorkspaceName() == *currentWorkspaceName) {
                            // reload
                            logDebug("TODO Reloading workspace");
                            renderNext();
                        } else if (plugin) {
                        // if (plugin) {
                            uint8_t dataId = plugin->getDataId("LOAD_WORKSPACE");
                            std::string workspaceName = getWorkspaceName();
                            plugin->data(dataId, &workspaceName);
                            renderNext();
                        }
                    }
                };
            }
            return func;
        })
        , bgColor(styles.colors.background)
        , color(styles.colors.text)
        , labelColor(alpha(styles.colors.text, 0.4))
        , badgeColor(rgb(39, 128, 39))
    {
        font = draw.getFont("PoppinsLight_16");
        activefont = draw.getFont("PoppinsLight_8");
        /*md md_config:Text */
        nlohmann::json& config = props.config;

        /// The font size of the text.
        fontSize = config.value("fontSize", fontSize); //eg: 8
        /// The font of the text. Default is null.
        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str()); //eg: "Sinclair_S"
        }

        /// The font size of the text.
        labelfontSize = config.value("labelFontSize", labelfontSize); //eg: 8
        /// The font of the text. Default is null.
        if (config.contains("labelFont")) {
            labelfont = draw.getFont(config["labelFont"].get<std::string>().c_str()); //eg: "Sinclair_S"
        }

        /// The background color of the text.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// The color of the text
        color = draw.getColor(config["color"], color); //eg: "#ffffff"

        /// The color of the text
        labelColor = draw.getColor(config["labelColor"], labelColor); //eg: "#ffffff"

        /// Color the active workspace badge.
        badgeColor = draw.getColor(config["badgeColor"], badgeColor); //eg: "#23a123"

        /// The encoder id that will interract with this component.
        encoderId = config.value("encoderId", encoderId); //eg: 0

        /// The audio plugin to get control on.
        if (config.contains("audioPlugin")) {
            plugin = &getPlugin(config["audioPlugin"].get<std::string>(), track); //eg: "audio_plugin_name"
            if (plugin) {
                currentWorkspaceName = (std::string*)plugin->data(plugin->getDataId(config.value("currentWorkspaceDataId", "CURRENT_WORKSPACE"))); //eg: "CURRENT_WORKSPACE"
            }
        }

        /*md md_config_end */
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });
        int x = relativePosition.x + (size.w) * 0.5;
        int y = relativePosition.y + size.h - labelfontSize - 4;
        draw.textCentered({ x, y }, "Workspace", labelfontSize, { labelColor, .font = labelfont, .maxWidth = size.w });
        y = y - fontSize - 10;
        draw.textCentered({ x, y }, getWorkspaceName(), fontSize, { color, .font = font, .maxWidth = size.w });
        if (currentWorkspaceName != NULL && *currentWorkspaceName == getWorkspaceName()) {
            y = y - 10;
            x = relativePosition.x + size.w - 40;
            // draw.filledRect({ x, y }, { 35, 10 }, { badgeColor });
            draw.filledRect({ x, y }, { 35, 10 }, 5, { badgeColor });
            draw.text({ x + 3, y }, "active", 8, { .font = activefont });
        }
    }

    void onEncoder(int8_t id, int8_t direction) override
    {
        if (id == encoderId) {
            workspace = CLAMP(workspace + direction, 1, 99);
            renderNext();
        }
    }
};
