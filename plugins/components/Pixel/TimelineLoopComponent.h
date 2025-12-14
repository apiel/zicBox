#pragma once

#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"
#include "plugins/audio/utils/Timeline.h"
#include "plugins/components/Pixel/TimelineCore.h"

class TimelineLoopComponent : public Component {
protected:
    // ---- Timeline access ----
    // Timeline* timeline = nullptr;

    uint8_t viewStepStartContextId = 0;

    // ---- Loop values ----
    ValueInterface* loopStartVal = nullptr;
    ValueInterface* loopLengthVal = nullptr;

    // ---- Colors ----
    Color background;
    Color loopColor;

    TimelineCore core;

protected:
    void drawDottedLine(int xStart, int xEnd, int y, Color color, int spacing = 2)
    {
        if (xEnd < xStart)
            std::swap(xStart, xEnd);

        for (int x = xStart; x <= xEnd; x += spacing) {
            draw.pixel({ x, y }, { color });
        }
    }

public:
    TimelineLoopComponent(ComponentInterface::Props props)
        : Component(props)
        , core(props)
    {
        nlohmann::json& config = props.config;

        // ---- Timeline plugin ----
        AudioPlugin* timelinePlugin = getPluginPtr(config, "timelinePlugin", track);
        if (timelinePlugin) {
            // timeline = (Timeline*)timelinePlugin->data(
            //     timelinePlugin->getDataId(config.value("timelineDataId", "TIMELINE"))
            // );

            loopStartVal = watch(timelinePlugin->getValue("LOOP_START"));
            loopLengthVal = watch(timelinePlugin->getValue("LOOP_LENGTH"));
        }

        // ---- Context sync ----
        viewStepStartContextId = config.value("viewStepStartContextId", viewStepStartContextId);

        // ---- Colors (reuse timeline style) ----
        background = draw.getColor(config["background"], styles.colors.background);
        loopColor = draw.getColor(config["loopColor"], alpha(styles.colors.white, 0.9f));
    }

    void resize() override
    {
        core.resize(size);
    }

    void onContext(uint8_t index, float value) override
    {
        if (index == viewStepStartContextId) {
            if ((int)value != core.viewStepStart) {
                core.viewStepStart = (int)value;
                renderNext();
            }
        }
        Component::onContext(index, value);
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { background });

        core.renderGrid(relativePosition);

        if (!loopStartVal || !loopLengthVal)
            return;

        int loopStart = (int)loopStartVal->get();
        int loopLength = (int)loopLengthVal->get();

        // ---- No loop → draw nothing ----
        if (loopLength <= 0)
            return;

        int loopEnd = loopStart + loopLength;

        int viewEnd = core.viewStepStart + core.viewStepCount;

        // ---- Visibility test ----
        if (loopEnd < core.viewStepStart || loopStart > viewEnd)
            return;

        int visibleStart = std::max(loopStart, core.viewStepStart);
        int visibleEnd = std::min(loopEnd, viewEnd);

        int xA = relativePosition.x + (visibleStart - core.viewStepStart) * core.stepPixel;
        int xB = relativePosition.x + (visibleEnd - core.viewStepStart) * core.stepPixel;

        int yMid = relativePosition.y + size.h / 2;

        // ---- Dotted loop range ----
        drawDottedLine(xA, xB, yMid, loopColor, 3);

        // ---- Loop start marker ----
        int xStart = relativePosition.x + (loopStart - core.viewStepStart) * core.stepPixel;
        draw.line(
            { xStart, relativePosition.y },
            { xStart, relativePosition.y + size.h },
            { loopColor }
        );

        // ---- Loop end marker ----
        int xEnd = relativePosition.x + (loopEnd - core.viewStepStart) * core.stepPixel;
        draw.line(
            { xEnd, relativePosition.y },
            { xEnd, relativePosition.y + size.h },
            { loopColor }
        );
    }
};
