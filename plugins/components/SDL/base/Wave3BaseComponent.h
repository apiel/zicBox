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
