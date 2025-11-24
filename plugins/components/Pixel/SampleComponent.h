/** Description:
This code defines a specialized graphical component named `SampleComponent`. Its core purpose is to act as a visual display for an audio sample, typically used within an audio plugin or synthesizer interface.

**What It Does:**

The component takes raw audio data (the waveform) from an external audio processing engine and draws it on the screen. It is designed to visualize critical user-defined points within that audio snippet:

1.  **Start and End Positions:** These markers define the usable segment of the audio file, shown as colored overlays.
2.  **Sustain/Loop Positions:** These points define a section that might be repeated during playback, often visualized with lines indicating the loop start and end.
3.  **Current Playback Position:** A small, distinct marker (like a dot) moves across the waveform in real-time to show exactly where the audio is currently being played.

**How It Works:**

The `SampleComponent` connects directly to a specific `AudioPlugin` to retrieve necessary information, such as the raw sample data and the current values of various control parameters (like "START" position or "LOOP_LENGTH").

It then uses several internal drawing functions to construct the image:
*   First, it draws the background and the fundamental waveform shape.
*   Next, it renders the overlays (start, end, and sustain markers) on top, using color settings defined by the user.
*   Finally, it constantly monitors the audio engine for the current playback location and updates the position of the moving marker, ensuring the visual feedback is instantaneous.

The appearance and the specific audio controls it links to are highly configurable through external settings, allowing users to customize its colors and behavior.

sha: 9678a94d8b00a44541299db1b1d998b7a8cba9034f9c729ff864a40631dd72de 
*/
#pragma once

#include "./utils/BaseWaveComponent.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"
#include "plugins/components/utils/inRect.h"

#include <string>

/*md
## Sample

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/Sample.png" />

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

    void renderStartOverlay()
    {
        if (startPosition != NULL && startPosition->get() > 0.0f) {
            int w = size.w * startPosition->pct();
            draw.filledRect({ relativePosition.x, overlayYtop }, { w, size.h }, { overlayColor });
            draw.line({ relativePosition.x + w, overlayYtop }, { relativePosition.x + w, overlayYbottom }, { overlayEdgeColor });
        }
    }

    void renderEndOverlay()
    {
        if (endPosition != NULL && endPosition->get() < 100.0f) {
            // FIXME overlay too big
            int w = size.w * endPosition->pct();
            draw.filledRect({ relativePosition.x + w, overlayYtop }, { size.w - w, size.h }, { overlayColor });
            draw.line({ relativePosition.x + w, overlayYtop }, { relativePosition.x + w, overlayYbottom }, { overlayEdgeColor });
        }
    }

    void renderSustainOverlay()
    {
        if (sustainPosition != NULL && sustainLength != NULL && sustainLength->get() > 0.0f) {
            int x = relativePosition.x + size.w * sustainPosition->pct();
            draw.line({ x, overlayYtop }, { x, overlayYbottom }, { loopStartColor });

            int w = size.w * sustainLength->pct();
            // draw.filledRect({ x, relativePosition.y }, { w, size.h }, styles.colors.overlay);
            draw.line({ x + w, overlayYtop }, { x + w, overlayYbottom }, { loopEndColor });
        }
    }

    void renderActiveSamples()
    {
        if (sampleIndex != NULL && *sampleIndex != sampleBuffer->count) {
            // int y = relativePosition.y + size.h / 2 - 2;
            int y = relativePosition.y + 4;
            draw.filledRect({ sampleIndexX, y }, { 2, 4 }, { sampleColor });
        }
    }

    Color background;
    Color overlayColor;
    Color overlayEdgeColor;
    Color loopStartColor;
    Color loopEndColor;
    Color sampleColor = rgb(255, 166, 0);

public:
    SampleComponent(ComponentInterface::Props props)
        : Component(props)
        , background(lighten(styles.colors.background, 0.2))
        , overlayColor(alpha(styles.colors.white, 0.2))
        , overlayEdgeColor(alpha(styles.colors.white, 0.4))
        , loopStartColor(styles.colors.white)
        , loopEndColor(styles.colors.white)
        , wave(props)
    {
        jobRendering = [this](unsigned long now) {
            if (sampleIndex != NULL) {
                int x = relativePosition.x + size.w * ((*sampleIndex) / sampleBuffer->count);
                if (sampleIndexX != x) {
                    // printf("rerender sampleIndexX:%d x:%d\n", sampleIndexX, x);
                    sampleIndexX = x;
                    renderNext();
                }
            }
        };

        /*md md_config:Rect */
        nlohmann::json& config = props.config;

        /// The background color of the component
        background = draw.getColor(config["bgColor"], background); //eg: "#000000"

        /// The overlay color of the component
        overlayColor = draw.getColor(config["overlayColor"], overlayColor); //eg: "#a2beb8"

        /// The overlay edge color of the component
        overlayEdgeColor = draw.getColor(config["overlayEdgeColor"], overlayEdgeColor); //eg: "#a2beb8"

        /// The loop start color of the component
        loopStartColor = draw.getColor(config["loopStartColor"], loopStartColor); //eg: "#548ebe"

        /// The loop end color of the component
        loopEndColor = draw.getColor(config["loopEndColor"], loopEndColor); //eg: "#548ebe"

        /// The sample color of the component
        sampleColor = draw.getColor(config["sampleColor"], sampleColor); //eg: "#548ebe"

        /// Value keys
        if (config.contains("valueKeys")) { //eg: {"browser": "BROWSER", "start": "START", "end": "END", "loopPosition": "LOOP_POSITION", "loopLength": "LOOP_LENGTH"}
            // logDebug("valueKeys: %s", config["valueKeys"].dump().c_str());
            valueKeys[0] = config["valueKeys"].value("browser", valueKeys[0]);
            valueKeys[1] = config["valueKeys"].value("start", valueKeys[1]);
            valueKeys[2] = config["valueKeys"].value("end", valueKeys[2]);
            valueKeys[3] = config["valueKeys"].value("loopPosition", valueKeys[3]);
            valueKeys[4] = config["valueKeys"].value("loopLength", valueKeys[4]);
        }

        /// The audio plugin to get control on.
        AudioPlugin* plugin = getPluginPtr(config, "audioPlugin", track); //eg: "audio_plugin_name"
        if (plugin != NULL) {
            sampleBuffer = (struct SampleBuffer*)plugin->data(plugin->getDataId("SAMPLE_BUFFER"));
            sampleIndex = (float*)plugin->data(plugin->getDataId("SAMPLE_INDEX"));

            watch(plugin->getValue(valueKeys[0].c_str())); // watch for file change
            startPosition = watch(plugin->getValue(valueKeys[1].c_str()));
            endPosition = watch(plugin->getValue(valueKeys[2].c_str()));
            sustainPosition = watch(plugin->getValue(valueKeys[3].c_str()));
            sustainLength = watch(plugin->getValue(valueKeys[4].c_str()));
        }

        /*md md_config_end */
    }

    void render()
    {

        if (sampleBuffer != NULL) {
            overlayYtop = relativePosition.y;
            overlayYbottom = relativePosition.y + size.h - 2;

            draw.filledRect(relativePosition, size, { background });
            wave.relativePosition.y = relativePosition.y;
            wave.render(sampleBuffer->data, sampleBuffer->count);

            renderStartOverlay();
            renderEndOverlay();
            renderSustainOverlay();
            renderActiveSamples();
        }
    }
};
