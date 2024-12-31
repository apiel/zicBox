#ifndef _UI_PIXEL_COMPONENT_CLIPS_H_
#define _UI_PIXEL_COMPONENT_CLIPS_H_

#include "plugins/components/component.h"

#include <string>

/*md
## Clips

Clips components to draw a rectangle.
*/

class ClipsComponent : public Component {
protected:
    Color bgColor;
    Color foreground;

    ValueInterface* valVariation = NULL;

    int clipH = 16;

public:
    ClipsComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , foreground({ 0x40, 0x40, 0x40 })
    {
    }
    void render()
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { bgColor });
            if (valVariation) {
                int count = valVariation->props().max;
                for (int i = 0; i < count; i++) {
                    draw.filledRect({ relativePosition.x, relativePosition.y + i * clipH }, { size.w, clipH - 1 }, { foreground });
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
