#ifndef _UI_PIXEL_COMPONENT_CLIPS_H_
#define _UI_PIXEL_COMPONENT_CLIPS_H_

#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <string>

/*md
## Clips

Clips components to draw a rectangle.
*/

class ClipsComponent : public Component {
protected:
    bool isActive = true;

    Color bgColor;
    Color foreground;
    Color textColor;
    Color foreground2;
    Color barColor;

    ValueInterface* valVariation = NULL;

    int clipH = 17;

    uint8_t selectionBank = 30;

    KeypadLayout keypadLayout;

public:
    ClipsComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , foreground({ 0x40, 0x40, 0x40 })
        , foreground2(lighten(foreground, 0.5))
        , textColor(styles.colors.text)
        , barColor(styles.colors.primary)
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            if (action == ".up") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (view->contextVar[selectionBank] > 0) {
                            setContext(selectionBank, view->contextVar[selectionBank] - 1);
                        }
                        renderNext();
                    }
                };
            }
            if (action == ".down") {
                func = [this](KeypadLayout::KeyMap& keymap) {
                    if (KeypadLayout::isReleased(keymap)) {
                        if (view->contextVar[selectionBank] < valVariation->props().max - 1) {
                            setContext(selectionBank, view->contextVar[selectionBank] + 1);
                        }
                        renderNext();
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
                int playingId = rand() % 12;
                int count = valVariation->props().max;
                for (int i = 0; i < count; i++) {
                    int y = relativePosition.y + i * clipH;

                    if (i == playingId) {
                        draw.filledRect({ relativePosition.x, y }, { size.w, clipH - 1 }, { darken(barColor, 0.8) });
                        draw.filledRect({ relativePosition.x, y }, { size.w, 2 }, { barColor });
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

                    if (rand() % 2 || i == playingId) {
                        draw.textCentered({ relativePosition.x + (int)(size.w * 0.5), y + (int)((clipH - 8) * 0.5) }, std::to_string(i + 1), 8, { textColor, .maxWidth = size.w });
                    }

                    if (isActive && i == view->contextVar[selectionBank]) {
                        draw.rect({ relativePosition.x, y }, { size.w, clipH - 2 }, { barColor });
                    }
                }
            }
        }
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now) override
    {
        if (isActive) {
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
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        if (keypadLayout.config(key, value)) {
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
            AudioPlugin* plugin = &getPlugin(value, track);
            valVariation = watch(plugin->getValue("VARIATION"));
            return true;
        }

        return false;
    }
};

#endif
