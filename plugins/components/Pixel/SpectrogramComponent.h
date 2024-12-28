#ifndef _UI_PIXEL_COMPONENT_SPECTROGRAM_H_
#define _UI_PIXEL_COMPONENT_SPECTROGRAM_H_

#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <cmath>
#include <string>
#include <vector>

/*md
## Spectrogram

Spectrogram components to display audio spectrogram.
*/

class SpectrogramComponent : public Component {
protected:
    Color bgColor;
    Color waveOut;
    Color waveIn;

    float *buffer;

public:
    SpectrogramComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , waveOut(styles.colors.primary)
        , waveIn(darken(styles.colors.primary, 0.5))
    {
    }
    void render()
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { bgColor });
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

        if (strcmp(key, "DATA") == 0) {
            AudioPlugin* plugin = NULL;
            plugin = &getPlugin(strtok(value, " "), track);

            uint8_t dataId = plugin->getDataId(strtok(NULL, " "));
            buffer = (float*)plugin->data(dataId);
            return true;
        }

        return false;
    }
};

#endif
