/** Description:
This header defines a specialized graphical user interface element called the `ClipsComponent`. Its main purpose is to provide a visual list and interactive control panel for managing audio "clips" or short data segments within a larger application, typically an audio sequencer or sampler.

**How it Works:**

1.  **Visual Display:** The component draws a scrolling rectangular list showing available clip slots. It uses different colors to provide immediate feedback to the user, such as a dedicated color for the background, a highlight color for the currently selected clip, and specific colors to indicate which clip is currently playing or queued up to play next.
2.  **Data Linkage:** It acts as an interface layer, connecting to dedicated audio software modules (referred to as "plugins"). It communicates with a "Sequencer Plugin" to determine the global playback status (is the music playing?) and a "Serializer Plugin" which handles the low-level tasks of saving, loading, deleting, and checking if data exists in a specific clip slot.
3.  **User Interaction:** The component processes user inputs, primarily from directional inputs (like an encoder or up/down keys) and action keys.
    *   Scrolling inputs change which clip is highlighted.
    *   Action inputs trigger commands: A "toggle" command might start playing the selected clip or queue it for the next sequence. Separate commands handle saving the current audio data into the selected clip slot or permanently deleting an existing clip.

In essence, the `ClipsComponent` simplifies clip management by presenting a navigable list and translating user actions into specific data commands handled by the specialized audio engine plugins.

sha: 27d84e115d23c622a8a0e6d576171bd4bfb4c8ec2dc563cb6ce6a2dbc770e0ef 
*/
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
    Color foreground;
    Color textColor;
    Color barColor;
    Color playColor;
    Color playNextColor;
    Color selectionColor;

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
    bool showSelectionRect = false;

    int encoderId = -1;

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

    bool clipExists(int id) { return pluginSerialize->data(clipExistsDataId, &id) != NULL; }

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
                        if (shouldDoAction() && view->contextVar[selectionBank] < valClip->props().max - 1) {
                            setContext(selectionBank, view->contextVar[selectionBank] + 1);
                        }
                        renderNext();
                    }
                };
            }
            if (action == ".toggle") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        int16_t id = view->contextVar[selectionBank];
                        if (clipExists(id)) {
                            if ((int16_t)valClip->get() == id) {
                                if (!*isPlaying) {
                                    pluginSerialize->data(loadClipDataId, &id);
                                } else if (valSeqStatus->get() == 1) {
                                    valSeqStatus->set(0);
                                } else {
                                    valSeqStatus->set(1);
                                }
                                // Ultimately we could check if sequencer is playing so it only start next if it is...
                                // however, to do this, we would have to track another data id again: IS_PLAYING
                                // for the moment let's stick to double press, it is fine...
                            } else if (nextClipToPlay != NULL && *nextClipToPlay == -1) {
                                if (!*isPlaying) {
                                    pluginSerialize->data(loadClipDataId, &id);
                                } else if (valSeqStatus->get() == 1) {
                                    pluginSerialize->data(loadClipNextDataId, &id);
                                }
                            } else {
                                pluginSerialize->data(loadClipDataId, &id);
                            }
                            renderNext();
                        }
                    }
                };
            }
            if (action == ".save") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        int16_t id = view->contextVar[selectionBank];
                        if (pluginSerialize) {
                            pluginSerialize->data(saveClipDataId, (void*)&id);
                            // valClip->set(id);
                            renderNext();
                        }
                    }
                };
            }
            if (action == ".delete") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        int16_t id = view->contextVar[selectionBank];
                        if (clipExists(id)) {
                            pluginSerialize->data(deleteClipDataId, (void*)&id);
                            renderNext();
                        }
                    }
                };
            }
            if (action == ".saveOrDelete") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        int16_t id = view->contextVar[selectionBank];
                        if (clipExists(id)) {
                            pluginSerialize->data(deleteClipDataId, (void*)&id);
                            renderNext();
                        } else {
                            pluginSerialize->data(saveClipDataId, (void*)&id);
                            // valClip->set(id);
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

        /// Id of the encoder to use in order to change selected row.
        encoderId = config.value("encoderId", encoderId); //eg: 0

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

        valClip = watch(pluginSerialize->getValue("CLIP"));
        saveClipDataId = pluginSerialize->getDataId("SAVE_CLIP");
        deleteClipDataId = pluginSerialize->getDataId("DELETE_CLIP");
        loadClipDataId = pluginSerialize->getDataId("LOAD_CLIP");
        loadClipNextDataId = pluginSerialize->getDataId("LOAD_CLIP_NEXT");
        clipExistsDataId = pluginSerialize->getDataId("CLIP_EXISTS");
        int nextClip = -1;
        nextClipToPlay = (int*)pluginSerialize->data(loadClipNextDataId, &nextClip);

        /*md md_config_end */
    }

    void render()
    {
        draw.filledRect(relativePosition, size, { bgColor });
        if (valClip) {
            int playingId = valClip->get();
            int selection = view->contextVar[selectionBank];
            int start = selection >= visibleCount ? selection - visibleCount + 1 : 0;
            for (int i = start, v = 0; i < visibleCount && v < visibleCount; i++, v++) {
                // int y = relativePosition.y + i * clipH;
                int y = relativePosition.y + (i - start) * clipH;

                bool selected = i == selection;
                draw.filledRect({ relativePosition.x, y }, { size.w, clipH - 1 }, { selected ? selectionColor : foreground });

                bool exists = clipExists(i);
                if (exists && (i == playingId || i == *nextClipToPlay)) {
                    // draw.filledRect({ relativePosition.x, y }, { size.w, clipH - 1 }, { darken(barColor, 0.8) });
                    draw.filledRect({ relativePosition.x, y }, { size.w, 2 }, { barColor });

                    if (i == *nextClipToPlay) {
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

                if (exists) {
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
        return Component::onKey(id, key, state, now);
    }

    void onContext(uint8_t index, float value) override
    {
        // printf("[ClipsComponent] onContext: %d %f\n", index, value);
        if (index == selectionBank && shouldDoAction()) {
            // printf("[ClipsComponent] renderNext\n");
            renderNext();
        }
    }

    void onEncoder(int8_t id, int8_t direction) override
    {
        if (id == encoderId) {
            if (shouldDoAction()) {
                setContext(selectionBank, CLAMP(view->contextVar[selectionBank] + direction, 0, valClip->props().max - 1));
            }
        }
    }
};
