/** Description:
This component is a specialized user interface element designed to visualize audio data, specifically displaying a waveform associated with an audio processing plugin. Think of it as a graphical window showing the shape of a sound.

### How It Works

This UI element is tightly integrated with an internal "Granular" audio plugin. Its primary function is displaying the audio sample currently loaded by the plugin's file "browser" setting.

1.  **Monitoring and Optimization:** The component constantly watches the value that controls which audio sample is selected. If the user loads a *new* sample (the value changes), the component triggers a full redraw. It renders the complex waveform onto a dedicated graphic buffer (a "texture").
2.  **Efficient Display:** Once the waveform image is generated, the component saves it. In subsequent display cycles, if the selected audio sample has *not* changed, the component simply pastes the saved image onto the screen instantly. This highly optimized process avoids re-calculating the complex waveform shape every moment, ensuring smooth performance.
3.  **Drawing:** It relies on a separate, dedicated sub-component (`WaveBaseComponent`) to handle the precise mathematical drawing of the wave itself, while the main component handles the surrounding frame, background colors, and informational text (like the sample name).
4.  **Customization:** The component allows configuration changes, primarily letting the user customize the background and text colors to match the application's overall theme.

sha: f27dba5544fa242febc9678df10fd7dc80b172f02b1fc3ddc4679c9785356c5a 
*/
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
