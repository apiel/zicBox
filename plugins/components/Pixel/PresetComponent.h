/** Description:
This code defines a specialized control panel element, the **Preset Component**, designed to manage saved configurations (presets) for an associated audio software plugin.

Its primary function is to provide a user interface for browsing, loading, and previewing different saved sounds.

**How It Works:**

1.  **Initialization:** When the component starts, it reads its setup instructions, determining which specific audio plugin it controls, defining its visual style (colors and font size), and setting the default folder where presets are stored (e.g., `data/presets`).
2.  **File Management:** It uses an internal utility to navigate this designated folder, tracking the list of available preset files.
3.  **Loading Presets:** When a user selects a preset, the component opens the corresponding file, reads the saved settings data, and immediately applies those configurations to the audio plugin, changing its sound.
4.  **Backup Feature:** Before loading any new file, the component automatically creates a temporary "backup" of the plugin's existing configuration. This allows the user to easily restore the original settings if they decide they don't like the new preset.
5.  **User Control:** The component is designed to react to physical controls, specifically managing an *encoder* (a rotary dial) to scroll through the list of presets displayed on the screen. It also includes functions to briefly "audition" or preview a preset by triggering a small sound before the full settings are loaded.
6.  **Visuals:** It handles its own appearance, using defined colors for the background, text, and a scroll indicator, ensuring the component fits seamlessly into the larger application interface.

sha: feb0715ae79c3bd37ff111ad640285402933112705175e87747b14b15f146305
*/
#pragma once

#include "audio/fileBrowser.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <fstream>
#include <string>

class PresetComponent : public Component {
protected:
    Color bgColor;
    Color textColor;
    Color textEditColor;
    Color foregroundColor;
    Color scrollColor;

    int fontSize = 12;
    void* font = NULL;

    int8_t encoderId = 1;

    AudioPlugin* audioPlugin = nullptr;
    std::string folder = "data/presets";

    FileBrowser fileBrowser;

    int loaded = -1;

    int visibleItems = 1;

    nlohmann::json backup;
    void load()
    {
        if (loaded == -1) {
            backup = nlohmann::json();
            audioPlugin->serializeJson(backup);
        }

        std::string filepath = fileBrowser.getFilePath(fileBrowser.position);
        std::ifstream file(filepath);
        if (file.is_open()) {
            nlohmann::json json;
            file >> json;
            audioPlugin->hydrateJson(json);
            file.close();
            loaded = fileBrowser.position;
        }
        //  else {
        //     logError("Preset file not found: %s", filepath.c_str());
        // }
    }

    void trig(bool isOn)
    {
        if (audioPlugin) {
            if (isOn) {
                audioPlugin->noteOn(60, 1.0f);
            } else {
                audioPlugin->noteOff(60, 0.0f);
            }
        }
    }

public:
    PresetComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;

