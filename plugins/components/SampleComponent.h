#ifndef _UI_COMPONENT_SAMPLE_H_
#define _UI_COMPONENT_SAMPLE_H_

#include "../../helpers/inRect.h"
#include "./base/KeypadLayout.h"
#include "./base/SamplePositionBaseComponent.h"
#include "./base/WaveBaseComponent.h"
#include "./component.h"
#include <string>

/*md
## Sample

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Sample.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Sample2.png" />

Sample is used to display an audio sample, sustain position, and start/end position.
The little green dot are the current playing positions of the sample.

[SampleComponent.webm](https://github.com/apiel/zicBox/assets/5873198/a31fe431-58dc-4379-bfd1-a752d2ea579d)

*/
class SampleComponent : public Component {
protected:
    AudioPlugin* plugin;
    int bufferDataId = -1;
    int sampleDataId = -1;
    int activeDataId = -1;
    ValueInterface* browser;
    float lastBrowser = -1.0f;
    ValueInterface* startPosition;
    ValueInterface* sustainPosition;
    ValueInterface* sustainLength;
    ValueInterface* endPosition;

    std::string valueKeys[5] = {
        "BROWSER",
        "START",
        "END",
        "LOOP_POSITION",
        "LOOP_LENGTH",
    };

    void* textureSampleWaveform = NULL;

    Rect waveRect;
    WaveBaseComponent wave;

    SamplePositionBaseComponent samplePosition;

    int overlayYtop = 0;
    int overlayYbottom = 0;

    KeypadLayout keypadLayout;

    void renderStartOverlay()
    {
        int w = size.w * startPosition->pct();
        draw.filledRect({ position.x, overlayYtop }, { w, size.h }, colors.overlay);
        draw.line({ position.x + w, overlayYtop }, { position.x + w, overlayYbottom }, colors.overlayEdge);
    }

    void renderEndOverlay()
    {
        // FIXME overlay too big
        int w = size.w * endPosition->pct();
        draw.filledRect({ position.x + w, overlayYtop }, { size.w - w, size.h }, colors.overlay);
        draw.line({ position.x + w, overlayYtop }, { position.x + w, overlayYbottom }, colors.overlayEdge);
    }

    void renderSustainOverlay()
    {
        int x = position.x + size.w * sustainPosition->pct();
        draw.line({ x, overlayYtop }, { x, overlayYbottom }, colors.loopLine);

        int w = size.w * sustainLength->pct();
        // draw.filledRect({ x, position.y }, { w, size.h }, styles.colors.overlay);
        draw.line({ x + w, overlayYtop }, { x + w, overlayYbottom }, colors.loopLine);
    }

    void renderWaveform()
    {
        struct SampleBuffer {
            uint64_t count;
            float* data;
        }* sampleBuffer = (struct SampleBuffer*)plugin->data(bufferDataId);
        wave.render(sampleBuffer->data, sampleBuffer->count);
    }

    void renderActiveSamples()
    {
        if (sampleDataId != -1) {
            samplePosition.position.x = position.x + size.w * startPosition->pct();
            samplePosition.size.w = size.w * endPosition->pct() - samplePosition.position.x;
            samplePosition.render((std::vector<SamplePositionBaseComponent::SampleState>*)plugin->data(sampleDataId));
        }
    }

    struct Colors {
        Color background;
        Color info;
        Color overlay;
        Color overlayEdge;
        Color loopLine;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.75),
            draw.darken(color, 0.3),
            draw.alpha(draw.darken(color, 0.80), 0.6),
            draw.alpha(draw.darken(color, 0.90), 0.3),
            draw.alpha(styles.colors.white, 0.3) });
    }

