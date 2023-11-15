#ifndef _UI_COMPONENT_GRANULAR_H_
#define _UI_COMPONENT_GRANULAR_H_

#include "component.h"
#include <string>

class GranularComponent : public Component {
protected:
    AudioPlugin& plugin;
    ValueInterface* browser = val(getPlugin("Granular").getValue("BROWSER"));
    float lastBrowser = -1.0f;
    ValueInterface* start = val(getPlugin("Granular").getValue("START"));
    ValueInterface* grainSize = val(getPlugin("Granular").getValue("GRAIN_SIZE"));
    ValueInterface* spray = val(getPlugin("Granular").getValue("SPRAY"));
    ValueInterface* density = val(getPlugin("Granular").getValue("DENSITY"));

    MotionInterface* motion1 = NULL;
    MotionInterface* motion2 = NULL;

    Size textureSize;
    void* textureSampleWaveform = NULL;

    Rect sprayToggleRect;

    void renderSampleWaveform()
    {
        if (textureSampleWaveform == NULL) {
            lastBrowser = browser->get();
            textureSampleWaveform = draw.setTextureRenderer(textureSize);

            draw.filledRect({ 0, 0 }, { textureSize.w, textureSize.h }, colors.background);

            uint64_t* samplesCount = (uint64_t*)plugin.data(0);
            float* bufferSamples = (float*)plugin.data(1);
            int h = textureSize.h * 0.5f;
            for (int i = 0; i < *samplesCount; i++) {
                int x = margin + (i * textureSize.w / *samplesCount);
                int graphH = bufferSamples[i] * h;
                if (graphH) {
                    int y1 = margin + (h - graphH);
                    int y2 = margin + (h + graphH);
                    draw.line({ x, y1 }, { x, y2 }, colors.samples);
                } else {
                    draw.pixel({ x, margin + h }, colors.samples);
                }
            }

            draw.text({ 10, 5 }, browser->string().c_str(), colors.info, 12);
            draw.setMainRenderer();
        }
        draw.applyTexture(textureSampleWaveform, { { position.x + margin, position.y + margin }, textureSize });
    }

    void renderStartRange()
    {
        int x = position.x + margin + (start->pct() * (textureSize.w));
        int w = (grainSize->pct() * (textureSize.w));
        if (x + w > position.x + textureSize.w) {
            w -= (x + w) - (position.x + textureSize.w);
        }
        draw.filledRect({ x, position.y + margin }, { w, textureSize.h }, colors.start);

        // draw spray line
        int xSpray = x + (spray->pct() * w);
        draw.line({ xSpray, position.y + margin }, { xSpray, position.y + textureSize.h }, colors.spray);
    }

    bool sprayControllerOn = true;

    void renderSprayToggle()
    {
        auto [x, y] = sprayToggleRect.position;
        draw.rect(sprayToggleRect.position, sprayToggleRect.size, colors.spray);
        draw.text({ x + 5, y }, "spray", colors.info, 10);
        if (sprayControllerOn) {
            draw.filledRect({ x + 37, y + 2 }, { 11, 11 }, colors.spray);
        } else {
            draw.filledRect({ x + 37, y + 2 }, { 11, 11 }, colors.spray);
        }
    }

    struct Colors {
        Color background;
        Color info;
        Color samples;
        Color start;
        Color spray;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.75),
            draw.darken(color, 0.3),
            draw.darken(color, 0.2),
            styles.colors.overlay,
            draw.alpha(color, 0.2) });
    }

    const int margin;

public:
    GranularComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
        , margin(styles.margin)
        , plugin(getPlugin("Granular"))
    {
        textureSize = { size.w - 2 * margin, size.h - 2 * margin };
        sprayToggleRect = { { position.x + size.w - 70, position.y + size.h - 30 }, { 50, 15 } };
    }

    void render()
    {
        if (lastBrowser != browser->get()) {
            if (textureSampleWaveform != NULL) {
                draw.destroyTexture(textureSampleWaveform);
                textureSampleWaveform = NULL;
            }
        }
        renderSampleWaveform();
        renderStartRange();
        renderSprayToggle();
    }

    float startOrigin = 0.0;
    float sizeOrigin = 0.0;
    void onMotion(MotionInterface& motion)
    {
        if (motion1 == NULL) {
            plugin.noteOn(48, 127);
            motion1 = &motion;
            startOrigin = start->pct();
        }

        if (motion1 == &motion) {
            float x = startOrigin + (motion.position.x - motion.origin.x) / (float)(textureSize.w);
            if (x - start->pct() > 0.01 || start->pct() - x > 0.01) {
                start->set(x * 100.0f);
            }

            float rangeMargin = 40;
            float y = 1.0 - (motion.position.y - position.y - margin - rangeMargin) / (float)(textureSize.h - (rangeMargin * 2));
            if (y - spray->pct() > 0.01 || spray->pct() - y > 0.01) {
                spray->set(y * 100.0f);
            }
        } else if (motion2 == NULL) {
            motion2 = &motion;
            sizeOrigin = grainSize->pct();
        }

        if (motion2 == &motion) {
            // float x = startOrigin + (motion.position.x - motion.origin.x) / (float)(textureSize.w);
            float x = sizeOrigin + (motion.position.x - motion.origin.x) / (float)(textureSize.w);
            if (x - grainSize->pct() > 0.01 || grainSize->pct() - x > 0.01) {
                grainSize->set(x * 100.0f);
            }

            float rangeMargin = 40;
            float y = 1.0 - (motion.position.y - position.y - margin - rangeMargin) / (float)(textureSize.h - (rangeMargin * 2));
            if (y - density->pct() > 0.01 || density->pct() - y > 0.01) {
                density->set(y * 100.0f);
            }
        }
    }

    void onMotionRelease(MotionInterface& motion)
    {
        Component::onMotionRelease(motion);
        if (motion1 == &motion) {
            motion1 = NULL;
            plugin.noteOff(48, 0);
        }
        if (motion2 == &motion) {
            motion2 = NULL;
        }
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
