#ifndef _UI_PIXEL_COMPONENT_CLIPS_H_
#define _UI_PIXEL_COMPONENT_CLIPS_H_

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
    Color foreground2;
    Color barColor;
    Color playColor;
    Color playNextColor;

    AudioPlugin* pluginSerialize = NULL;
    ValueInterface* valVariation = NULL;
    uint8_t loadVariationDataId = -1;
    uint8_t loadVariationNextDataId = -1;
    uint8_t saveVariationDataId = -1;
    uint8_t deleteVariationDataId = -1;
    ValueInterface* valSeqStatus = NULL;
    int* nextVariationToPlay = NULL;

    struct Variation {
        bool exists;
        // std::string filepath;
    };
    std::vector<Variation> variations;

    int clipH = 17;

    int visibleCount = 16;

    uint8_t selectionBank = 30;

    KeypadLayout keypadLayout;

    bool shouldDoAction() {
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
        : Component(props)
        , bgColor(styles.colors.background)
        , foreground(rgb(53, 53, 53))
        , foreground2(lighten(foreground, 0.5))
        , textColor(styles.colors.text)
        , barColor(styles.colors.primary)
        , playColor(rgb(35, 161, 35))
        , playNextColor(rgb(253, 111, 14))
        , keypadLayout(this, [&](std::string action) {
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
            // if (action == ".next") {
            //     func = [this](KeypadLayout::KeyMap& keymap) {
            //         if (KeypadLayout::isReleased(keymap)) {
            //             int16_t id = view->contextVar[selectionBank];
            //             if (variations[id].exists) {
            //                 if (valVariation->get() == id) {
            //                     if (valSeqStatus->get() == 0) {
            //                         valSeqStatus->set(2);
            //                     }
            //                 } else {
            //                     // TODO Would need to set variation only at the next round...
            //                     // valVariation->set(id);
            //                 }
            //                 renderNext();
            //             }
            //         }
            //     };
            // }
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
    {
    }

    void render()
    {
        if (updatePosition()) {
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

                    if (variation.exists && i == playingId) {
                        draw.filledRect({ relativePosition.x, y }, { size.w, clipH - 1 }, { darken(barColor, 0.8) });
                        draw.filledRect({ relativePosition.x, y }, { size.w, 2 }, { barColor });

                        if (valSeqStatus) {
                            if (valSeqStatus->get() == 1) {
                                draw.filledRect({ relativePosition.x + size.w - 5, y + 3 }, { 3, 3 }, { playColor });
                            } else if (valSeqStatus->get() == 2) {
                                draw.filledRect({ relativePosition.x + size.w - 5, y + 3 }, { 3, 3 }, { playNextColor });
                            }
                        }
                    } else {
                        draw.filledRect({ relativePosition.x, y }, { size.w, clipH - 1 }, { foreground });
                        // for (int xx = 0; xx < size.w; xx += 2) {
                        //     if (rand() % 2) {
                        //         int c = rand() % 7;
                        //         draw.filledRect({ relativePosition.x + xx, y + c + 4 }, { 2, 1 }, { foreground2 });
                        //     }
                        // }
                        draw.filledRect({ relativePosition.x, y }, { size.w, 1 }, { darken(barColor, 0.3) });
                    }

                    if (variation.exists) {
                        draw.textCentered({ relativePosition.x + (int)(size.w * 0.5), y + (int)((clipH - 8) * 0.5) }, std::to_string(i + 1), 8, { textColor, .maxWidth = size.w });
                    }

                    if ((isActive || isGroupAll) && i == selection) {
                        draw.rect({ relativePosition.x, y }, { size.w, clipH - 2 }, { barColor });
                    }
                }
            }
        }
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now) override
    {
        if (isActive || isGroupAll) {
            keypadLayout.onKey(id, key, state, now);
        }
    }

    void onGroupChanged(int8_t index) override
    {
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

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        if (keypadLayout.config(key, value)) {
            return true;
        }

        /*md - `GROUP_ALL: group` is the group to change clips for all tracks. */
        if (strcmp(key, "GROUP_ALL") == 0) {
            groupAll = atoi(value);
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(value);
            return true;
        }

        /*md - `FOREGROUND_COLOR: color` is the foreground color of the component. */
        if (strcmp(key, "FOREGROUND_COLOR") == 0) {
            foreground = draw.getColor(value);
            foreground2 = lighten(foreground, 0.2);
            return true;
        }

        /*md - `TEXT_COLOR: color` is the text color of the component. */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            textColor = draw.getColor(value);
            return true;
        }

        /*md - `COLOR: color` is the foreground color of the component. */
        if (strcmp(key, "COLOR") == 0) {
            barColor = draw.getColor(value);
            // textColor = barColor;
            return true;
        }

        /*md - `PLUGIN: plugin_name` set plugin target for serializer */
        if (strcmp(key, "PLUGIN") == 0) {
            pluginSerialize = &getPlugin(value, track);
            valVariation = watch(pluginSerialize->getValue("VARIATION"));
            saveVariationDataId = pluginSerialize->getDataId("SAVE_VARIATION");
            deleteVariationDataId = pluginSerialize->getDataId("DELETE_VARIATION");
            loadVariationDataId = pluginSerialize->getDataId("LOAD_VARIATION");
            loadVariationNextDataId = pluginSerialize->getDataId("LOAD_VARIATION_NEXT");
            int nextVariation = -1;
            nextVariationToPlay = (int *)pluginSerialize->data(loadVariationNextDataId, &nextVariation);

            for (int i = 0; i < valVariation->props().max; i++) {
                bool exists = pluginSerialize->data(pluginSerialize->getDataId("GET_VARIATION"), &i) != NULL;
                variations.push_back({ exists });
            }
            return true;
        }

        /*md - `VISIBLE_COUNT: number` set the number of visible clips. */
        if (strcmp(key, "VISIBLE_COUNT") == 0) {
            visibleCount = atoi(value);
            return true;
        }

        /*md - `SEQ_PLUGIN: plugin_name` set plugin target for sequencer. */
        if (strcmp(key, "SEQ_PLUGIN") == 0) {
            AudioPlugin* pluginSeq = &getPlugin(value, track);
            valSeqStatus = watch(pluginSeq->getValue("STATUS"));
            return true;
        }

        return false;
    }
};

#endif
