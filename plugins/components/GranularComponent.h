#ifndef _UI_COMPONENT_GRANULAR_H_
#define _UI_COMPONENT_GRANULAR_H_

#include "../../helpers/inRect.h"
#include "./base/WaveBaseComponent.h"
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
    bool sprayControllerOn = true;

    Rect playToggleRect;
    bool playControllerOn = true;

    Rect saveRect;
    bool savePressed = false;
    bool saved = false;

    WaveBaseComponent wave;

    // void renderSampleStart()
    // {
    //     int w = textureSize.w * sampleStartPosition->pct();
    //     draw.filledRect({ position.x, position.y }, { w, size.h }, colors.startOverlay);
    //     draw.line(
    //         { position.x + w, position.y }, { position.x + w, position.y + textureSize.h }, colors.startEndOverlayEdge);
    // }

    // void renderSampleEnd()
    // {
    //     int w = textureSize.w * sampleEndPosition->pct();
    //     draw.filledRect({ position.x + w, position.y }, { textureSize.w - w, textureSize.h }, colors.startOverlay);
    //     draw.line(
    //         { position.x + w, position.y },
    //         { position.x + w, position.y + textureSize.h },
    //         colors.startEndOverlayEdge);
    // }

    void renderGrains()
    {
        struct GrainState {
            int index;
            float position;
            float release;
        };
        std::vector<GrainState>* grainStates = (std::vector<GrainState>*)plugin.data(3);
        int spacing = 13;
        int totalHeight = grainStates->size() * spacing;
        int marginTop = 5;
        if (totalHeight < textureSize.h) {
            marginTop = (textureSize.h - totalHeight) / 2;
        }
        int y = position.y + marginTop;
        for (auto& grain : *grainStates) {
            int x = position.x + grain.position * textureSize.w;
            Color color = colors.grain;
            if (grain.release != 1.0) {
                color = draw.alpha(color, grain.release);
            }
            draw.filledRect({ x, y + (grain.index * spacing % (textureSize.h - 10 )) }, { 4, 4 }, color);
        }
    }

    void renderGrainStartRange()
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

    void renderToggle(Rect rect, const char* label, bool state)
    {
        auto [x, y] = rect.position;
        draw.rect(rect.position, rect.size, colors.toggle);
        draw.text({ x + 5, y + 1 }, label, colors.info, 10);
        if (state) {
            draw.filledRect({ x + 37, y + 2 }, { 11, 11 }, colors.toggle);
        } else {
            draw.rect({ x + 37, y + 2 }, { 11, 11 }, colors.toggle);
        }
    }

    void renderButton(Rect rect, const char* label, bool pressed)
    {
        auto [x, y] = rect.position;
        draw.rect(rect.position, rect.size, colors.toggle);
        if (pressed) {
            draw.filledRect({ x + 2, y + 2 }, { rect.size.w - 4, rect.size.h - 4 }, colors.toggle);
        }
        draw.textCentered({ (int)(x + rect.size.w * 0.5), y + 1 }, label, colors.info, 10);
    }

    struct Colors {
        Color background;
        Color info;
        Color start;
        Color toggle;
        Color spray;
        // Color startEndOverlay;
        // Color startEndOverlayEdge;
        Color grain;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.75),
            draw.darken(color, 0.3),
            styles.colors.overlay,
            draw.alpha(color, 0.2),
            draw.alpha(color, 0.2),
            // draw.alpha(draw.darken(color, 0.80), 0.6),
            // draw.alpha(draw.darken(color, 0.90), 0.3),
            draw.lighten(draw.getColor((char*)"#9dfe86"), 0.3) });
    }

    const int margin;

    bool jobDidRender = false;

public:
    GranularComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
        , margin(styles.margin)
        , plugin(getPlugin("Granular"))
        , wave(getNewPropsRect(props,
              { { 0, 20 },
                  { props.size.w - 2 * styles.margin, (int)(props.size.h - 2 * (20 + styles.margin)) } }))
    {
        textureSize = { size.w - 2 * margin, size.h - 2 * margin };
        sprayToggleRect = { { position.x + size.w - 70, position.y + size.h - 30 }, { 50, 15 } };
        playToggleRect = { { position.x + size.w - 130, position.y + size.h - 30 }, { 50, 15 } };
        saveRect = { { position.x + size.w - 190, position.y + size.h - 30 }, { 50, 15 } };

        jobRendering = [this](unsigned long now) {
            if (plugin.data(4) != NULL) {
                jobDidRender = true;
                renderNext();
            } else if (jobDidRender) {
                jobDidRender = false;
                renderNext();
            }
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
            textureSampleWaveform = draw.setTextureRenderer(textureSize);
            draw.filledRect({ 0, 0 }, { textureSize.w, textureSize.h }, colors.background);
            wave.render((float*)plugin.data(1), *(uint64_t*)plugin.data(0));
            draw.text({ 10, 5 }, browser->string().c_str(), colors.info, 12);
            draw.setMainRenderer();
        }
        draw.applyTexture(textureSampleWaveform, { { position.x + margin, position.y + margin }, textureSize });

        renderGrainStartRange();
        renderToggle(sprayToggleRect, "spray", sprayControllerOn);
        renderToggle(playToggleRect, "play", playControllerOn);
        renderButton(saveRect, saved ? "saved" : "save", savePressed);
        renderGrains();
        if (saved) {
            saved = false;
        }
    }

    float startOrigin = 0.0;
    float sizeOrigin = 0.0;
    void onMotion(MotionInterface& motion)
    {
        if (inRect(sprayToggleRect, motion.origin)) {
            return;
        }

        if (inRect(playToggleRect, motion.origin)) {
            return;
        }

        if (inRect(saveRect, motion.origin)) {
            savePressed = true;
            renderNext();
            return;
        }

        if (motion1 == NULL) {
            if (playControllerOn) {
                plugin.noteOn(48, 127);
            }
            motion1 = &motion;
            startOrigin = start->pct();
        }

        if (motion1 == &motion) {
            float x = startOrigin + (motion.position.x - motion.origin.x) / (float)(textureSize.w);
            if (x - start->pct() > 0.01 || start->pct() - x > 0.01) {
                start->set(x * 100.0f);
            }

            if (sprayControllerOn) {
                float rangeMargin = 40;
                float y = 1.0 - (motion.position.y - position.y - margin - rangeMargin) / (float)(textureSize.h - (rangeMargin * 2));
                if (y - spray->pct() > 0.01 || spray->pct() - y > 0.01) {
                    spray->set(y * 100.0f);
                }
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

        if (inRect(sprayToggleRect, motion.origin) && inRect(sprayToggleRect, motion.position)) {
            sprayControllerOn = !sprayControllerOn;
            renderNext();
            return;
        }

        if (inRect(playToggleRect, motion.origin) && inRect(playToggleRect, motion.position)) {
            playControllerOn = !playControllerOn;
            renderNext();
            return;
        }

        if (inRect(saveRect, motion.origin)) {
            plugin.data(2);
            savePressed = false;
            saved = true;
            renderNext();
            return;
        }

        if (motion1 == &motion) {
            motion1 = NULL;
            if (playControllerOn) {
                plugin.noteOff(48, 0);
            }
        }
        if (motion2 == &motion) {
            motion2 = NULL;
        }
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
            wave.setColors(draw.getColor(value));
            return true;
        }

        if (strcmp(key, "SPRAY_CONTROLLER") == 0) {
            sprayControllerOn = (strcmp(value, "ON") == 0);
            return true;
        }

        return wave.config(key, value);
    }
};

#endif