            if (action == ".loadTrig") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (loaded == fileBrowser.position) {
                        trig(KeypadLayout::isPressed(keymap));
                    } else if (KeypadLayout::isReleased(keymap)) {
                        load();
                    }
                };
            }

            if (action == ".restore") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        audioPlugin->hydrateJson(backup);
                        loaded = -1;
                    }
                };
            }

            if (action == ".exit") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    loaded = -1;
                };
            }

            if (action == ".delete") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isPressed(keymap)) {
                        std::filesystem::remove(fileBrowser.getFilePath(fileBrowser.position));
                        int pos = fileBrowser.position;
                        fileBrowser.openFolder(folder);
                        fileBrowser.position = pos > fileBrowser.count ? fileBrowser.count : pos;
                        renderNext();
                    }
                };
            }

            if (action == ".up") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isPressed(keymap)) {
                        scroll(-1);
                    }
                };
            }

            if (action == ".down") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isPressed(keymap)) {
                        scroll(1);
                    }
                };
            }

            return func;
        })
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , textEditColor(darken(styles.colors.text, 0.5))
        , foregroundColor(lighten(styles.colors.background, 0.5))
        , scrollColor(lighten(styles.colors.background, 1.0))
    {
        nlohmann::json& config = props.config;
        audioPlugin = getPluginPtr(config, "audioPlugin", track); //eg: "audio_plugin_name"

        folder = config.value("folder", folder);

        bgColor = draw.getColor(config["bgColor"], bgColor);
        textColor = draw.getColor(config["textColor"], textColor);
        foregroundColor = draw.getColor(config["foregroundColor"], foregroundColor);
        scrollColor = draw.getColor(config["scrollColor"], scrollColor); //eg: "#000000"

        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str());
            fontSize = draw.getDefaultFontSize(font);
        }
        fontSize = config.value("fontSize", fontSize);

        encoderId = config.value("encoderId", encoderId);

        fileBrowser.openFolder(folder);

        visibleItems = config.value("visibleItems", visibleItems);
        if (visibleItems < 1) visibleItems = 1;
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });

        // ===== SINGLE ITEM MODE (UNCHANGED LOOK) =====
        if (visibleItems <= 1) {
            draw.filledRect(relativePosition, size, { foregroundColor });

            if (fileBrowser.count > 1) {
                int scrollW = size.w * ((float)(fileBrowser.position - 1) / (float)(fileBrowser.count - 1));
                draw.filledRect(relativePosition, { scrollW, 2 }, { scrollColor });
            }

            int textY = relativePosition.y + (size.h - fontSize) * 0.5;
            std::string filename = fileBrowser.getFileWithoutExtension(fileBrowser.position);
            draw.text(
                { relativePosition.x + 4, textY },
                filename,
                fontSize,
                { textEditColor, .font = font });
            return;
        }

        // ===== LIST MODE =====
        int itemHeight = size.h / visibleItems;
        if (itemHeight < fontSize + 2)
            itemHeight = fontSize + 2;

        int maxVisible = size.h / itemHeight;
        if (maxVisible < 1)
            maxVisible = 1;

        int total = fileBrowser.count;
        int selected = fileBrowser.position;

        // Center selected item when possible
        int firstIndex = selected - maxVisible / 2;
        if (firstIndex < 1)
            firstIndex = 1;
        if (firstIndex > total - maxVisible + 1)
            firstIndex = std::max(1, total - maxVisible + 1);

        int lastIndex = std::min(total, firstIndex + maxVisible - 1);

        // Draw list items
        for (int i = firstIndex; i <= lastIndex; ++i) {
            int idx = i - firstIndex;
            Point itemPos = {
                relativePosition.x,
                relativePosition.y + idx * itemHeight
            };
            Size itemSize = { size.w, itemHeight };

            bool isSelected = (i == selected);

            if (isSelected) {
                draw.filledRect(itemPos, itemSize, { foregroundColor });
            }

            int textY = itemPos.y + (itemHeight - fontSize) * 0.5;
            std::string name = fileBrowser.peekFileWithoutExtension(i);

            draw.text(
                { itemPos.x + 4, textY },
                name,
                fontSize,
                { isSelected ? textEditColor : textColor, .font = font });
        }

        // ===== SCROLLBAR =====
        if (total > maxVisible) {
            float ratio = (float)maxVisible / (float)total;
            int barHeight = size.h * ratio;
            if (barHeight < 4)
                barHeight = 4;

            float scrollRatio = (float)(firstIndex - 1) / (float)(total - maxVisible);
            int barY = relativePosition.y + scrollRatio * (size.h - barHeight);

            Point barPos = {
                relativePosition.x + size.w - 3,
                barY
            };
            Size barSize = { 3, barHeight };

            draw.filledRect(barPos, barSize, { scrollColor });
        }
    }

    void onEncoder(int8_t id, int8_t direction) override
    {
        if (id == encoderId) {
            scroll(direction);
        }
    }

    void scroll(int8_t direction)
    {
        int position = fileBrowser.position + direction;
        if (position < 1)
            position = 1;
        if (position > fileBrowser.count)
            position = fileBrowser.count;

        fileBrowser.position = position;
        renderNext();
    }

    const std::vector<EventInterface::EncoderPosition> getEncoderPositions() override
    {
        if (encoderId < 0) {
            return {};
        }

        return {
            { encoderId, size, relativePosition },
        };
    }
};
