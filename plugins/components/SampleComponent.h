#ifndef _UI_COMPONENT_SAMPLE_H_
#define _UI_COMPONENT_SAMPLE_H_

#include "../../helpers/inRect.h"
#include "./base/WaveBaseComponent.h"
#include "./component.h"
#include <string>

class SampleComponent : public Component {
protected:
    AudioPlugin& plugin;
    ValueInterface* browser;
    float lastBrowser = -1.0f;
    ValueInterface* startPosition;
    ValueInterface* sustainPosition;
    ValueInterface* sustainLength;
    ValueInterface* endPosition;

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

        float length = sustainPosition->pct();
        if (length > 0.0f) {
            int w = size.w * length;
            // draw.filledRect({ x, position.y }, { w, size.h }, styles.colors.overlay);
            draw.line({ x + w, overlayYtop }, { x + w, overlayYbottom }, colors.loopLine);
        }
    }

    void renderSamples()
    {
    }

    struct Colors {
        Color background;
        Color info;
        Color overlay;
        Color overlayEdge;
        Color loopLine;
        Color sampleCursor;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.75),
            draw.darken(color, 0.3),
            draw.alpha(draw.darken(color, 0.80), 0.6),
            draw.alpha(draw.darken(color, 0.90), 0.3),
            draw.alpha(styles.colors.white, 0.3),
            draw.lighten(color, 0.3) });
    }

    bool jobDidRender = false;

public:
    SampleComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
        , plugin(getPlugin("Sample"))
        , waveRect({ { 0, 20 },
              { props.size.w, (int)(props.size.h - 2 * 20) } })
        , wave(getNewPropsRect(props, waveRect))
    {
        browser = val(plugin.getValue("BROWSER"));
        startPosition = val(plugin.getValue("START"));
        endPosition = val(plugin.getValue("END"));
        sustainPosition = val(plugin.getValue("SUSTAIN"));
        sustainLength = val(plugin.getValue("SUSTAIN_LENGTH"));

        overlayYtop = position.y;
        overlayYbottom = position.y + size.h - 2;

        jobRendering = [this](unsigned long now) {
            // bool* active = (active*)plugin.data(2);
            // if (*active) {
            //     jobDidRender = true;
            //     renderNext();
            //     return;
            // }
            // if (jobDidRender) {
            //     jobDidRender = false;
            //     renderNext();
            // }
        };
    }

    void render()
    {
        if (lastBrowser != browser->get()) {
            if (textureSampleWaveform != NULL) {
                draw.destroyTexture(textureSampleWaveform);
                textureSampleWaveform = NULL;
            }
            lastBrowser = browser->get();
            textureSampleWaveform = draw.setTextureRenderer(size);
            draw.filledRect({ 0, 0 }, size, colors.background);
            // int count = *(uint64_t*)plugin.data(0);
            // wave.render((float*)plugin.data(1), count);
            draw.text({ 10, 5 }, browser->string().c_str(), colors.info, 12);
            draw.setMainRenderer();
        }
        draw.applyTexture(textureSampleWaveform, { position, size });
        // renderStartOverlay();
        // renderEndOverlay();
        // renderSustainOverlay();
        // renderSamples();
    }

    bool noteTriggered = false;
    void onMotion(MotionInterface& motion)
    {
        if (!noteTriggered) {
            noteTriggered = true;
            plugin.noteOn(60, 127);
        }
    }

    void onMotionRelease(MotionInterface& motion)
    {
        if (noteTriggered) {
            noteTriggered = false;
            plugin.noteOff(60, 0);
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

        if (strcmp(key, "SAMPLE_CURSOR_COLOR") == 0) {
            colors.sampleCursor = draw.getColor(value);
            return true;
        }

        return wave.config(key, value);
    }
};

#endif
