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
    Color waveMiddle;
    Color waveIn;

    Color textColor;
    std::string text;

    float* buffer;

    bool mirror = true;

public:
    SpectrogramComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , waveIn(styles.colors.primary)
        , waveMiddle(darken(styles.colors.primary, 0.2))
        , waveOut(darken(styles.colors.primary, 0.4))
        , textColor(lighten(styles.colors.background, 0.9))
    {
        jobRendering = [this](unsigned long now) {
            renderNext();
        };
    }
    void render()
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { bgColor });
            int yCenter = relativePosition.y + size.h / 2;

            if (!text.empty()) {
                draw.textCentered({ relativePosition.x + (int)(size.w / 2), relativePosition.y }, text, 16, { textColor });
            }

            if (mirror) {
                for (int i = 0; i < size.w; i++) {
                    int graphH = buffer[i] * size.h;
                    if (graphH) {
                        int y1 = yCenter - graphH;
                        int y2 = yCenter + graphH;
                        draw.line({ i, y1 }, { i, y2 }, { waveIn });
                        draw.line({ i, (int)(y1 + graphH * 0.25) }, { i, (int)(y2 - graphH * 0.25) }, { waveMiddle });
                        draw.line({ i, (int)(y1 + graphH * 0.75) }, { i, (int)(y2 - graphH * 0.75) }, { waveOut });
                    } else {
                        draw.pixel({ i, yCenter }, { waveOut });
                    }
                }
            } else {
                for (int i = 0; i < size.w; i++) {
                    int graphH = buffer[i] * size.h;
                    if (graphH) {
                        draw.line({ i, yCenter }, { i, yCenter + graphH }, { waveIn });
                        draw.line({ i, yCenter }, { i, (int)(yCenter + graphH * 0.25) }, { waveMiddle });
                        draw.line({ i, yCenter }, { i, (int)(yCenter + graphH * 0.75) }, { waveOut });
                    } else {
                        draw.pixel({ i, yCenter }, { waveOut });
                    }
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

        /*md - `WAVE_COLOR_OUT: color` is the color of the wave. */
        if (strcmp(key, "WAVE_COLOR_OUT") == 0) {
            waveOut = draw.getColor(value);
            return true;
        }

        /*md - `WAVE_COLOR_MIDDLE: color` is the color of the wave. */
        if (strcmp(key, "WAVE_COLOR_MIDDLE") == 0) {
            waveMiddle = draw.getColor(value);
            return true;
        }

        /*md - `WAVE_COLOR_IN: color` is the color of the wave. */
        if (strcmp(key, "WAVE_COLOR_IN") == 0) {
            waveIn = draw.getColor(value);
            return true;
        }

        /*md - `TEXT_COLOR: color` is the color of the text. */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            textColor = draw.getColor(value);
            return true;
        }

        /*md - `TEXT: text` is the text to display. */
        if (strcmp(key, "TEXT") == 0) {
            text = value;
            return true;
        }

        /*md - `MIRROR: false` mirror the waveform horizontally (default: true). */
        if (strcmp(key, "MIRROR") == 0) {
            mirror = strcmp(value, "true") == 0;
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
