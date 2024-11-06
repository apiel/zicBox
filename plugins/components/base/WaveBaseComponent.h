#ifndef _UI_BASE_COMPONENT_WAVE_H_
#define _UI_BASE_COMPONENT_WAVE_H_

#include "../component.h"
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
                draw.line({ i, y1 }, { i, y2 }, colors.wave);
                draw.line({ i, (int)(y1 + graphH * 0.25) }, { i, (int)(y2 - graphH * 0.25) }, colors.waveMiddle);
                draw.line({ i, (int)(y1 + graphH * 0.75) }, { i, (int)(y2 - graphH * 0.75) }, colors.waveOutline);
            } else {
                draw.pixel({ i, yCenter }, colors.waveOutline);
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
