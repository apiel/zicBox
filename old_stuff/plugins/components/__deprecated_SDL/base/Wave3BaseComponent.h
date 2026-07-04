/** Description:
This header file defines a reusable software building block, called `Wave3BaseComponent`, specifically designed to display a dynamic data stream as a visual wave, similar to an audio waveform or a detailed sensor reading.

**Core Functionality:**

This component inherits properties from a standard UI element, allowing it to be positioned and sized within a larger screen layout. Its main job is managing and interpreting raw numerical data (the "samples") to draw a graph.

**How the Visualization Works:**

When drawing, the component takes the entire set of input data and intelligently scales it to fit the component's pixel width. For every pixel column on the screen, it determines the corresponding data value. It then draws a vertical line at that position, centered vertically.

A key feature is its dual-color design. The component automatically uses two shades derived from a primary color: a darker shade for the main body of the wave and a lighter shade for the outline. By drawing these two shades together, it creates a visual effect that gives the waveform depth and clarity.

**Customization and Setup:**

The component provides functions to load the new data samples whenever the underlying information changes. Additionally, users can extensively customize the appearance. It accepts configuration commands (like setting key-value pairs) to instantly change the colors of the wave, allowing the main body, the outline, or both to be adjusted independently.

sha: 3b50583b04708b7837ef09ec58cf2569914305745c1da5a9d8ccab6b80b6e471 
*/
#ifndef _UI_BASE_COMPONENT_WAVE3_H_
#define _UI_BASE_COMPONENT_WAVE3_H_

#include "../../component.h"
#include "../../utils/color.h"
#include <string>

class Wave3BaseComponent : public Component {
protected:
    uint64_t samplesCount = 0;
    float* bufferSamples = NULL;
    int lineHeight;
    int yCenter;

    struct Colors {
        Color wave;
        Color waveOutline;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({
            darken(color, 0.4),
            color,
        });
    }

public:
    Wave3BaseComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.primary))
    {
        lineHeight = props.size.h * 0.5f;
        yCenter = position.y + lineHeight;
    }

    void set(float* buffer, uint64_t count)
    {
        bufferSamples = buffer;
        samplesCount = count;
    }

    // Here we have 2 shades of colors.
    // We draw based on the pixel width picking the sample relative to the index.
    // Doing this, we are jumping over some sample and miss some of them.
    void render(float* buffer, uint64_t count)
    {
        for (int i = 0; i < size.w; i++) {
            int index = i * count / size.w;
            int graphH = buffer[index] * lineHeight;
            if (graphH) {
                int y1 = yCenter - graphH;
                int y2 = yCenter + graphH;
                draw.line({ i, y1 }, { i, y2 }, { colors.wave });
                draw.line({ i, (int)(y1 + graphH * 0.5) }, { i, (int)(y2 - graphH * 0.5) }, { colors.waveOutline });
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

    bool config(char* key, char* value)
    {
        if (strcmp(key, "WAVE_COLOR") == 0) {
            setColors(draw.getColor(value));
            return true;
        }

        if (strcmp(key, "WAVE_OUTLINE_COLOR") == 0) {
            colors.waveOutline = draw.getColor(value);
            return true;
        }

        if (strcmp(key, "WAVE_INLINE_COLOR") == 0) {
            colors.wave = draw.getColor(value);
            return true;
        }

        return false;
    }
};

#endif
