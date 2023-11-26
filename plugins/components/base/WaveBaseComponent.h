#ifndef _UI_BASE_COMPONENT_WAVE_H_
#define _UI_BASE_COMPONENT_WAVE_H_

#include "../../../helpers/inRect.h"
#include "../component.h"
#include <string>

class WaveBaseComponent : public Component {
protected:
    uint64_t samplesCount = 0;
    float* bufferSamples = NULL;

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
    WaveBaseComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
    {
    }

    void set(float* buffer, uint64_t count)
    {
        bufferSamples = buffer;
        samplesCount = count;
    }

    void render(float* buffer, uint64_t count)
    {
        int h = size.h * 0.5f;
        for (int i = 0; i < count; i++) {
            int x = i * size.w / count;
            int graphH = buffer[i] * h;
            if (graphH) {
                int y1 = position.y + h - graphH;
                int y2 = position.y + h + graphH;
                draw.line({ x, y1 }, { x, y2 }, colors.wave);
                draw.line({ x, (int)(y1 + graphH * 0.5) }, { x, (int)(y2 - graphH * 0.5) }, colors.waveOutline);
            } else {
                draw.pixel({ x, position.y + h }, colors.waveOutline);
            }
        }
    }

    void render()
    {
        render(bufferSamples, samplesCount);
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "WAVE_COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
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
