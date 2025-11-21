#pragma once

#include "helpers/clamp.h"
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
    ValueInterface* valClip = NULL;
    uint8_t loadClipDataId = -1;
    uint8_t loadClipNextDataId = -1;
    uint8_t saveClipDataId = -1;
    uint8_t deleteClipDataId = -1;
    uint8_t clipExistsDataId = -1;
    AudioPlugin* pluginSeq = NULL;
    ValueInterface* valSeqStatus = NULL;
    bool* isPlaying = NULL;
    int* nextClipToPlay = NULL;

    int clipW = 0;
    int visibleCount = 10;
    int startBankIndex = 1;
    int addIndex = 0;
    std::string bank = "A";
    int bankSize = 100;
    std::string redirectView = "";

    uint8_t renderContextId = 0;

    bool allowToggleReload = false;

    bool clipExists(int id) { return pluginSerialize->data(clipExistsDataId, &id) != NULL; }

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
            if (!clipExists(idAndBank)) {
                showPopupMessage("Empty");
                renderNext();
                return;
            }
            if ((int16_t)valClip->get() == idAndBank) {
                if (!*isPlaying) {
                    if (allowToggleReload) {
                        pluginSerialize->data(loadClipDataId, (void*)&idAndBank);
                        redirect();
                    } // else do nothing
                } else if (valSeqStatus->get() == 1) {
                    valSeqStatus->set(0); // Mute
                } else if (valSeqStatus->get() == 0) {
                    valSeqStatus->set(2); // Play next
                } else {
                    valSeqStatus->set(1); // Play right away
                }
                // Ultimately we could check if sequencer is playing so it only start next if it is...
                // however, to do this, we would have to track another data id again: IS_PLAYING
                // for the moment let's stick to double press, it is fine...
            } else if (nextClipToPlay != NULL && *nextClipToPlay == -1) {
                if (!*isPlaying) {
                    pluginSerialize->data(loadClipDataId, (void*)&idAndBank);
                    redirect();
                } else if (valSeqStatus->get() == 1) {
                    pluginSerialize->data(loadClipNextDataId, (void*)&idAndBank);
                }
            } else {
                pluginSerialize->data(loadClipDataId, (void*)&idAndBank);
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
    int popupMessage = 0;
    std::string popupMessageText = "";

    void showPopupMessage(const std::string& text, int duration = 10) {
        popupMessage = duration;
        popupMessageText = text;
    }

public:
    ClipsComponent(ComponentInterface::Props props)
        : Component(props, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".reload") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        int16_t id = (int16_t)valClip->get();
                        pluginSerialize->data(loadClipDataId, (void*)&id);
                        // redirect();
                        showPopupMessage("Reloaded");
                        renderNext();
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
                            pluginSerialize->data(saveClipDataId, (void*)&idAndBank);
                            // valClip->set(id);
                            // popupMessage = 10;
                            showPopupMessage("Saved");
                            renderNextAndContext();
                        }
                    }
                };
            }
            if (action.rfind(".delete:") == 0) {
                int16_t id = std::stoi(action.substr(8));
                func = [this, id](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        int idAndBank = getIdWithBank(id);
                        pluginSerialize->data(deleteClipDataId, (void*)&idAndBank);
                        renderNextAndContext();
                    }
                };
            }
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
            if (action.rfind(".message:") == 0) {
                std::string text = action.substr(9);
                func = [this, text](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        showPopupMessage(text);
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

        valClip = watch(pluginSerialize->getValue("CLIP"));
        saveClipDataId = pluginSerialize->getDataId("SAVE_CLIP");
        deleteClipDataId = pluginSerialize->getDataId("DELETE_CLIP");
        loadClipDataId = pluginSerialize->getDataId("LOAD_CLIP");
        loadClipNextDataId = pluginSerialize->getDataId("LOAD_CLIP_NEXT");
        clipExistsDataId = pluginSerialize->getDataId("CLIP_EXISTS");
        int nextClip = -1;
        nextClipToPlay = (int*)pluginSerialize->data(loadClipNextDataId, &nextClip);

        // Allow .toggle action to reload selected clip
        if (config.contains("allowToggleReload")) {
            allowToggleReload = config["allowToggleReload"];
        }

        /// The view to redirect once clip saved or selected.
        if (config.contains("redirectView")) {
            redirectView = config["redirectView"].get<std::string>(); //eg: "view_name"
        }

        // std::string controllerId = config.value("controller", "Default");
        // controller = getController(controllerId.c_str());

        /*md md_config_end */

        resize();

        jobRendering = [this](unsigned long now) {
            if (lastIsPlaying != *isPlaying || (*isPlaying && lastPlayingId != valClip->get())) {
                lastIsPlaying = *isPlaying;
                lastPlayingId = valClip->get();
                renderNext();
            }
            if (popupMessage > 0) {
                popupMessage--;
                if (popupMessage == 0) {
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
        int playingId = valClip ? valClip->get() : -1;

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
                int id = i + 1 + startBankIndex + addIndex;
                Point pos = { relativePosition.x + i * clipW, relativePosition.y };
                draw.filledRect({ relativePosition.x + i * clipW, relativePosition.y }, { clipW - 2, size.h }, { id == playingId ? playingClipBgColor : clipBgColor });
                Color& color = clipExists(id) ? textColor : textMissingColor;
                draw.textCentered({ pos.x + center.x, pos.y + center.y }, bank + std::to_string(i + 1 + addIndex), fontSize, { color, .maxWidth = clipW });

                if (*isPlaying) {
                    if (id == *nextClipToPlay) {
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

        if (popupMessage > 0) {
            // int h = fontSize < size.h ? fontSize : size.h;
            int h = fontSize + 2;
            if (h > size.h) {
                h = size.h;
            }
            draw.filledRect(relativePosition, { 100, h }, { playingClipBgColor });
            draw.rect(relativePosition, { 100, h }, { textColor });
            draw.textCentered({ (int)(relativePosition.x + 50), relativePosition.y }, popupMessageText, fontSize, { textColor });
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
