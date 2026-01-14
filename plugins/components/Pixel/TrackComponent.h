#pragma once

#include "helpers/clamp.h"
#include "helpers/getTicks.h"
#include "plugins/components/base/Icon.h"
#include "plugins/components/component.h"

#include <string>

/*md
## Track
*/

class TrackComponent : public Component {
protected:
    Icon icon;

    Color bgColor;
    Color iconColor;

    int iconSize = 8;

    AudioPlugin* pluginSeq = NULL;
    ValueInterface* valSeqStatus = NULL;

public:
    TrackComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".mute") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (valSeqStatus->get() == 1) {
                            valSeqStatus->set(0);
                        } else {
                            valSeqStatus->set(1);
                        }
                    }
                };
            }

            return func;
        })
        , icon(props.view->draw)
        , bgColor(styles.colors.background)
        , iconColor(styles.colors.text)
    {
        /*md md_config:Rect */
        nlohmann::json& config = props.config;

        /// The background color.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// The color of the text
        iconColor = draw.getColor(config["color"], iconColor); //eg: "#ffffff"

        /// The font size of the text.
        iconSize = config.value("iconSize", iconSize); //eg: 8

        /// Sequencer plugin
        pluginSeq = &getPlugin(config.value("sequencerPlugin", "Sequencer"), track); //eg: "audio_plugin_name"
        if (pluginSeq != NULL) {
            valSeqStatus = watch(pluginSeq->getValue("STATUS"));
        }

        /*md md_config_end */
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });

        if (valSeqStatus->get() == 0) {
            icon.render("&icon::mute::filled", relativePosition, { iconSize, iconSize }, { iconColor });
        }
    }
};
