/** Description:
## UI Component Analysis: SampleComponent

This C++ header defines a specialized user interface element called the `SampleComponent`. Its primary function is to provide a visual, interactive display of an audio sound sample, effectively serving as a miniature audio editor within a larger application.

### Core Purpose and Display

The component is designed to visualize a loaded sound clip (an audio waveform). It doesn't just display the wave itself; it acts as a monitor and control panel by highlighting key points of the sample:
1.  **Start and End Points:** Using colored, semi-transparent overlays, it clearly shows the sections of the audio that are being used.
2.  **Loop/Sustain Points:** It marks the section where the audio is set to loop during sustained playback.
3.  **Playback Markers:** During actual playback, small dots track the current active position within the waveform, providing real-time feedback.

### How it Works

The component operates by connecting directly to an external sound processing module (an "Audio Plugin").

1.  **Data Linkage:** It reads the raw audio data (the waveform) and monitors crucial control parameters—such as the percentage values for "Start Position" or "Loop Length"—which are provided by the plugin.
2.  **Efficient Rendering:** To ensure high performance, the static waveform image is generated once and stored in memory (as a texture). This way, the component only has to redraw the dynamic elements—like the boundary overlays, loop lines, and moving playback dots—in real-time, making the interface very responsive.
3.  **Interaction:** It handles basic touch or motion input and is configurable via keyboard mapping. Users can map keys to trigger "Note On" and "Note Off" events, allowing them to instantly play the loaded sample.

In essence, the `SampleComponent` is a dynamic viewport that allows a user to monitor, analyze, and visually adjust the playback boundaries of an audio sample being managed by the connected sound engine.

sha: 6eaccf608e46fb89c6c4179d1c7ef04b4ed588545de4da1b441a497e9c7c1d3d 
*/
#ifndef _UI_COMPONENT_SAMPLE_H_
#define _UI_COMPONENT_SAMPLE_H_

#include "../base/KeypadLayout.h"
#include "../component.h"
#include "../utils/color.h"
#include "../utils/inRect.h"
#include "./base/SamplePositionBaseComponent.h"
#include "./base/WaveBaseComponent.h"
#include <string>

/*md
## SDL Sample

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/Sample.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/Sample2.png" />

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
        draw.filledRect({ position.x, overlayYtop }, { w, size.h }, { colors.overlay });
        draw.line({ position.x + w, overlayYtop }, { position.x + w, overlayYbottom }, { colors.overlayEdge });
    }

    void renderEndOverlay()
    {
        // FIXME overlay too big
        int w = size.w * endPosition->pct();
        draw.filledRect({ position.x + w, overlayYtop }, { size.w - w, size.h }, { colors.overlay });
        draw.line({ position.x + w, overlayYtop }, { position.x + w, overlayYbottom }, { colors.overlayEdge });
    }

    void renderSustainOverlay()
    {
        int x = position.x + size.w * sustainPosition->pct();
        draw.line({ x, overlayYtop }, { x, overlayYbottom }, { colors.loopLine });

        int w = size.w * sustainLength->pct();
        // draw.filledRect({ x, position.y }, { w, size.h }, styles.colors.overlay);
        draw.line({ x + w, overlayYtop }, { x + w, overlayYbottom }, { colors.loopLine });
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
        return Colors({ darken(color, 0.75),
            darken(color, 0.3),
            alpha(darken(color, 0.80), 0.6),
            alpha(darken(color, 0.90), 0.3),
            alpha(styles.colors.white, 0.3) });
    }

public:
    SampleComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.primary))
        , waveRect({ { 0, 20 }, { props.size.w, (int)(props.size.h - 2 * 20) } })
        , wave(getNewPropsRect(props, { { 0, 20 }, { props.size.w, (int)(props.size.h - 2 * 20) } }))
        , samplePosition(getNewPropsRect(props, { { props.position.x, props.position.y + 20 }, { props.size.w, (int)(props.size.h - 2 * 20) } }))
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            /*md **Keyboard actions**: */
            /*md - `play` is used to play the sample. `KEYMAP: Keyboard 44 play 60` will trigger note on 60 when pressing space on keyboard. */
            if (action.rfind("play:") == 0) {
                uint8_t* note = new uint8_t(atoi(action.substr(5).c_str()));
                func = [this, note](KeypadLayout::KeyMap& keymap) {
                    if (plugin) {
                        if (keymap.pressedTime != -1) {
                            plugin->noteOn(*note, 1.0f);
                        } else {
                            plugin->noteOff(*note, 0.0f);
                        }
                    }
                };
            }
            return func;
        })
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
            draw.filledRect({ 0, 0 }, size, { colors.background });
            renderWaveform();
            draw.text({ 10, 5 }, browser->string().c_str(), 12, { colors.info });
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

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        keypadLayout.onKey(id, key, state, now);
    }
};

#endif
