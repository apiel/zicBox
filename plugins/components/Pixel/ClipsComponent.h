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
    uint8_t variationExistsDataId = -1;
    AudioPlugin* pluginSeq = NULL;
    ValueInterface* valSeqStatus = NULL;
    bool* isPlaying = NULL;
    int* nextVariationToPlay = NULL;

    int clipW = 0;
    int visibleCount = 10;
    int startBankIndex = 1;
    int addIndex = 0;
    std::string bank = "A";
    int bankSize = 100;
    std::string redirectView = "";

    uint8_t renderContextId = 0;

    bool allowToggleReload = false;

    bool variationExists(int id) { return pluginSerialize->data(variationExistsDataId, &id) != NULL; }

    void redirect()
    {
        logDebug("redirecting to " + redirectView);
        if (redirectView != "") {
            logDebug("redirect done");
            view->setView(redirectView);
        }
    }

    void toggleAction(KeypadLayout::KeyMap& keymap, int16_t id)
    {
        if (KeypadLayout::isReleased(keymap)) {
            int idAndBank = getIdWithBank(id);
            if ((int16_t)valVariation->get() == idAndBank) {
                if (!*isPlaying) {
                    if (allowToggleReload) {
                        pluginSerialize->data(loadVariationDataId, (void*)&idAndBank);
                        redirect();
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
                    pluginSerialize->data(loadVariationDataId, (void*)&idAndBank);
                    redirect();
                } else if (valSeqStatus->get() == 1) {
                    pluginSerialize->data(loadVariationNextDataId, (void*)&idAndBank);
                }
            } else {
                pluginSerialize->data(loadVariationDataId, (void*)&idAndBank);
                redirect();
            }
            renderNextAndContext();
        }
    }

    void bankAction(KeypadLayout::KeyMap& keymap, char c)
    {
        if (KeypadLayout::isReleased(keymap)) {
            bank = std::string(1, c);
            startBankIndex = 1 + (c - 'A') * bankSize;
            renderNextAndContext();
        }
    }

    void renderNextAndContext()
    {
        if (renderContextId > 0) {
            setContext(renderContextId, startBankIndex);
        }
        renderNext();
    }

    int getIdWithBank(int id) { return id + startBankIndex - 1; }

    bool bankToggle = false;
    int savedMessage = 0;

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

            if (action.rfind(".bank:") == 0) {
                char c = action[6];
                func = [this, c](KeypadLayout::KeyMap& keymap) {
                    bankAction(keymap, c);
                };
            }

            if (action == ".bank") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    bankToggle = KeypadLayout::isPressed(keymap);
                    renderNext();
                };
            }

            if (action.rfind(".set:") == 0) {
                int16_t id = std::stoi(action.substr(5));
                func = [this, id](KeypadLayout::KeyMap& keymap) {
                    if (bankToggle) {
                        char c = 'A' + (id - 1);
                        bankAction(keymap, c);
                    } else {
                        toggleAction(keymap, id);
                    }
                };
            }

            if (action.rfind(".toggle:") == 0) {
                int16_t id = std::stoi(action.substr(8));
                func = [this, id](KeypadLayout::KeyMap& keymap) {
                    toggleAction(keymap, id);
                };
            }
            if (action.rfind(".save:") == 0) {
                int16_t id = std::stoi(action.substr(6));
                func = [this, id](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (pluginSerialize) {
                            int idAndBank = getIdWithBank(id);
                            pluginSerialize->data(saveVariationDataId, (void*)&idAndBank);
                            // valVariation->set(id);
                            savedMessage = 10;
                            renderNextAndContext();
                            redirect();
                        }
                    }
                };
            }
            if (action.rfind(".delete:") == 0) {
                int16_t id = std::stoi(action.substr(8));
                func = [this, id](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        int idAndBank = getIdWithBank(id);
                        pluginSerialize->data(deleteVariationDataId, (void*)&idAndBank);
                        renderNextAndContext();
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

        /// Add extra index base on the current index and bank
        addIndex = config.value("addIndex", addIndex);

        /// The color of the text
        playColor = draw.getColor(config["playColor"], playColor); //eg: "#ffffff"

        /// The color of the text
        playNextColor = draw.getColor(config["playNextColor"], playNextColor); //eg: "#ffffff"

        /// The color of the text
        playingClipBgColor = draw.getColor(config["color"], playingClipBgColor); //eg: "#ffffff"
        clipBgColor = alpha(playingClipBgColor, 0.5); //eg: "#ffffff"

        /// The number of visible clips
        visibleCount = config.value("visibleCount", visibleCount); //eg: 10

        /// Size of bank
        bankSize = config.value("bankSize", bankSize);

        /// Context id to trigger re-rendering if multiple instance of this component on the same view. (must be different to 0)
        renderContextId = config.value("renderContextId", renderContextId);

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
        variationExistsDataId = pluginSerialize->getDataId("VARIATION_EXISTS");
        int nextVariation = -1;
        nextVariationToPlay = (int*)pluginSerialize->data(loadVariationNextDataId, &nextVariation);

        // Allow .toggle action to reload selected variation
        if (config.contains("allowToggleReload")) {
            allowToggleReload = config["allowToggleReload"];
        }

        /// The view to redirect once variation saved or selected.
        if (config.contains("redirectView")) {
            redirectView = config["redirectView"].get<std::string>(); //eg: "view_name"
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
            if (savedMessage > 0) {
                savedMessage--;
                if (savedMessage == 0) {
                    renderNext();
                }
            }
        };
    }
    bool lastIsPlaying = false;
    int lastPlayingId = -1;

    void resize() override
    {
        clipW = size.w / visibleCount;
    }

    void render()
    {
        draw.filledRect(relativePosition, size, { bgColor });
        int playingId = valVariation ? valVariation->get() : -1;

        Point center = { (int)(clipW * 0.5), (int)((size.h - fontSize - 1) * 0.5) };
        if (bankToggle) {
            for (int i = 0; i < visibleCount; i++) {
                Point pos = { relativePosition.x + i * clipW, relativePosition.y };
                std::string bankItem = std::string(1, 'A' + i + addIndex);
                draw.filledRect({ relativePosition.x + i * clipW, relativePosition.y }, { clipW - 2, size.h }, { bankItem == bank ? playingClipBgColor : clipBgColor });
                draw.textCentered({ pos.x + center.x, pos.y + center.y }, bankItem, fontSize, { textColor, .maxWidth = clipW });
            }
        } else {
            for (int i = 0; i < visibleCount; i++) {
                int id = i + startBankIndex + addIndex;
                Point pos = { relativePosition.x + i * clipW, relativePosition.y };
                draw.filledRect({ relativePosition.x + i * clipW, relativePosition.y }, { clipW - 2, size.h }, { id == playingId ? playingClipBgColor : clipBgColor });
                Color& color = variationExists(id) ? textColor : textMissingColor;
                draw.textCentered({ pos.x + center.x, pos.y + center.y }, bank + std::to_string(i + 1 + addIndex), fontSize, { color, .maxWidth = clipW });

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

        if (savedMessage > 0) {
            int h = fontSize < size.h ? fontSize : size.h;
            draw.filledRect(relativePosition, { 100, h }, { playingClipBgColor });
            draw.textCentered({ (int)(relativePosition.x + 50), relativePosition.y }, "Saved", fontSize, { textColor });
        }
    }

    void onContext(uint8_t index, float value) override
    {
        if (renderContextId > 0 && index == renderContextId) {
            if ((int)value != startBankIndex) {
                bank = std::string(1, 'A' + (int)(value / bankSize));
            }
            startBankIndex = value;
            renderNext();
        }
        Component::onContext(index, value);
    }
};
