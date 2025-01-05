#ifndef _UI_COMPONENT_PIXEL_SAMPLE_H_
#define _UI_COMPONENT_PIXEL_SAMPLE_H_

#include "./utils/SamplePositionBaseComponent.h"
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
    int bufferDataId = -1;
    int sampleDataId = -1;
    int activeDataId = -1;
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

    BaseWaveComponent wave;

    SamplePositionBaseComponent samplePosition;

    int overlayYtop = 0;
    int overlayYbottom = 0;

    KeypadLayout keypadLayout;

    void renderStartOverlay()
    {
        int w = size.w * startPosition->pct();
        draw.filledRect({ position.x, overlayYtop }, { w, size.h }, { overlayColor });
        draw.line({ position.x + w, overlayYtop }, { position.x + w, overlayYbottom }, { overlayEdgeColor });
    }

    void renderEndOverlay()
    {
        // FIXME overlay too big
        int w = size.w * endPosition->pct();
        draw.filledRect({ position.x + w, overlayYtop }, { size.w - w, size.h }, { overlayColor });
        draw.line({ position.x + w, overlayYtop }, { position.x + w, overlayYbottom }, { overlayEdgeColor });
    }

    void renderSustainOverlay()
    {
        int x = position.x + size.w * sustainPosition->pct();
        draw.line({ x, overlayYtop }, { x, overlayYbottom }, { loopLineColor });

        int w = size.w * sustainLength->pct();
        // draw.filledRect({ x, position.y }, { w, size.h }, styles.colors.overlay);
        draw.line({ x + w, overlayYtop }, { x + w, overlayYbottom }, { loopLineColor });
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

    Color background;
    Color overlayColor;
    Color overlayEdgeColor;
    Color loopLineColor;

public:
    SampleComponent(ComponentInterface::Props props)
        : Component(props)
        , background(lighten(styles.colors.background, 0.2))
        , overlayColor({ 0x90, 0x90, 0x90, 100 })
        , overlayEdgeColor({ 0x90, 0x90, 0x90 })
        , loopLineColor(styles.colors.white)
        , wave(props)
        , samplePosition(props)
        , keypadLayout(this)
    {
        overlayYtop = position.y;
        overlayYbottom = position.y + size.h - 2;

        // jobRendering = [this](unsigned long now) {
        //     if (plugin && activeDataId != -1) {
        //         if (samplePosition.shouldRender(plugin->data(activeDataId))) {
        //             renderNext();
        //         }
        //     }
        // };
    }

    void render()
    {

        if (plugin != NULL && updatePosition()) {
            draw.filledRect(relativePosition, size, { background });
            renderWaveform();

            renderStartOverlay();
            renderEndOverlay();
            if (sustainLength->get() > 0.0f) {
                renderSustainOverlay();
            }
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

        /*md - `KEYS: BROWSER START END SUSTAIN LENGTH` set the key parameter to use from plugin */
        if (strcmp(key, "KEYS") == 0) {
            valueKeys[0] = strtok(value, " ");
            valueKeys[1] = strtok(NULL, " ");
            valueKeys[2] = strtok(NULL, " ");
            valueKeys[3] = strtok(NULL, " ");
            valueKeys[4] = strtok(NULL, " ");
            return true;
        }

        /*md - `PLUGIN: pluginName bufferDataId` set the plugin to use from plugin */
        if (strcmp(key, "PLUGIN") == 0) {
            char* pluginName = strtok(value, " ");
            plugin = &getPlugin(pluginName, track);
            bufferDataId = plugin->getDataId(strtok(NULL, " "));

            watch(plugin->getValue(valueKeys[0].c_str())); // watch for file change
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
