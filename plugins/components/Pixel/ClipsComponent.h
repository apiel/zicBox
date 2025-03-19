#pragma once

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
    bool isActive = true;

    int8_t groupAll = -1;
    bool isGroupAll = false;

    Color bgColor;
    Color foreground;
    Color textColor;
    Color barColor;
    Color playColor;
    Color playNextColor;
    Color selectionColor;

    AudioPlugin* pluginSerialize = NULL;
    ValueInterface* valVariation = NULL;
    uint8_t loadVariationDataId = -1;
    uint8_t loadVariationNextDataId = -1;
    uint8_t saveVariationDataId = -1;
    uint8_t deleteVariationDataId = -1;
    ValueInterface* valSeqStatus = NULL;
    int* nextVariationToPlay = NULL;
    bool showSelectionRect = false;

    struct Variation {
        bool exists;
        // std::string filepath;
    };
    std::vector<Variation> variations;

    int clipH = 17;

    int visibleCount = 16;

    uint8_t selectionBank = 30;

    bool shouldDoAction()
    {
        // Not all action should happen when it is in groupAll mode
        // For example, up/down should not happen in all clips component,
        // else instead to go up/down for one step, it will go as much as clip group exist...
        if (!isGroupAll || groupAll - 1 == group) {
            return true;
        }
        return false;
    }

