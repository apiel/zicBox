#ifndef _UI_COMPONENT_WAVE2_H_
#define _UI_COMPONENT_WAVE2_H_

#include "../../helpers/inRect.h"
#include "component.h"
#include <string>

class Wave2Component : public Component {
protected:
    AudioPlugin& plugin;
    ValueInterface* browser = val(getPlugin("Granular").getValue("BROWSER"));
    float lastBrowser = -1.0f;

    MotionInterface* motion1 = NULL;
    MotionInterface* motion2 = NULL;

    Size textureSize;
    void* textureSampleWaveform = NULL;

    void renderSampleWaveform()
    {
        if (textureSampleWaveform == NULL) {
            lastBrowser = browser->get();
            textureSampleWaveform = draw.setTextureRenderer(textureSize);

            draw.filledRect({ 0, 0 }, { textureSize.w, textureSize.h }, colors.background);

            uint64_t* samplesCount = (uint64_t*)plugin.data(0);
            float* bufferSamples = (float*)plugin.data(1);
            int h = textureSize.h * 0.5f;
            for (int i = 0; i < *samplesCount; i++) {
                int x = margin + (i * textureSize.w / *samplesCount);
                int graphH = bufferSamples[i] * h;
                if (graphH) {
                    int y1 = margin + (h - graphH);
                    int y2 = margin + (h + graphH);
                    draw.line({ x, y1 }, { x, y2 }, colors.wave);
                    draw.line({ x, (int)(y1 + graphH * 0.5) }, { x, (int)(y2 - graphH * 0.5) }, colors.waveOutline);
                } else {
                    draw.pixel({ x, margin + h }, colors.waveOutline);
                }
            }

            draw.text({ 10, 5 }, browser->string().c_str(), colors.info, 12);
            draw.setMainRenderer();
        }
        draw.applyTexture(textureSampleWaveform, { { position.x + margin, position.y + margin }, textureSize });
    }

    struct Colors {
        Color background;
        Color wave;
        Color waveOutline;
        Color info;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({
            draw.darken(color, 0.75),
            draw.darken(color, 0.4),
            color,
            draw.darken(color, 0.3),
        });
    }

    const int margin;

public:
    Wave2Component(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
        , margin(styles.margin)
        , plugin(getPlugin("Granular"))
    {
        textureSize = { size.w - 2 * margin, size.h - 2 * margin };
    }

    void render()
    {
        if (lastBrowser != browser->get()) {
            if (textureSampleWaveform != NULL) {
                draw.destroyTexture(textureSampleWaveform);
                textureSampleWaveform = NULL;
            }
        }
        renderSampleWaveform();
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
