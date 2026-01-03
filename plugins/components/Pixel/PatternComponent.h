#pragma once

#include "plugins/components/component.h"
#include "plugins/audio/stepInterface.h"
#include "plugins/components/utils/color.h"

#include <string>

class PatternComponent : public Component {
protected:
    // ---- Pattern data ----
    AudioPlugin* plugin = nullptr;
    std::vector<Step>* steps = nullptr;
    uint16_t* stepCount = nullptr;

    // ---- Rendering ----
    Color bgColor;
    Color patternColor;

    // ---- Fixed MIDI range (same as TimelineComponent) ----
    const int MIDI_MIN = 12;   // C0
    const int MIDI_MAX = 119;  // B9
    const float MIDI_RANGE = (float)(MIDI_MAX - MIDI_MIN);

public:
    PatternComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , patternColor(styles.colors.primary)
    {
        /*md md_config:Pattern */
        nlohmann::json& config = props.config;

        /// Sequencer audio plugin
        plugin = getPluginPtr(config, "audioPlugin", track);

        if (plugin) {
            steps = (std::vector<Step>*)plugin->data(
                plugin->getDataId(config.value("stepsDataId", "STEPS"))
            );

            stepCount = (uint16_t*)plugin->data(
                plugin->getDataId(config.value("stepCountDataId", "STEP_COUNT"))
            );
        }

        /// Background color
        bgColor = draw.getColor(config["bgColor"], bgColor);

        /// Pattern (note) color
        patternColor = draw.getColor(config["color"], patternColor);

        /*md md_config_end */
    }

    void render() override
    {
        // ---- Background ----
        draw.filledRect(relativePosition, size, { bgColor });

        if (!steps || !stepCount || *stepCount == 0)
            return;

        const float stepPixel = (float)size.w / (float)(*stepCount);

        // ---- Render notes (Timeline-style) ----
        for (const auto& step : *steps) {
            if (!step.enabled || step.len <= 0)
                continue;

            int stStart = step.position;
            int stEnd   = step.position + step.len;

            // ---- Horizontal clipping ----
            if (stEnd <= 0 || stStart >= *stepCount)
                continue;

            int clippedStart = std::max(0, stStart);
            int clippedEnd   = std::min((int)*stepCount, stEnd);

            int px = relativePosition.x + (int)(clippedStart * stepPixel);
            int pw = std::max(1, (int)((clippedEnd - clippedStart) * stepPixel));

            // ---- Vertical placement (MIDI → Y) ----
            float noteNorm = (float)(step.note - MIDI_MIN) / MIDI_RANGE;
            noteNorm = std::clamp(noteNorm, 0.0f, 1.0f);

            int py = relativePosition.y + size.h
                     - (int)(noteNorm * (size.h - 1));

            int ph = 3; // same visual thickness as timeline

            // ---- Draw note ----
            draw.filledRect({ px, py - ph }, { pw, ph }, { patternColor });
            draw.line(
                { px, py - ph },
                { px, py - 1 },
                { darken(patternColor, 0.2f) }
            );
        }
    }
};
