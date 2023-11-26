#ifndef _UI_COMPONENT_WAVE2_H_
#define _UI_COMPONENT_WAVE2_H_

#include "../../helpers/inRect.h"
#include "./base/WaveBaseComponent.h"
#include "component.h"
#include <string>

class Wave2Component : public Component {
protected:
    AudioPlugin& plugin;
    ValueInterface* browser = val(getPlugin("Granular").getValue("BROWSER"));
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
            draw.darken(color, 0.75),
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
        , wave(props)
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
            draw.filledRect({ 0, 0 }, { textureSize.w, textureSize.h }, colors.background);
            wave.render((float*)plugin.data(1), *(uint64_t*)plugin.data(0));
            draw.text({ 10, 5 }, browser->string().c_str(), colors.info, 12);
            draw.setMainRenderer();
        }
        draw.applyTexture(textureSampleWaveform, { { position.x + margin, position.y + margin }, textureSize });
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
