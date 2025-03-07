#pragma once

#include "helpers/range.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

class BaseGraphComponent : public Component {
protected:
    AudioPlugin* plugin = NULL;

    bool filled = true;
    bool outline = true;

    Color bgColor;

    Color fillColor;
    Color outlineColor;

    int waveformHeight = 30;

    void renderGraph()
    {
        std::vector<Point> relativePoints = getPoints();
        // printf("renderGraph %ld points\n", relativePoints.size());
        if (relativePoints.size() > 2) {
            for (auto& point : relativePoints) {
                point.y += relativePosition.y;
                point.x += relativePosition.x;
                // printf("[x %d y %d]\n", point.x, point.y);
            }
            // printf("draw\n");
            if (filled) {
                draw.filledPolygon(relativePoints, { fillColor });
            }
            if (outline) {
                draw.lines(relativePoints, { outlineColor });
            }
            // printf("draw end\n");
        }
    }

public:
    BaseGraphComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , fillColor(styles.colors.primary)
        , outlineColor(lighten(styles.colors.primary, 0.5))
    {
        waveformHeight = props.size.h;

        nlohmann::json& config = props.config;
        plugin = getPluginPtr(config, "audioPlugin", track);
        outline = config.value("outline", outline);
        filled = config.value("filled", filled);

        bgColor = draw.getColor(config["bgColor"], bgColor);
        fillColor = draw.getColor(config["fillColor"], fillColor);
        outlineColor = draw.getColor(config["outlineColor"], outlineColor);
    }

    virtual std::vector<Point> getPoints() = 0;

    void render() override
    {
        printf("render pos %dx%d size %dx%d\n", relativePosition.x, relativePosition.y, size.w, size.h);
        draw.filledRect(relativePosition, size, { bgColor });
        renderGraph();
    }

    bool isActive = true;
    void onGroupChanged(int8_t index) override
    {
        bool shouldActivate = false;
        if (group == index || group == -1) {
            shouldActivate = true;
        }
        if (shouldActivate != isActive) {
            isActive = shouldActivate;
            renderNext();
        }
    }
};
