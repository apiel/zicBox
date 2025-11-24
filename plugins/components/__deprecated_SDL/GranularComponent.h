/** Description:
This C++ Header file acts as the blueprint for a highly specialized graphical user interface (GUI) element called the "Granular Component."

### Core Purpose

This component is designed to be the primary control panel for an associated "Granular" audio synthesizer or effect plugin. Granular synthesis is an audio technique that generates sound by taking tiny, rapid snippets of a source audio file (called "grains"). The component provides all the visual feedback and controls necessary to manipulate this process in real-time.

### Functionality Overview

1.  **Audio Parameter Control:** The component tracks and allows the user to adjust several key audio parameters managed by the underlying plugin, such as:
    *   **Browser:** Which audio sample is currently loaded.
    *   **Start & Grain Size:** Where in the sample the sound reading begins and the duration of the audio grains.
    *   **Spray & Density:** How much randomization is applied to the grain positions and how frequently new grains are created.

2.  **Visual Feedback:** The component is responsible for all drawing operations. It renders the waveform of the loaded audio sample and overlays colored markers and lines to indicate the current grain start position and the active range of the grain size and spray. It constantly checks if the underlying audio sample has changed and re-draws the waveform when necessary.

3.  **User Interaction:** It is highly interactive, designed to respond to mouse movements or touch input. The code maps dragging motions directly to the audio parameters, allowing a user to slide the grain start point or adjust the grain size simply by moving their finger or mouse across the screen. It also features clearly defined buttons for controls like "play," "spray" mode toggles, and "save" functionality.

In essence, this file defines a responsive, custom visual dashboard that gives the user precise, dynamic control over the complex settings of a granular audio engine.

sha: 1b0dc410cecbe1dc47980678e240ea9209ed64873387c95f3d84bfa1f721b448 
*/
#ifndef _UI_COMPONENT_GRANULAR_H_
#define _UI_COMPONENT_GRANULAR_H_

#include "../utils/inRect.h"
#include "../utils/color.h"
#include "./base/SamplePositionBaseComponent.h"
#include "./base/WaveBaseComponent.h"
#include "../component.h"
#include <string>

class GranularComponent : public Component {
protected:
    AudioPlugin& plugin;
    ValueInterface* browser = watch(getPlugin("Granular", track).getValue("BROWSER"));
    float lastBrowser = -1.0f;
    ValueInterface* start = watch(getPlugin("Granular", track).getValue("START"));
    ValueInterface* grainSize = watch(getPlugin("Granular", track).getValue("GRAIN_SIZE"));
    ValueInterface* spray = watch(getPlugin("Granular", track).getValue("SPRAY"));
    ValueInterface* density = watch(getPlugin("Granular", track).getValue("DENSITY"));

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
    SamplePositionBaseComponent samplePosition;

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
        samplePosition.render((std::vector<SamplePositionBaseComponent::SampleState>*)plugin.data(3));
    }

    void renderGrainStartRange()
    {
        int x = position.x + margin + (start->pct() * (textureSize.w));
        int w = (grainSize->pct() * (textureSize.w));
        if (x + w > position.x + textureSize.w) {
            w -= (x + w) - (position.x + textureSize.w);
        }
        draw.filledRect({ x, position.y + margin }, { w, textureSize.h }, { colors.start });

        // draw spray line
        int xSpray = x + (spray->pct() * w);
        draw.line({ xSpray, position.y + margin }, { xSpray, position.y + textureSize.h }, { colors.spray });
    }

    void renderToggle(Rect rect, const char* label, bool state)
    {
        auto [x, y] = rect.position;
        draw.rect(rect.position, rect.size, { colors.toggle });
        draw.text({ x + 5, y + 1 }, label, 10, { colors.info });
        if (state) {
            draw.filledRect({ x + 37, y + 2 }, { 11, 11 }, { colors.toggle });
        } else {
            draw.rect({ x + 37, y + 2 }, { 11, 11 }, { colors.toggle });
        }
    }

    void renderButton(Rect rect, const char* label, bool pressed)
    {
        auto [x, y] = rect.position;
        draw.rect(rect.position, rect.size, { colors.toggle });
        if (pressed) {
            draw.filledRect({ x + 2, y + 2 }, { rect.size.w - 4, rect.size.h - 4 }, { colors.toggle });
        }
        draw.textCentered({ (int)(x + rect.size.w * 0.5), y + 1 }, label, 10, { colors.info });
    }

    struct Colors {
        Color background;
        Color info;
        Color start;
        Color toggle;
        Color spray;
        // Color startEndOverlay;
        // Color startEndOverlayEdge;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({
            darken(color, 0.75),
            darken(color, 0.3),
            { 0xd1, 0xe3, 0xff, 0x1e },
            alpha(color, 0.2),
            alpha(color, 0.2),
            // alpha(darken(color, 0.80), 0.6),
            // alpha(darken(color, 0.90), 0.3),
        });
    }

    const int margin;

public:
    GranularComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.primary))
        , margin(styles.margin)
        , plugin(getPlugin("Granular", track))
        , wave(getNewPropsRect(props, { { 0, 20 }, { props.size.w - 2 * styles.margin, (int)(props.size.h - 2 * (20 + styles.margin)) } }))
        , samplePosition(getNewPropsRect(props, { { props.position.x, props.position.y + 20 }, { props.size.w, (int)(props.size.h - 2 * 20) } }))
    {
        textureSize = { size.w - 2 * margin, size.h - 2 * margin };
        sprayToggleRect = { { position.x + size.w - 70, position.y + size.h - 30 }, { 50, 15 } };
        playToggleRect = { { position.x + size.w - 130, position.y + size.h - 30 }, { 50, 15 } };
        saveRect = { { position.x + size.w - 190, position.y + size.h - 30 }, { 50, 15 } };

        jobRendering = [this](unsigned long now) {
            if (samplePosition.shouldRender(plugin.data(4))) {
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
            draw.filledRect({ 0, 0 }, { textureSize.w, textureSize.h }, { colors.background });
            wave.render((float*)plugin.data(1), *(uint64_t*)plugin.data(0));
            draw.text({ 10, 5 }, browser->string().c_str(), 12, { colors.info });
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
                plugin.noteOn(48, 1.0f);
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

        return samplePosition.config(key, value) || wave.config(key, value);
    }
};

#endif
