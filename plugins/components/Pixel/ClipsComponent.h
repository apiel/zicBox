#ifndef _UI_PIXEL_COMPONENT_CLIPS_H_
#define _UI_PIXEL_COMPONENT_CLIPS_H_

#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <string>

/*md
## Clips

Clips components to draw a rectangle.
*/

class ClipsComponent : public Component {
protected:
    Color bgColor;
    Color foreground;
    Color textColor;
    Color foreground2;
    Color barColor;

    ValueInterface* valVariation = NULL;

    int clipH = 17;

public:
    ClipsComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , foreground({ 0x40, 0x40, 0x40 })
        , foreground2(lighten(foreground, 0.5))
        , textColor(styles.colors.text)
        , barColor(styles.colors.primary)
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
                        draw.filledRect({ relativePosition.x, y }, { size.w, clipH - 1 }, { darken(barColor, 0.7) });
                        draw.filledRect({ relativePosition.x, y }, { size.w, 2 }, { barColor });
                    } else {
                        draw.filledRect({ relativePosition.x, y }, { size.w, clipH - 1 }, { foreground });
                        Color fg2 = i == playingId ? darken(barColor, 0.7) : foreground2;
                        for (int xx = 0; xx < size.w; xx += 4) {
                            for (int yy = 0; yy < clipH - 1; yy += 4) {
                                int c = rand() % 2;
                                Color color = c == 0 ? foreground : fg2;
                                draw.filledRect({ relativePosition.x + xx, y + yy }, { 4, 4 }, { color });
                            }
                        }
                        Color color = i == playingId ? barColor : darken(barColor, 0.3);
                        draw.filledRect({ relativePosition.x, y }, { size.w, i == playingId ? 2 : 1 }, { color });
                    }

                    draw.textCentered({ relativePosition.x + (int)(size.w * 0.5), y + (int)((clipH - 8) * 0.5) }, std::to_string(i), 8, { textColor, .maxWidth = size.w });
                }
            }
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
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
