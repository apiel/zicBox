#ifndef _UI_COMPONENT_WAVE_H_
#define _UI_COMPONENT_WAVE_H_

#include <string>
#include <vector>

#include "./base/KeypadLayout.h"
#include "./utils/color.h"
#include "component.h"

/*md
## WaveComponent

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Wave.png" />

WaveComponent will display the represention of a waveform and associated modulation envelope.
*/
class WaveComponent : public Component {
protected:
    AudioPlugin* plugin = NULL;
    ValueInterface* browser = NULL;

    Size waveSize;
    Point wavePosition;
    void* textureWave = NULL;

    struct Data {
        float modulation;
        float time;
    };

    struct Colors {
        Color background;
        Color samples;
        Color samplesFill;
        Color env;
        Color info;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ darken(color, 0.75),
            color,
            alpha(color, 0.15),
            darken(color, 0.65),
            darken(color, 0.3) });
    }

    const int margin;

    int lastUpdateUi = -1;

    KeypadLayout keypadLayout;

    uint8_t envDataId = 2;

    void renderWaveName()
    {
        if (browser) {
            draw.text({ position.x + 10, position.y + 5 }, browser->string().c_str(), colors.info, 12);
        }
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

    WaveComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
        , margin(styles.margin)
        , keypadLayout(getController, [&](KeypadInterface* controller, uint16_t controllerId, int8_t key, int param, std::string action, uint8_t color) { addKeyMap(controller, controllerId, key, param, action, color); })
    {
        waveSize = { size.w - 2 * margin, size.h - 2 * margin };
        wavePosition = { position.x + margin, position.y + margin };

        jobRendering = [this](unsigned long now) {
            if (plugin) {
                int* last = (int*)plugin->data(0);
                if (*last != lastUpdateUi) {
                    lastUpdateUi = *last + 0;
                    renderNext();
                }
            }
        };
    }

    void render()
    {
        draw.filledRect(wavePosition, size, colors.background);

        if (plugin) {
            std::vector<Data>* envData = (std::vector<Data>*)plugin->data(envDataId);
            if (envData) {
                std::vector<Point> points = { { wavePosition.x, wavePosition.y + waveSize.h } };
                // printf("envData size %lu\n", envData->size());
                for (int i = 0; i < envData->size() - 1; i++) {
                    Data& data1 = envData->at(i);
                    // printf("%f %f\n", data1.modulation, data1.time);
                    points.push_back({ (int)(wavePosition.x + waveSize.w * data1.time),
                        (int)(wavePosition.y + waveSize.h - waveSize.h * data1.modulation) });
                }
                points.push_back({ (int)(wavePosition.x + waveSize.w), (int)(wavePosition.y + waveSize.h) });

                draw.filledPolygon(points, colors.env);
            }

            uint64_t samplesCount = 1000;
            float* bufferSamples = (float*)plugin->data(1);

            int h = waveSize.h * 0.5f;
            float yRatio = samplesCount / waveSize.w;
            for (int xIndex = 0; xIndex < waveSize.w - 1; xIndex++) {
                {
                    int x = wavePosition.x + xIndex;
                    int i = xIndex * yRatio;
                    int y1 = wavePosition.y + (h - (int)(bufferSamples[i] * h));
                    int i2 = (xIndex + 1) * yRatio;
                    int y2 = wavePosition.y + (h - (int)(bufferSamples[i2] * h));
                    draw.aaline({ x, y1 }, { x + 1, y2 }, colors.samples);
                }
                {
                    // draw line from center
                    int x = wavePosition.x + xIndex;
                    int y1 = wavePosition.y + waveSize.h / 2;
                    int i = xIndex * yRatio;
                    int y2 = wavePosition.y + (h - (int)(bufferSamples[i] * h));
                    draw.line({ x, y1 }, { x, y2 }, colors.samplesFill);
                    draw.pixel({ x, y2 }, colors.samples);
                }
            }
        }
        renderWaveName();
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
            return true;
        }

        /*md - `BACKGROUND_COLOR: #000000` set background color */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            colors.background = draw.getColor(value);
            return true;
        }

        /*md - `ENVELOP_DATA_ID: id` is the id of the envelope data.*/
        if (strcmp(key, "ENVELOP_DATA_ID") == 0) {
            envDataId = atoi(value);
            return true;
        }

        /*md - `PLUGIN: plugin_name` set plugin target */
        if (strcmp(key, "PLUGIN") == 0) {
            plugin = &getPlugin(value, track);
            return true;
        }

        /*md - `NAME: key` set key to get name from wave */
        if (strcmp(key, "NAME") == 0) {
            if (plugin) {
                browser = watch(plugin->getValue(value));
            }
            return true;
        }

        if (keypadLayout.config(key, value)) {
            return true;
        }

        return false;
    }

    void onKey(uint16_t id, int key, int8_t state)
    {
        keypadLayout.onKey(id, key, state);
    }
};

#endif
