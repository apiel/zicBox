#ifndef _UI_COMPONENT_WAVE_H_
#define _UI_COMPONENT_WAVE_H_

#include "component.h"
#include <string>

#include <vector>

class WaveComponent : public Component {
protected:
    AudioPlugin& plugin;

    Size waveSize;
    Point wavePosition;
    void* textureWave = NULL;

    bool showAmp = false;
    bool showFreq = false;

    struct Data {
        float modulation;
        float time;
    };

    struct Colors {
        Color background;
        Color samples;
        Color env;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.75),
            color,
            draw.darken(color, 0.3) });
    }

    const int margin;

    int lastUpdateUi = -1;

public:
    WaveComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
        , margin(styles.margin)
        , plugin(getPlugin("Kick23"))
    {
        waveSize = { size.w - 2 * margin, size.h - 2 * margin };
        wavePosition = { position.x + margin, position.y + margin };

        jobRendering = [this](unsigned long now) {
            int* last = (int*)plugin.data(0);
            if (*last != lastUpdateUi) {
                lastUpdateUi = *last + 0;
                renderNext();
            }
        };
    }

    void render()
    {
        draw.filledRect(wavePosition, size, colors.background);

        uint64_t samplesCount = 1000;
        float* bufferSamples = (float*)plugin.data(1);

        int h = waveSize.h * 0.5f;
        float yRatio = samplesCount / waveSize.w;
        for (int xIndex = 0; xIndex < waveSize.w - 1; xIndex++) {
            int x = wavePosition.x + xIndex;
            int i = xIndex * yRatio;
            int y1 = wavePosition.y + (h - (int)(bufferSamples[i] * h));
            int i2 = (xIndex + 1) * yRatio;
            int y2 = wavePosition.y + (h - (int)(bufferSamples[i2] * h));
            draw.line({ x, y1 }, { x + 1, y2 }, colors.samples);
        }

        std::vector<Data>* envData = (std::vector<Data>*)plugin.data(2);
        if (envData) {

            for (int i = 0; i < envData->size() - 1; i++) {
                Data& data1 = envData->at(i);
                Data& data2 = envData->at(i + 1);
                draw.line({ (int)(wavePosition.x + waveSize.w * data1.time),
                              (int)(wavePosition.y + waveSize.h - waveSize.h * data1.modulation) },
                    { (int)(wavePosition.x + waveSize.w * data2.time),
                        (int)(wavePosition.y + waveSize.h - waveSize.h * data2.modulation) },
                    colors.env);
            }
        }
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
            return true;
        }

        return false;
    }
};

#endif
