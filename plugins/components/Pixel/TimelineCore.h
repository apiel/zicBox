#pragma once

#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

class TimelineCore {
protected:
    ComponentInterface::Props props;
    DrawInterface& draw;
    Styles& styles;

    Color gridColor;
    Color loopColor;

    Size size;

public:
    Color barColor;

    int stepPixel = 4;
    int32_t viewStepStart = 0; // first visible step
    int32_t viewStepCount = 128; // steps visible on screen (auto-filled on resize)

    TimelineCore(ComponentInterface::Props props)
        : props(props)
        , draw(props.view->draw)
        , styles(props.view->draw.styles)
        , size(props.size)
    {
        nlohmann::json& config = props.config;

        // Colors
        gridColor = draw.getColor(config["gridColor"], lighten(styles.colors.background, 0.4));
        barColor = draw.getColor(config["barColor"], lighten(styles.colors.background, 1.0));
        loopColor = draw.getColor(config["loopColor"], alpha(styles.colors.white, 0.9f));
    }

    void resize(Size size)
    {
        this->size = size;
        viewStepCount = size.w / stepPixel;
    }

    void renderGrid(Point relativePosition)
    {
        for (int i = 0; i <= viewStepCount; i++) {
            int step = viewStepStart + i;
            int x = relativePosition.x + i * stepPixel;
            Color col = (step % 16 == 0)
                ? barColor
                : (step % 4 == 0) ? gridColor
                                  : darken(gridColor, 0.5);

            draw.line({ x, relativePosition.y }, { x, relativePosition.y + size.h }, { col });
        }
    }
};
