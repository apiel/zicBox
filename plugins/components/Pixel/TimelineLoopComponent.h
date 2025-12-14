#pragma once

#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"
#include "plugins/audio/utils/Timeline.h"

class TimelineLoopComponent : public Component {
protected:
    // ---- Timeline access ----
    AudioPlugin* timelinePlugin = nullptr;
    Timeline* timeline = nullptr;

    // ---- View sync ----
    int32_t viewStepStart = 0;
    int32_t viewStepCount = 128;
    int stepPixel = 4;

    uint8_t viewStepStartContextId = 0;

    // ---- Loop values ----
    ValueInterface* loopStartVal = nullptr;
    ValueInterface* loopLengthVal = nullptr;

    // ---- Colors ----
    Color background;
    Color loopColor;
    Color gridColor;

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
    {
        nlohmann::json& config = props.config;

        // ---- Timeline plugin ----
        timelinePlugin = getPluginPtr(config, "timelinePlugin", track);
        if (timelinePlugin) {
            timeline = (Timeline*)timelinePlugin->data(
                timelinePlugin->getDataId(config.value("timelineDataId", "TIMELINE"))
            );

            loopStartVal = watch(timelinePlugin->getValue("LOOP_START"));
            loopLengthVal = watch(timelinePlugin->getValue("LOOP_LENGTH"));
        }

        // ---- Context sync ----
        viewStepStartContextId = config.value("viewStepStartContextId", viewStepStartContextId);

        // ---- Colors (reuse timeline style) ----
        background = draw.getColor(config["background"], styles.colors.background);
        gridColor = draw.getColor(config["gridColor"], lighten(styles.colors.background, 0.4));
        loopColor = draw.getColor(config["loopColor"], alpha(styles.colors.white, 0.9f));
    }

    void resize() override
    {
        viewStepCount = size.w / stepPixel;
    }

    void onContext(uint8_t index, float value) override
    {
        if (index == viewStepStartContextId) {
            if ((int)value != viewStepStart) {
                viewStepStart = (int)value;
                renderNext();
            }
        }
        Component::onContext(index, value);
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { background });

        // ---- Grid (same feel as timeline) ----
        for (int i = 0; i <= viewStepCount; ++i) {
            int step = viewStepStart + i;
            int x = relativePosition.x + i * stepPixel;

            Color col = (step % 16 == 0)
                ? gridColor
                : darken(gridColor, 0.5f);

            draw.line(
                { x, relativePosition.y },
                { x, relativePosition.y + size.h },
                { col }
            );
        }

        if (!loopStartVal || !loopLengthVal)
            return;

        int loopStart = (int)loopStartVal->get();
        int loopLength = (int)loopLengthVal->get();

        // ---- No loop → draw nothing ----
        if (loopLength <= 0)
            return;

        int loopEnd = loopStart + loopLength;

        int viewEnd = viewStepStart + viewStepCount;

        // ---- Visibility test ----
        if (loopEnd < viewStepStart || loopStart > viewEnd)
            return;

        int visibleStart = std::max(loopStart, viewStepStart);
        int visibleEnd = std::min(loopEnd, viewEnd);

        int xA = relativePosition.x + (visibleStart - viewStepStart) * stepPixel;
        int xB = relativePosition.x + (visibleEnd - viewStepStart) * stepPixel;

        int yMid = relativePosition.y + size.h / 2;

        // ---- Dotted loop range ----
        drawDottedLine(xA, xB, yMid, loopColor, 3);

        // ---- Loop start marker ----
        int xStart = relativePosition.x + (loopStart - viewStepStart) * stepPixel;
        draw.line(
            { xStart, relativePosition.y },
            { xStart, relativePosition.y + size.h },
            { loopColor }
        );

        // ---- Loop end marker ----
        int xEnd = relativePosition.x + (loopEnd - viewStepStart) * stepPixel;
        draw.line(
            { xEnd, relativePosition.y },
            { xEnd, relativePosition.y + size.h },
            { loopColor }
        );
    }
};
