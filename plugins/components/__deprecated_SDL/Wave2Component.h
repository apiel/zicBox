#ifndef _UI_COMPONENT_WAVE2_H_
#define _UI_COMPONENT_WAVE2_H_

#include "../utils/inRect.h"
#include "../utils/color.h"
#include "./base/WaveBaseComponent.h"
#include "../component.h"
#include <string>

class Wave2Component : public Component {
protected:
    AudioPlugin& plugin;
    ValueInterface* browser = watch(getPlugin("Granular", track).getValue("BROWSER"));
    float lastBrowser = -1.0f;

    Size textureSize;
    void* textureSampleWaveform = NULL;

    WaveBaseComponent wave;

    struct Colors {
        Color background;
        Color info;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({
            darken(color, 0.75),
            darken(color, 0.3),
        });
    }

    const int margin;

public:
    Wave2Component(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.primary))
        , margin(styles.margin)
        , plugin(getPlugin("Granular", track))
        , wave(getNewPropsRect(props,
              { { 0, 20 },
                  { props.size.w - 2 * styles.margin, (int)(props.size.h - 2 * (20 + styles.margin)) } }))
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
            lastBrowser = browser->get();
            textureSampleWaveform = draw.setTextureRenderer(textureSize);
            draw.filledRect({ 0, 0 }, { textureSize.w, textureSize.h }, { colors.background });
            wave.render((float*)plugin.data(1), *(uint64_t*)plugin.data(0));
            draw.text({ 10, 5 }, browser->string().c_str(), 12, { colors.info });
            draw.setMainRenderer();
        }
        draw.applyTexture(textureSampleWaveform, { { position.x + margin, position.y + margin }, textureSize });
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
            wave.setColors(draw.getColor(value));
            return true;
        }

        return wave.config(key, value);
    }
};

#endif