public:
    /*md **Keyboard actions**: */
    void addKeyMap(KeypadInterface* controller, uint16_t controllerId, uint8_t key, int param, std::string action, uint8_t color)
    {
        /*md - `play` is used to play the sample. `KEYMAP: Keyboard 44 play 60` will trigger note on 60 when pressing space on keyboard. */
        if (action == "play") {
            keypadLayout.mapping.push_back({ controller, controllerId, key, param, [&](int8_t state, KeypadLayout::KeyMap& keymap) {
                if (plugin) {
                    if (state) {
                        plugin->noteOn(keymap.param, 1.0f);
                    } else {
                        plugin->noteOff(keymap.param, 0.0f);
                    }
                } }, color, [&](KeypadLayout::KeyMap& keymap) { return keymap.color || 20; } });
        }
    }

    SampleComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
        , waveRect({ { 0, 20 }, { props.size.w, (int)(props.size.h - 2 * 20) } })
        , wave(getNewPropsRect(props, { { 0, 20 }, { props.size.w, (int)(props.size.h - 2 * 20) } }))
        , samplePosition(getNewPropsRect(props, { { props.position.x, props.position.y + 20 }, { props.size.w, (int)(props.size.h - 2 * 20) } }))
        , keypadLayout(getController, [&](KeypadInterface* controller, uint16_t controllerId, int8_t key, int param, std::string action, uint8_t color) { addKeyMap(controller, controllerId, key, param, action, color); })
    {
        overlayYtop = position.y;
        overlayYbottom = position.y + size.h - 2;

        jobRendering = [this](unsigned long now) {
            if (plugin && activeDataId != -1) {
                if (samplePosition.shouldRender(plugin->data(activeDataId))) {
                    renderNext();
                }
            }
        };
    }

    void render()
    {
        if (plugin == NULL) {
            return;
        }

        if (lastBrowser != browser->get()) {
            if (textureSampleWaveform != NULL) {
                draw.destroyTexture(textureSampleWaveform);
                textureSampleWaveform = NULL;
            }
            lastBrowser = browser->get();
            textureSampleWaveform = draw.setTextureRenderer(size);
            draw.filledRect({ 0, 0 }, size, colors.background);
            renderWaveform();
            draw.text({ 10, 5 }, browser->string().c_str(), colors.info, 12);
            draw.setMainRenderer();
        }
        draw.applyTexture(textureSampleWaveform, { position, size });
        renderStartOverlay();
        renderEndOverlay();
        if (sustainLength->get() > 0.0f) {
            renderSustainOverlay();
        }
        renderActiveSamples();
    }

    bool noteTriggered = false;
    void onMotion(MotionInterface& motion)
    {
        if (!noteTriggered) {
            noteTriggered = true;
            if (plugin) {
                plugin->noteOn(60, 1.0f);
            }
        }
    }

    void onMotionRelease(MotionInterface& motion)
    {
        if (noteTriggered) {
            noteTriggered = false;
            if (plugin) {
                plugin->noteOff(60, 0);
            }
        }
    }

    bool config(char* key, char* value)
    {
        /*md - `COLOR: #FFFFFF` set color of the waveform */
        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
            wave.setColors(draw.getColor(value));
            return true;
        }

        /*md - `BACKGROUND_COLOR: #000000` set background color */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            colors.background = draw.getColor(value);
            return true;
        }

        /*md - `KEYS: BROWSER START END SUSTAIN LENGTH` set the key parameter to use from plugin */
        if (strcmp(key, "KEYS") == 0) {
            valueKeys[0] = strtok(value, " ");
            valueKeys[1] = strtok(NULL, " ");
            valueKeys[2] = strtok(NULL, " ");
            valueKeys[3] = strtok(NULL, " ");
            valueKeys[4] = strtok(NULL, " ");
            return true;
        }

        /*md - `AUDIO_PLUGIN: pluginName bufferDataId` set the plugin to use from plugin */
        if (strcmp(key, "AUDIO_PLUGIN") == 0) {
            char* pluginName = strtok(value, " ");
            bufferDataId = atoi(strtok(NULL, " "));
            plugin = &getPlugin(pluginName, track);

            browser = watch(plugin->getValue(valueKeys[0].c_str()));
            startPosition = watch(plugin->getValue(valueKeys[1].c_str()));
            endPosition = watch(plugin->getValue(valueKeys[2].c_str()));
            sustainPosition = watch(plugin->getValue(valueKeys[3].c_str()));
            sustainLength = watch(plugin->getValue(valueKeys[4].c_str()));
            return true;
        }

        if (strcmp(key, "DATA_STATE") == 0) {
            sampleDataId = atoi(strtok(value, " "));
            activeDataId = atoi(strtok(NULL, " "));
            return true;
        }

        if (keypadLayout.config(key, value)) {
            return true;
        }

        return samplePosition.config(key, value) || wave.config(key, value);
    }

    void onKey(uint16_t id, int key, int8_t state)
    {
        keypadLayout.onKey(id, key, state);
    }
};

#endif
