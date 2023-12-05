#ifndef _UI_BASE_COMPONENT_WAVE2_H_
#define _UI_BASE_COMPONENT_WAVE2_H_

#include "../component.h"
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
            draw.darken(color, 0.4),
            color,
        });
    }

public:
    Wave2BaseComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
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
                draw.line({ x, y1 }, { x, y2 }, colors.wave);
                draw.line({ x, (int)(y1 + graphH * 0.5) }, { x, (int)(y2 - graphH * 0.5) }, colors.waveOutline);
            } else {
                draw.pixel({ x, yCenter }, colors.waveOutline);
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
