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
    Color textMissingColor;
    Color playingClipBgColor;
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

    int clipW = 0;
    int visibleCount = 10;
    int startIndex = 1;

    bool allowToggleReload = false;

    std::vector<bool> exists;

    void loadExists() {
        for (int i = 0; i < visibleCount; i++) {
            int16_t id = i + startIndex;
            exists.push_back(pluginSerialize->data(pluginSerialize->getDataId("VARIATION_EXISTS"), &id) != NULL);
        }
    }

public:
    ClipsComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".reload") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        int16_t id = (int16_t)valVariation->get();
                        // logDebug("reload variation %d", id);
                        pluginSerialize->data(loadVariationDataId, (void*)&id);
                    }
                };
            }

            if (action.rfind(".toggle:") == 0) {
                int16_t id = std::stoi(action.substr(8));
                func = [this, id](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if ((int16_t)valVariation->get() == id) {
                            if (!*isPlaying) {
                                if (allowToggleReload) {
                                    pluginSerialize->data(loadVariationDataId, (void*)&id);
                                } // else do nothing
                            } else if (valSeqStatus->get() == 1) {
                                valSeqStatus->set(0);
                            } else {
                                valSeqStatus->set(1);
                            }
                            // Ultimately we could check if sequencer is playing so it only start next if it is...
                            // however, to do this, we would have to track another data id again: IS_PLAYING
                            // for the moment let's stick to double press, it is fine...
                        } else if (nextVariationToPlay != NULL && *nextVariationToPlay == -1) {
                            if (!*isPlaying) {
                                pluginSerialize->data(loadVariationDataId, (void*)&id);
                            } else if (valSeqStatus->get() == 1) {
                                pluginSerialize->data(loadVariationNextDataId, (void*)&id);
                            }
                        } else {
                            pluginSerialize->data(loadVariationDataId, (void*)&id);
                        }
                        renderNext();
                    }
                };
            }
            if (action.rfind(".save:") == 0) {
                int16_t id = std::stoi(action.substr(6));
                func = [this, id](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (pluginSerialize) {
                            pluginSerialize->data(saveVariationDataId, (void*)&id);
                            exists[id - startIndex] = true;
                            // valVariation->set(id);
                            renderNext();
                        }
                    }
                };
            }
            if (action.rfind(".delete:") == 0) {
                int16_t id = std::stoi(action.substr(8));
                func = [this, id](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        pluginSerialize->data(deleteVariationDataId, (void*)&id);
                        exists[id - startIndex] = false;
                        renderNext();
                    }
                };
            }
            return func;
        })
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , textMissingColor(alpha(styles.colors.text, 0.2))
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
        textMissingColor = alpha(styles.colors.text, 0.2);

        /// The color of the text
        playColor = draw.getColor(config["playColor"], playColor); //eg: "#ffffff"

        /// The color of the text
        playNextColor = draw.getColor(config["playNextColor"], playNextColor); //eg: "#ffffff"

        /// The color of the text
        playingClipBgColor = draw.getColor(config["color"], playingClipBgColor); //eg: "#ffffff"
        clipBgColor = alpha(playingClipBgColor, 0.5); //eg: "#ffffff"

        /// The number of visible clips
        visibleCount = config.value("visibleCount", visibleCount); //eg: 10

        /// The sequencer audio plugin.
        pluginSeq = &getPlugin(config.value("sequencerPlugin", "Sequencer"), track); //eg: "audio_plugin_name"
        isPlaying = (bool*)pluginSeq->data(pluginSeq->getDataId("IS_PLAYING"));
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

        // Allow .toggle action to reload selected variation
        if (config.contains("allowToggleReload")) {
            allowToggleReload = config["allowToggleReload"];
        }

        // std::string controllerId = config.value("controller", "Default");
        // controller = getController(controllerId.c_str());

        /*md md_config_end */

        resize();

        jobRendering = [this](unsigned long now) {
            if (lastIsPlaying != *isPlaying || (*isPlaying && lastPlayingId != valVariation->get())) {
                lastIsPlaying = *isPlaying;
                lastPlayingId = valVariation->get();
                renderNext();
            }
        };

        loadExists();
    }
    bool lastIsPlaying = false;
    int lastPlayingId = -1;

    void resize() override
    {
        clipW = size.w / visibleCount;
    }

    // ControllerInterface* controller = NULL;
    // void initView(uint16_t counter) override
    // {
    //     Component::initView(counter);
    //     // loadExists();
    //     renderKeys();
    // }
    // void renderKeys()
    // {
    //     if (controller) {
    //     }
    // }

    void render()
    {
        draw.filledRect(relativePosition, size, { bgColor });
        int playingId = valVariation ? valVariation->get() : -1;

        Point center = { (int)(clipW * 0.5), (int)((size.h - fontSize - 1) * 0.5) };
        for (int i = 0; i < visibleCount; i++) {
            int id = i + startIndex;
            Point pos = { relativePosition.x + i * clipW, relativePosition.y };
            draw.filledRect({ relativePosition.x + i * clipW, relativePosition.y }, { clipW - 2, size.h }, { id == playingId ? playingClipBgColor : clipBgColor });
            Color& color = exists[i] ? textColor : textMissingColor;
            draw.textCentered({ pos.x + center.x, pos.y + center.y }, std::to_string(i + 1), fontSize, { color, .maxWidth = clipW });

            if (*isPlaying) {
                if (id == *nextVariationToPlay) {
                    draw.filledRect({ pos.x + 2, pos.y + 2 }, { 3, 3 }, { playNextColor });
                } else if (valSeqStatus && id == playingId) {
                    if (valSeqStatus->get() == 1) {
                        draw.filledRect({ pos.x + 2, pos.y + 2 }, { 3, 3 }, { playColor });
                    } else if (valSeqStatus->get() == 2) {
                        draw.filledRect({ pos.x + 2, pos.y + 2 }, { 3, 3 }, { playNextColor });
                    }
                }
            }
        }
    }
};
