#pragma once

#include "helpers/range.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <string>
#include <vector>

/*md
## Clips

Clips components to draw a rectangle.
*/

class ClipsComponent : public Component {
protected:
    Color bgColor;
    Color textColor;
    Color clipBgColor;
    Color playColor;
    Color playNextColor;

    int fontSize = 12;

    AudioPlugin* pluginSerialize = NULL;
    ValueInterface* valVariation = NULL;
    uint8_t loadVariationDataId = -1;
    uint8_t loadVariationNextDataId = -1;
    uint8_t saveVariationDataId = -1;
    uint8_t deleteVariationDataId = -1;
    AudioPlugin* pluginSeq = NULL;
    ValueInterface* valSeqStatus = NULL;
    bool* isPlaying = NULL;
    int* nextVariationToPlay = NULL;

    struct Variation {
        bool exists;
        // std::string filepath;
    };
    std::vector<Variation> variations;

    int clipW = 0;
    int visibleCount = 10;

public:
    ClipsComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".toggle") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        // int16_t id = view->contextVar[selectionBank];
                        // if (variations[id].exists) {
                        //     if (isGroupAll) {
                        //         pluginSerialize->data(loadVariationNextDataId, &id);
                        //     } else if ((int16_t)valVariation->get() == id) {
                        //         if (!*isPlaying) {
                        //             pluginSerialize->data(loadVariationDataId, &id);
                        //         } else if (valSeqStatus->get() == 1) {
                        //             valSeqStatus->set(0);
                        //         } else {
                        //             valSeqStatus->set(1);
                        //         }
                        //         // Ultimately we could check if sequencer is playing so it only start next if it is...
                        //         // however, to do this, we would have to track another data id again: IS_PLAYING
                        //         // for the moment let's stick to double press, it is fine...
                        //     } else if (nextVariationToPlay != NULL && *nextVariationToPlay == -1) {
                        //         if (!*isPlaying) {
                        //             pluginSerialize->data(loadVariationDataId, &id);
                        //         } else if (valSeqStatus->get() == 1) {
                        //             pluginSerialize->data(loadVariationNextDataId, &id);
                        //         }
                        //     } else {
                        //         pluginSerialize->data(loadVariationDataId, &id);
                        //     }
                        //     renderNext();
                        // }
                    }
                };
            }
            if (action == ".save") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    // if (KeypadLayout::isReleased(keymap)) {
                    //     int16_t id = view->contextVar[selectionBank];
                    //     if (pluginSerialize) {
                    //         pluginSerialize->data(saveVariationDataId, (void*)&id);
                    //         variations[id].exists = true;
                    //         // valVariation->set(id);
                    //         renderNext();
                    //     }
                    // }
                };
            }
            if (action == ".delete") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    // if (KeypadLayout::isReleased(keymap)) {
                    //     int16_t id = view->contextVar[selectionBank];
                    //     if (variations[id].exists) {
                    //         pluginSerialize->data(deleteVariationDataId, (void*)&id);
                    //         variations[id].exists = false;
                    //         renderNext();
                    //     }
                    // }
                };
            }
            if (action == ".saveOrDelete") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        // int16_t id = view->contextVar[selectionBank];
                        // if (variations[id].exists) {
                        //     pluginSerialize->data(deleteVariationDataId, (void*)&id);
                        //     variations[id].exists = false;
                        //     renderNext();
                        // } else {
                        //     pluginSerialize->data(saveVariationDataId, (void*)&id);
                        //     variations[id].exists = true;
                        //     // valVariation->set(id);
                        //     renderNext();
                        // }
                    }
                };
            }
            return func;
        })
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , playColor(rgb(35, 161, 35))
        , playNextColor(rgb(253, 111, 14))
        , clipBgColor(styles.colors.primary)
    {
        /*md md_config:Clips */
        nlohmann::json& config = props.config;

        /// The background color of the text.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// The color of the text
        textColor = draw.getColor(config["textColor"], textColor); //eg: "#ffffff"

        /// The color of the text
        playColor = draw.getColor(config["playColor"], playColor); //eg: "#ffffff"

        /// The color of the text
        playNextColor = draw.getColor(config["playNextColor"], playNextColor); //eg: "#ffffff"

        /// The color of the text
        clipBgColor = draw.getColor(config["color"], clipBgColor); //eg: "#ffffff"

        /// The number of visible clips
        visibleCount = config.value("visibleCount", visibleCount); //eg: 10

        /// The sequencer audio plugin.
        pluginSeq = &getPlugin(config.value("sequencerPlugin", "Sequencer"), track); //eg: "audio_plugin_name"
        if (pluginSeq != NULL) {
            valSeqStatus = watch(pluginSeq->getValue("STATUS"));
        }
        isPlaying = (bool*)pluginSeq->data(pluginSeq->getDataId("IS_PLAYING"));

        /// The audio plugin to get serialized data.
        pluginSerialize = &getPlugin(config.value("serializerPlugin", "SerializeTrack"), track); //eg: "audio_plugin_name"
        if (!pluginSerialize) {
            logWarn("Clips component is missing serializerPlugin.");
            return;
        }

        valVariation = watch(pluginSerialize->getValue("VARIATION"));
        saveVariationDataId = pluginSerialize->getDataId("SAVE_VARIATION");
        deleteVariationDataId = pluginSerialize->getDataId("DELETE_VARIATION");
        loadVariationDataId = pluginSerialize->getDataId("LOAD_VARIATION");
        loadVariationNextDataId = pluginSerialize->getDataId("LOAD_VARIATION_NEXT");
        int nextVariation = -1;
        nextVariationToPlay = (int*)pluginSerialize->data(loadVariationNextDataId, &nextVariation);

        for (int i = 0; i < valVariation->props().max; i++) {
            bool exists = pluginSerialize->data(pluginSerialize->getDataId("GET_VARIATION"), &i) != NULL;
            variations.push_back({ exists });
        }

        /*md md_config_end */

        resize();
    }

    void resize() override
    {
        clipW = size.w / visibleCount;
    }

    void render()
    {
        draw.filledRect(relativePosition, size, { bgColor });
        int playingId = valVariation ? valVariation->get() : -1;

        Point center = { (int)(clipW * 0.5), (int)((size.h - fontSize - 1) * 0.5) };
        for (int i = 0; i < visibleCount; i++) {
            Point pos = { relativePosition.x + i * clipW, relativePosition.y };
            draw.filledRect({ relativePosition.x + i * clipW, relativePosition.y }, { clipW - 2, size.h }, { clipBgColor });
            draw.textCentered({ pos.x + center.x, pos.y + center.y }, std::to_string(i + 1), fontSize, { textColor, .maxWidth = clipW });
        }

        // int selection = view->contextVar[selectionBank];
        // int start = selection >= visibleCount ? selection - visibleCount + 1 : 0;
        // for (int i = start, v = 0; i < count && v < visibleCount; i++, v++) {
        //     Variation& variation = variations[i];
        //     // int y = relativePosition.y + i * clipH;
        //     int y = relativePosition.y + (i - start) * clipH;

        //     bool selected = (isActive || isGroupAll) && i == selection;
        //     draw.filledRect({ relativePosition.x, y }, { size.w, clipH - 1 }, { selected ? selectionColor : foreground });

        //     if (variation.exists && (i == playingId || i == *nextVariationToPlay)) {
        //         // draw.filledRect({ relativePosition.x, y }, { size.w, clipH - 1 }, { darken(separatorColor, 0.8) });
        //         // draw.filledRect({ relativePosition.x, y }, { size.w, 2 }, { separatorColor });

        //         if (i == *nextVariationToPlay) {
        //             draw.filledRect({ relativePosition.x + size.w - 5, y + 3 }, { 3, 3 }, { playNextColor });
        //         } else if (valSeqStatus) {
        //             if (valSeqStatus->get() == 1) {
        //                 draw.filledRect({ relativePosition.x + size.w - 5, y + 3 }, { 3, 3 }, { playColor });
        //             } else if (valSeqStatus->get() == 2) {
        //                 draw.filledRect({ relativePosition.x + size.w - 5, y + 3 }, { 3, 3 }, { playNextColor });
        //             }
        //         }
        //     } else {
        //         // draw.filledRect({ relativePosition.x, y }, { size.w, clipH - 1 }, { foreground });
        //         // draw.filledRect({ relativePosition.x, y }, { size.w, 1 }, { darken(separatorColor, 0.3) });
        //     }

        //     if (variation.exists) {
        //         draw.textCentered({ relativePosition.x + (int)(size.w * 0.5), y + (int)((clipH - 12) * 0.5) }, std::to_string(i + 1), 12, { textColor, .maxWidth = size.w });
        //     }
        // }
    }
};
