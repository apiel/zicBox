#ifndef _UI_PIXEL_COMPONENT_BASE_GRAPH_H_
#define _UI_PIXEL_COMPONENT_BASE_GRAPH_H_

#include "./GroupColorComponent.h"
#include "helpers/range.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

class BaseGraphComponent : public GroupColorComponent {
protected:
    AudioPlugin* plugin = NULL;

    bool filled = true;
    bool outline = true;

    Color bgColor;

    ToggleColor fillColor;
    ToggleColor outlineColor;

    int waveformHeight = 30;
    int waveformY = 0;

    void renderGraph()
    {
        std::vector<Point> relativePoints = getPoints();
        if (relativePoints.size() > 2) {
            float halfHeight = waveformHeight * 0.5;

            for (auto& point : relativePoints) {
                point.y += waveformY;
                point.x += relativePosition.x;
            }
            if (filled) {
                draw.filledPolygon(relativePoints, { fillColor.color });
            }
            if (outline) {
                draw.lines(relativePoints, { outlineColor.color });
            }
        }
    }

public:
    BaseGraphComponent(ComponentInterface::Props props)
        : GroupColorComponent(props, { { "FILL_COLOR", &fillColor }, { "OUTLINE_COLOR", &outlineColor } })
        , bgColor(styles.colors.background)
        , fillColor(styles.colors.primary, inactiveColorRatio)
        , outlineColor(lighten(styles.colors.primary, 0.5), inactiveColorRatio)
    {
        updateColors();
        waveformHeight = props.size.h;
    }
    virtual std::vector<Point> getPoints() = 0;

    void render() override
    {
        waveformY = relativePosition.y + 8;
        draw.filledRect(relativePosition, size, { bgColor });
        renderGraph();
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "PLUGIN") == 0) {
            plugin = &getPlugin(value, track);
            return true;
        }

        if (strcmp(key, "OUTLINE") == 0) {
            outline = strcmp(value, "true") == 0;
            return true;
        }

        if (strcmp(key, "FILLED") == 0) {
            filled = strcmp(value, "true") == 0;
            return true;
        }

        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(value);
            return true;
        }

        return GroupColorComponent::config(key, value);
    }
};

/*.md **Config**: */
/*.md - `PLUGIN: plugin_name` set plugin target */
/*.md - `OUTLINE: true/false` is if the envelop should be outlined (default: true). */
/*.md - `FILLED: true/false` is if the envelop should be filled (default: true). */
/*.md - `BACKGROUND_COLOR: color` is the background color of the component. */
/*.md - `FILL_COLOR: color` is the color of the envelop. */
/*.md - `OUTLINE_COLOR: color` is the color of the envelop outline. */
/*.md - `INACTIVE_COLOR_RATIO: 0.0 - 1.0` is the ratio of darkness for the inactive color (default: 0.5). */

#endif
