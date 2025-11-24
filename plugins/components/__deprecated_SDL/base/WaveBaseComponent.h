/** Description:
This header file defines a specialized user interface element called the `WaveBaseComponent`. Its primary purpose is to visualize numerical data as a classic waveform, similar to what you might see in an audio editor or music player.

### How It Works

This component acts as a canvas for drawing wave shapes based on raw numerical inputs, referred to as "samples."

1.  **Data Input:** The component accepts a stream of raw numbers representing the shape of the wave.
2.  **Mapping:** When drawing, it calculates which input sample corresponds to each vertical column of pixels across the component's width. It determines the height of the wave at that precise location.
3.  **Visualization:** To render the waveform, the component draws three overlapping vertical lines at each pixel column. These lines use three different shades derived from a single primary color (a dark core, a middle tone, and a bright outline). This layered approach creates a rich, three-dimensional or gradient appearance instead of a simple flat line.

### Key Features

*   **Waveform Rendering:** Efficiently translates thousands of data points into a constrained visual display.
*   **Visual Depth:** Automatically generates a color scheme with multiple shades from a single input color, adding visual complexity and depth to the resulting wave.
*   **Customization:** The color of the wave can be easily changed both through direct programming calls or via a simplified string configuration system.
*   **Base Component:** It inherits capabilities from a standard UI element, allowing it to be positioned, resized, and styled within a larger interface framework.

sha: 04f01389a4854d4ccd3407fd3d8c626101af7e67962553928c7503157c8ee353 
*/
#ifndef _UI_BASE_COMPONENT_WAVE_H_
#define _UI_BASE_COMPONENT_WAVE_H_

#include "../../component.h"
#include "../../utils/color.h"
#include <string>

class WaveBaseComponent : public Component {
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
    WaveBaseComponent(ComponentInterface::Props props)
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

    // Here we have 3 shades of colors.
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

    bool config(char* key, char* value)
    {
        if (strcmp(key, "WAVE_COLOR") == 0) {
            setColors(draw.getColor(value));
            return true;
        }

        return false;
    }
};

#endif