public:
    ClipsComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".up") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (shouldDoAction() && view->contextVar[selectionBank] > 0) {
                            setContext(selectionBank, view->contextVar[selectionBank] - 1);
                        }
                        renderNext();
                    }
                };
            }
            if (action == ".down") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (shouldDoAction() && view->contextVar[selectionBank] < valVariation->props().max - 1) {
                            setContext(selectionBank, view->contextVar[selectionBank] + 1);
                        }
                        renderNext();
                    }
                };
            }
            if (action == ".save") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        int16_t id = view->contextVar[selectionBank];
                        if (pluginSerialize) {
                            pluginSerialize->data(saveVariationDataId, (void*)&id);
                            variations[id].exists = true;
                            // valVariation->set(id);
                            renderNext();
                        }
                    }
                };
            }
            if (action == ".toggle") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        int16_t id = view->contextVar[selectionBank];
                        if (variations[id].exists) {
                            if (isGroupAll) {
                                pluginSerialize->data(loadVariationNextDataId, &id);
                            } else if (valVariation->get() == id) {
                                if (valSeqStatus->get() == 1) {
                                    valSeqStatus->set(0);
                                } else {
                                    valSeqStatus->set(1);
                                }
                                // Ultimately we could check if sequencer is playing so it only start next if it is...
                                // however, to do this, we would have to track another data id again: IS_PLAYING
                                // for the moment let's stick to double press, it is fine...
                            } else if (nextVariationToPlay != NULL && *nextVariationToPlay == -1) {
                                pluginSerialize->data(loadVariationNextDataId, &id);
                            } else {
                                pluginSerialize->data(loadVariationDataId, &id);
                            }
                            renderNext();
                        }
                    }
                };
            }
            if (action == ".delete") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        int16_t id = view->contextVar[selectionBank];
                        if (variations[id].exists) {
                            pluginSerialize->data(deleteVariationDataId, (void*)&id);
                            variations[id].exists = false;
                            renderNext();
                        }
                    }
                };
            }
            return func;
        })
        , bgColor(styles.colors.background)
        , foreground(rgb(53, 53, 53))
        , selectionColor(lighten(foreground, 0.3))
        , textColor(styles.colors.text)
        , barColor(styles.colors.primary)
        , playColor(rgb(35, 161, 35))
        , playNextColor(rgb(253, 111, 14))
    {
        /*md md_config:Clips */
        nlohmann::json& config = props.config;

        /// The group to change clips for all tracks.
        groupAll = config.value("groupAll", groupAll); //eg: 10

        /// The background color of the text.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// Selection forground color.
        selectionColor = draw.getColor(config["selectionColor"], selectionColor); //eg: "#ffffff"

        /// The color of the text
        textColor = draw.getColor(config["textColor"], textColor); //eg: "#ffffff"

        /// The identifier color of the component...
        barColor = draw.getColor(config["color"], barColor); //eg: "#ffffff"

        /// Show rectangle around selected clip.
        showSelectionRect = config.value("showSelectionRect", showSelectionRect); //eg: true

        /// The number of visible clips
        visibleCount = config.value("visibleCount", visibleCount); //eg: 10

        /// The sequencer audio plugin.
        AudioPlugin* pluginSeq = &getPlugin(config.value("sequencerPlugin", "Sequencer"), track); //eg: "audio_plugin_name"
        if (pluginSeq != NULL) {
            valSeqStatus = watch(pluginSeq->getValue("STATUS"));
        }

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
    }

    void render()
    {
        draw.filledRect(relativePosition, size, { bgColor });
        if (valVariation) {
            int playingId = valVariation->get();
            int count = variations.size();
            int selection = view->contextVar[selectionBank];
            int start = selection >= visibleCount ? selection - visibleCount + 1 : 0;
            for (int i = start, v = 0; i < count && v < visibleCount; i++, v++) {
                Variation& variation = variations[i];
                // int y = relativePosition.y + i * clipH;
                int y = relativePosition.y + (i - start) * clipH;

                bool selected = (isActive || isGroupAll) && i == selection;
                draw.filledRect({ relativePosition.x, y }, { size.w, clipH - 1 }, { selected ? selectionColor : foreground });

                if (variation.exists && (i == playingId || i == *nextVariationToPlay)) {
                    // draw.filledRect({ relativePosition.x, y }, { size.w, clipH - 1 }, { darken(barColor, 0.8) });
                    draw.filledRect({ relativePosition.x, y }, { size.w, 2 }, { barColor });

                    if (i == *nextVariationToPlay) {
                        draw.filledRect({ relativePosition.x + size.w - 5, y + 3 }, { 3, 3 }, { playNextColor });
                    } else if (valSeqStatus) {
                        if (valSeqStatus->get() == 1) {
                            draw.filledRect({ relativePosition.x + size.w - 5, y + 3 }, { 3, 3 }, { playColor });
                        } else if (valSeqStatus->get() == 2) {
                            draw.filledRect({ relativePosition.x + size.w - 5, y + 3 }, { 3, 3 }, { playNextColor });
                        }
                    }
                } else {
                    // draw.filledRect({ relativePosition.x, y }, { size.w, clipH - 1 }, { foreground });
                    draw.filledRect({ relativePosition.x, y }, { size.w, 1 }, { darken(barColor, 0.3) });
                }

                if (variation.exists) {
                    draw.textCentered({ relativePosition.x + (int)(size.w * 0.5), y + (int)((clipH - 12) * 0.5) }, std::to_string(i + 1), 12, { textColor, .maxWidth = size.w });
                }

                if (showSelectionRect && selected) {
                    draw.rect({ relativePosition.x, y }, { size.w, clipH - 2 }, { barColor });
                }
            }
        }
    }

    bool onKey(uint16_t id, int key, int8_t state, unsigned long now) override
    {
        if (isActive || isGroupAll) {
            return Component::onKey(id, key, state, now);
        }
        return false;
    }

    void onGroupChanged(int8_t index) override
    {
        // printf("current group: %d inccoming group: %d\n", group, index);
        if (isActive) {
            renderNext();
        }

        bool shouldActivate = false;
        if (group == index || group == -1) {
            shouldActivate = true;
        }
        if (shouldActivate != isActive) {
            isActive = shouldActivate;
            renderNext();
        }

        bool shouldUpdateGroupAll = false;
        if (groupAll == index) {
            shouldUpdateGroupAll = true;
        }
        if (shouldUpdateGroupAll != isGroupAll) {
            isGroupAll = shouldUpdateGroupAll;
            renderNext();
        }
    }

    void onContext(uint8_t index, float value) override
    {
        if (index == selectionBank && shouldDoAction()) {
            renderNext();
        }
    }
};
