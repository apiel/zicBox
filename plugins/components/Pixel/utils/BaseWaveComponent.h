/** Description:
This header file defines a specialized graphical building block called the "Base Wave Component." Its primary function is to take raw numerical data, typically representing an audio signal or similar time-based information (the "waveform"), and visually draw it onto a user interface.

**How the Component Works:**

1.  **Data Input:** The component is designed to receive a stream of numerical "samples" (like the height measurements of a wave).
2.  **Visualization Process:** When the wave needs to be drawn, the component maps the long sequence of input samples onto the limited width of the screen area. For every pixel column across its width, it selects the appropriate sample value and translates that number into a vertical line height.
3.  **Centered Drawing:** All vertical lines are drawn centered around a middle horizontal axis of the component, showing positive sample values extending up and negative values extending down.
4.  **Layered Appearance:** To give the waveform a defined look and depth, the component uses three automatically generated shades of color (a light shade, a middle shade, and a darker outline shade) derived from a single primary color. It draws three nested lines at every point, creating a visually layered effect.
5.  **Customization:** The component allows external controls to easily feed it new sample data for real-time updates and to change the entire color scheme of the wave representation.

In essence, this component acts as a high-performance translator, converting complex numerical data into a simple, dynamic, and layered visual wave display.

sha: e85718bcc72ef3d9f81bb3805c49a052410c3bdcf806b5785af81a6f56c69bec 
*/
#ifndef _UI_PIXEL_BASE_COMPONENT_WAVE_H_
#define _UI_PIXEL_BASE_COMPONENT_WAVE_H_

#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <string>

class BaseWaveComponent : public Component {
protected:
    uint64_t samplesCount = 0;
    float* bufferSamples = NULL;
    int lineHeight;
    int yCenter;

    struct Colors {
        Color wave;
        Color waveMiddle;
        Color waveOutline;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({
            darken(color, 0.4),
            darken(color, 0.2),
            color,
        });
    }

public:
    BaseWaveComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.primary))
    {
        lineHeight = props.size.h * 0.5f;
    }

    void set(float* buffer, uint64_t count)
    {
        bufferSamples = buffer;
        samplesCount = count;
    }

    // Here we have 3 shades of colors.
    // We draw based on the pixel width picking the sample relative to the index.
    // Doing this, we are jumping over some sample and miss some of them.
    void render(float* buffer, uint64_t count)
    {
        if (!buffer || count <= 0 || size.w <= 0) {
            return; // avoid segfaults
        }

        yCenter = relativePosition.y + lineHeight;

        for (int i = 0; i < size.w; i++) {
            int index = static_cast<int>((i * count) / size.w);
            // if (index >= static_cast<int>(count)) {
            //     continue;
            // }

            int graphH = static_cast<int>(buffer[index] * lineHeight);
            if (graphH) {
                int y1 = yCenter - graphH;
                int y2 = yCenter + graphH;
                draw.line({ i, y1 }, { i, y2 }, { colors.wave });
                draw.line({ i, (int)(y1 + graphH * 0.25) }, { i, (int)(y2 - graphH * 0.25) }, { colors.waveMiddle });
                draw.line({ i, (int)(y1 + graphH * 0.75) }, { i, (int)(y2 - graphH * 0.75) }, { colors.waveOutline });
            } else {
                draw.pixel({ i, yCenter }, { colors.waveOutline });
            }
        }
    }

    void render()
    {
        render(bufferSamples, samplesCount);
    }

    void setColors(Color color)
    {
        colors = getColorsFromColor(color);
    }

    // bool config(char* key, char* value)
    // {
    //     if (strcmp(key, "WAVE_COLOR") == 0) {
    //         setColors(draw.getColor(value));
    //         return true;
    //     }

    //     return false;
    // }
};

#endif
