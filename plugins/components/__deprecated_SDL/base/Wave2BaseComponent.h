/** Description:
This component acts as a specialized building block for a User Interface (UI), designed specifically to visualize fluctuating numerical information, much like an audio waveform or a graphic equalizer display.

**What it does:**
This element takes a stream of raw measurements (called "samples") and transforms them into a visual "wave" drawn on the screen. For every measurement in the stream, it calculates and draws a vertical line. The height of that line directly represents the intensity or value of the measurement at that specific point in time.

**Key Features:**
1.  **Data Handling:** It stores the input data and its size, allowing the component to be updated and redrawn dynamically.
2.  **Visualization:** The wave is drawn using two distinct color shades: an inner color (the main body of the wave) and a slightly different outline color (for clarity). This gives the visualization depth and makes it easier to read.
3.  **Automatic Styling:** By default, if you provide a primary color, the component automatically generates the secondary outline color for a harmonious look.
4.  **Configuration:** Users can easily customize the appearance by setting specific properties like the primary wave color or the outline color using simple configuration settings.
5.  **Scaling:** It automatically handles the necessary math to ensure the input data is correctly scaled and centered within the component's defined height, simplifying the display of complex, dynamic information.

sha: b39b2a569612f043820b9020b71b028d1c3dd712490ac149ffe2dd2afe132f04 
*/
#ifndef _UI_BASE_COMPONENT_WAVE2_H_
#define _UI_BASE_COMPONENT_WAVE2_H_

#include "../../component.h"
#include "../../utils/color.h"
#include <string>

class Wave2BaseComponent : public Component {
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
    Wave2BaseComponent(ComponentInterface::Props props)
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
    // We draw based on the samples in the buffer, meaning that we draw all samples.
    // Doing this, might sometime be a bit messy as we draw a lot of information...
    void render(float* buffer, uint64_t count)
    {
        for (int i = 0; i < count; i++) {
            int x = i * size.w / count;
            int graphH = buffer[i] * lineHeight;
            if (graphH) {
                int y1 = yCenter - graphH;
                int y2 = yCenter + graphH;
                draw.line({ x, y1 }, { x, y2 }, { colors.wave });
                draw.line({ x, (int)(y1 + graphH * 0.5) }, { x, (int)(y2 - graphH * 0.5) }, { colors.waveOutline });
            } else {
                draw.pixel({ x, yCenter }, { colors.waveOutline });
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
