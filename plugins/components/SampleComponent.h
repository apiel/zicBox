#ifndef _UI_COMPONENT_SAMPLE_H_
#define _UI_COMPONENT_SAMPLE_H_

#include "../../helpers/inRect.h"
#include "./base/WaveBaseComponent.h"
#include "./component.h"
#include <string>

class SampleComponent : public Component {
protected:
    AudioPlugin* plugin;
    int bufferDataId = -1;
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
        "SUSTAIN_POSITION",
        "SUSTAIN_LENGTH",
    };

    void* textureSampleWaveform = NULL;

    Rect waveRect;
    WaveBaseComponent wave;

    int overlayYtop = 0;
    int overlayYbottom = 0;

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
        struct SampleState {
            float position;
            int index;
            float release;
        };
        std::vector<SampleState>* sampleStates = (std::vector<SampleState>*)plugin->data(2);
        int spacing = 13;
        int totalHeight = sampleStates->size() * spacing;
        int marginTop = 5;
        if (totalHeight < wave.size.h) {
            marginTop = (wave.size.h - totalHeight) / 2;
        }
        int y = position.y + marginTop;
        for (auto& sample : *sampleStates) {
            int x = position.x + sample.position * wave.size.w;
            Color color = colors.sample;
            if (sample.release != 1.0) {
                color = draw.alpha(color, sample.release);
            }
            draw.filledRect({ x, y + (sample.index * spacing % (wave.size.h - 10)) }, { 4, 4 }, color);
        }
    }

    struct Colors {
        Color background;
        Color info;
        Color overlay;
        Color overlayEdge;
        Color loopLine;
        Color sample;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.75),
            draw.darken(color, 0.3),
            draw.alpha(draw.darken(color, 0.80), 0.6),
            draw.alpha(draw.darken(color, 0.90), 0.3),
            draw.alpha(styles.colors.white, 0.3),
            draw.lighten(draw.getColor((char*)"#9dfe86"), 0.3) });
    }

    bool jobDidRender = false;

public:
    SampleComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
        , waveRect({ { 0, 20 }, { props.size.w, (int)(props.size.h - 2 * 20) } })
        , wave(getNewPropsRect(props, { { 0, 20 }, { props.size.w, (int)(props.size.h - 2 * 20) } }))
    {
        overlayYtop = position.y;
        overlayYbottom = position.y + size.h - 2;

        jobRendering = [this](unsigned long now) {
            if (plugin) {
                void* active = (void*)plugin->data(1);
                if (active) {
                    jobDidRender = true;
                    renderNext();
                    return;
                }
                if (jobDidRender) {
                    jobDidRender = false;
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
                plugin->noteOn(60, 127);
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
        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
            wave.setColors(draw.getColor(value));
            return true;
        }

        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            colors.background = draw.getColor(value);
            return true;
        }

        if (strcmp(key, "SAMPLE_COLOR") == 0) {
            colors.sample = draw.getColor(value);
            return true;
        }

        if (strcmp(key, "KEYS") == 0) {
            valueKeys[0] = strtok(value, " ");
            valueKeys[1] = strtok(NULL, " ");
            valueKeys[2] = strtok(NULL, " ");
            valueKeys[3] = strtok(NULL, " ");
            valueKeys[4] = strtok(NULL, " ");
            return true;
        }

        if (strcmp(key, "AUDIO_PLUGIN") == 0) {
            char* pluginName = strtok(value, " ");
            bufferDataId = atoi(strtok(NULL, " "));
            plugin = &getPlugin(pluginName);
            browser = val(plugin->getValue(valueKeys[0].c_str()));
            startPosition = val(plugin->getValue(valueKeys[1].c_str()));
            endPosition = val(plugin->getValue(valueKeys[2].c_str()));
            sustainPosition = val(plugin->getValue(valueKeys[3].c_str()));
            sustainLength = val(plugin->getValue(valueKeys[4].c_str()));
            return true;
        }

        return wave.config(key, value);
    }
};

#endif
