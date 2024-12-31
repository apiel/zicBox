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

    ValueInterface* valVariation = NULL;

    int clipH = 16;

public:
    ClipsComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , foreground({ 0x40, 0x40, 0x40 })
        , textColor({ 0x80, 0x80, 0x80 })
    {
    }
    void render()
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { bgColor });
            if (valVariation) {
                int count = valVariation->props().max;
                for (int i = 0; i < count; i++) {
                    int y = relativePosition.y + i * clipH;
                    draw.filledRect({ relativePosition.x, y }, { size.w, clipH - 1 }, { foreground });
                    for (int xx = 0; xx < size.w; xx += 4) {
                        for (int yy = 0; yy < clipH - 4; yy += 4) {
                            // // get rand value between 0.0 and 1.0
                            // float v = rand() / (float)RAND_MAX;
                            // draw.filledRect({ relativePosition.x + xx, y + yy }, {4, 4}, { lighten(foreground, v * 0.5) });

                            int c = rand() % 2;
                            Color color = c == 0 ? foreground : lighten(foreground, 0.2);
                            draw.filledRect({ relativePosition.x + xx, y + yy }, { 4, 4 }, { color });
                        }
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
