#pragma once

#include "helpers/clamp.h"
#include "helpers/getTicks.h"
#include "plugins/components/base/Icon.h"
#include "plugins/components/component.h"

#include <string>

/*md
## Clip
*/

class ClipComponent : public Component {
protected:
    Icon icon;

    Color bgColor;
    Color textColor;

    int fontSize = 12;
    void* font = NULL;

    int smallFontSize = 8;
    void* smallFont = NULL;

    AudioPlugin* pluginSerialize = NULL;
    ValueInterface* valClip = NULL;
    int* nextClipToPlay = NULL;
    uint8_t loadClipNextDataId = -1;
    AudioPlugin* pluginSeq = NULL;
    ValueInterface* valSeqStatus = NULL;
    bool* isPlaying = NULL;

    int8_t encoderId = -1;
    int selectedClip = -1;
    uint64_t selectionTime = 0;

public:
    ClipComponent(ComponentInterface::Props props)
        : Component(props)
        , icon(props.view->draw)
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
    {
        /*md md_config:Rect */
        nlohmann::json& config = props.config;

        /// The background color.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// The color of the text
        textColor = draw.getColor(config["color"], textColor); //eg: "#ffffff"

        /// The font of the text. Default is null.
        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str()); //eg: "Sinclair_S"
            fontSize = draw.getDefaultFontSize(font);
        }
        /// The font size of the text.
        fontSize = config.value("fontSize", fontSize); //eg: 8

        /// The font of the text. Default is null.
        if (config.contains("smallFont")) {
            smallFont = draw.getFont(config["smallFont"].get<std::string>().c_str()); //eg: "Sinclair_S"
            smallFontSize = draw.getDefaultFontSize(smallFont);
        }
        /// The font size of the text.
        smallFontSize = config.value("smallFontSize", smallFontSize); //eg: 8

        /// The audio plugin to get serialized data.
        pluginSerialize = &getPlugin(config.value("serializerPlugin", "SerializeTrack"), track); //eg: "audio_plugin_name"
        if (!pluginSerialize) {
            logWarn("Clips component is missing serializerPlugin.");
            return;
        }

        valClip = watch(pluginSerialize->getValue("CLIP"));
        loadClipNextDataId = pluginSerialize->getDataId("LOAD_CLIP_NEXT");
        int nextClip = -1;
        nextClipToPlay = (int*)pluginSerialize->data(loadClipNextDataId, &nextClip);

        pluginSeq = &getPlugin(config.value("sequencerPlugin", "Sequencer"), track); //eg: "audio_plugin_name"
        if (pluginSeq != NULL) {
            isPlaying = (bool*)pluginSeq->data(pluginSeq->getDataId("IS_PLAYING"));
            valSeqStatus = watch(pluginSeq->getValue("STATUS"));
        }

        /// The encoder id that will interract with this component.
        encoderId = config.value("encoderId", encoderId); //eg: 0

        /*md md_config_end */

        jobRendering = [this](unsigned long now) {
            if (selectedClip != -1) {
                if (now - selectionTime > 10000) { // > 10s
                    selectedClip = -1;
                    renderNext();
                }
            }
        };
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });

        draw.text({ relativePosition.x + 2, relativePosition.y }, "Clip", smallFontSize, { textColor, .font = smallFont });

        draw.textCentered({ relativePosition.x + size.w / 2, relativePosition.y + smallFontSize }, valClip ? std::to_string((int)valClip->get()) : "NULL", fontSize * 2, { textColor, .font = font, .maxWidth = size.w - 4 });

        if (selectedClip != -1) {
            renderInfo("Select " + std::to_string(selectedClip));
        } else if (nextClipToPlay != NULL && *nextClipToPlay != -1) {
            renderInfoAndIcon("Next " + std::to_string(*nextClipToPlay), "&icon::play::filled");
            // Put a second play icon next
            renderIcon("&icon::play::filled", smallFontSize + 4);
        } else if (isPlaying != NULL && *isPlaying) {
            if (valSeqStatus->get() == 0) {
                renderInfoAndIcon("Muted", "&icon::mute::filled");
            } else {
                renderInfoAndIcon("Playing", "&icon::play::filled");
            }
        } else {
            renderInfoAndIcon("Stopped", "&icon::stop::filled");
        }
    }

    void renderInfoAndIcon(std::string info, std::string iconName)
    {
        renderInfo(info);
        renderIcon(iconName);
    }

    void renderInfo(std::string info)
    {
        draw.textCentered({ relativePosition.x + size.w / 2, relativePosition.y + size.h - fontSize - fontSize / 2 }, info, fontSize, { textColor, .font = font, .maxWidth = size.w - 4 });
    }

    void renderIcon(std::string iconName, int marginLeft = 0)
    {
        icon.render(iconName, { relativePosition.x + size.w - smallFontSize - 2 - marginLeft, relativePosition.y + 2 }, { smallFontSize - 2, smallFontSize - 2 }, { textColor });
    }

    void onEncoder(int8_t id, int8_t direction) override
    {
        if (valClip && id == encoderId) {
            if (selectedClip < 0) {
                selectedClip = valClip->get();
            } else {
                selectedClip = CLAMP(selectedClip + direction, 0, valClip->props().max);
            }
            selectionTime = getTicks();
            renderNext();
        }
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
