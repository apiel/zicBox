#ifndef _UI_COMPONENT_PIXEL_SAMPLE_H_
#define _UI_COMPONENT_PIXEL_SAMPLE_H_

#include "./utils/BaseWaveComponent.h"
#include "plugins/components/base/KeypadLayout.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"
#include "plugins/components/utils/inRect.h"

#include <string>

/*md
## Sample

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/pixel/Sample.png" />

Sample is used to display an audio sample, sustain position, and start/end position.
The little green dot are the current playing positions of the sample.

*/
class SampleComponent : public Component {
protected:
    AudioPlugin* plugin;
    float lastBrowser = -1.0f;
    ValueInterface* startPosition;
    ValueInterface* endPosition;
    ValueInterface* sustainPosition;
    ValueInterface* sustainLength;

    struct SampleBuffer {
        uint64_t count;
        float* data;
    }* sampleBuffer = NULL;
    float* sampleIndex = NULL;
    int sampleIndexX = -1;

    std::string valueKeys[5] = {
        "BROWSER",
        "START",
        "END",
        "LOOP_POSITION",
        "LOOP_LENGTH",
    };

    void* textureSampleWaveform = NULL;

    BaseWaveComponent wave;

    int overlayYtop = 0;
    int overlayYbottom = 0;

    KeypadLayout keypadLayout;

    void renderStartOverlay()
    {
        if (startPosition != NULL && startPosition->get() > 0.0f) {
            int w = size.w * startPosition->pct();
            draw.filledRect({ position.x, overlayYtop }, { w, size.h }, { overlayColor });
            draw.line({ position.x + w, overlayYtop }, { position.x + w, overlayYbottom }, { overlayEdgeColor });
        }
    }

    void renderEndOverlay()
    {
        if (endPosition != NULL && endPosition->get() < 100.0f) {
            // FIXME overlay too big
            int w = size.w * endPosition->pct();
            draw.filledRect({ position.x + w, overlayYtop }, { size.w - w, size.h }, { overlayColor });
            draw.line({ position.x + w, overlayYtop }, { position.x + w, overlayYbottom }, { overlayEdgeColor });
        }
    }

    void renderSustainOverlay()
    {
        if (sustainPosition != NULL && sustainLength != NULL && sustainLength->get() > 0.0f) {
            int x = position.x + size.w * sustainPosition->pct();
            draw.line({ x, overlayYtop }, { x, overlayYbottom }, { loopStartColor });

            int w = size.w * sustainLength->pct();
            // draw.filledRect({ x, position.y }, { w, size.h }, styles.colors.overlay);
            draw.line({ x + w, overlayYtop }, { x + w, overlayYbottom }, { loopEndColor });
        }
    }

    void renderWaveform()
    {
        wave.render(sampleBuffer->data, sampleBuffer->count);
    }

    void renderActiveSamples()
    {
        if (sampleIndex != NULL && *sampleIndex != sampleBuffer->count) {
            draw.filledRect({ sampleIndexX, relativePosition.y }, { 1, size.h }, { sampleColor });
        }
    }

    Color background;
    Color overlayColor;
    Color overlayEdgeColor;
    Color loopStartColor;
    Color loopEndColor;
    Color sampleColor = { 0x9d, 0xfe, 0x86 }; // #9dfe89

public:
    SampleComponent(ComponentInterface::Props props)
        : Component(props)
        , background(lighten(styles.colors.background, 0.2))
        , overlayColor(alpha(styles.colors.white, 0.2))
        , overlayEdgeColor(alpha(styles.colors.white, 0.4))
        , loopStartColor(styles.colors.white)
        , loopEndColor(styles.colors.white)
        , wave(props)
        , keypadLayout(this)
    {
        overlayYtop = position.y;
        overlayYbottom = position.y + size.h - 2;

        jobRendering = [this](unsigned long now) {
            if (sampleIndex != NULL) {
                int x = relativePosition.x + size.w * ((*sampleIndex) / sampleBuffer->count);
                if (sampleIndexX != x) {
                    sampleIndexX = x;
                    renderNext();
                }
            }
        };
    }

    void render()
    {

        if (sampleBuffer != NULL && updatePosition()) {
            draw.filledRect(relativePosition, size, { background });
            renderWaveform();

            renderStartOverlay();
            renderEndOverlay();
            renderSustainOverlay();
            renderActiveSamples();
        }
    }

    bool config(char* key, char* value)
    {
        /*md - `BACKGROUND_COLOR: #000000` set background color */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            background = draw.getColor(value);
            return true;
        }

        /*md - `OVERLAY_COLOR:#a2beb8` set overlay color */
        if (strcmp(key, "OVERLAY_COLOR") == 0) {
            overlayColor = draw.getColor(value);
            return true;
        }

        /*md - `OVERLAY_EDGE_COLOR:#a6b1ab` set overlay edge color */
        if (strcmp(key, "OVERLAY_EDGE_COLOR") == 0) {
            overlayEdgeColor = draw.getColor(value);
            return true;
        }

        /*md - `LOOP_START_COLOR:#548ebe` set loop start point color */
        if (strcmp(key, "LOOP_START_COLOR") == 0) {
            loopStartColor = draw.getColor(value);
            return true;
        }

        /*md - `LOOP_END_COLOR:#548ebe` set loop end point color */
        if (strcmp(key, "LOOP_END_COLOR") == 0) {
            loopEndColor = draw.getColor(value);
            return true;
        }

        /*md - `LOOP_POINTS_COLOR:#548ebe` set loop end and start point color */
        if (strcmp(key, "LOOP_POINTS_COLOR") == 0) {
            loopStartColor = draw.getColor(value);
            loopEndColor = draw.getColor(value);
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

        /*md - `PLUGIN: pluginName bufferDataId [sampleIndexDataId]` set the plugin to use from plugin */
        if (strcmp(key, "PLUGIN") == 0) {
            char* pluginName = strtok(value, " ");
            plugin = &getPlugin(pluginName, track);
            sampleBuffer = (struct SampleBuffer*)plugin->data(plugin->getDataId(strtok(NULL, " ")));
            char* sampleIndexDataIdStr = strtok(NULL, " ");
            if (sampleIndexDataIdStr != NULL) {
                sampleIndex = (float*)plugin->data(plugin->getDataId(sampleIndexDataIdStr));
            }

            watch(plugin->getValue(valueKeys[0].c_str())); // watch for file change
            startPosition = watch(plugin->getValue(valueKeys[1].c_str()));
            endPosition = watch(plugin->getValue(valueKeys[2].c_str()));
            sustainPosition = watch(plugin->getValue(valueKeys[3].c_str()));
            sustainLength = watch(plugin->getValue(valueKeys[4].c_str()));
            return true;
        }

        if (keypadLayout.config(key, value)) {
            return true;
        }

        return wave.config(key, value);
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        keypadLayout.onKey(id, key, state, now);
    }
};

#endif
