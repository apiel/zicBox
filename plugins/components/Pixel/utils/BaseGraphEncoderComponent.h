#ifndef _UI_PIXEL_COMPONENT_BASE_GRAPH_ENCODER_H_
#define _UI_PIXEL_COMPONENT_BASE_GRAPH_ENCODER_H_

#include "./GroupColorComponent.h"
#include "helpers/range.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

class BaseGraphEncoderComponent : public GroupColorComponent {
public:
    struct Title {
        std::string text1;
        std::string text2 = "";
    };

protected:
    AudioPlugin* plugin = NULL;

    bool filled = true;
    bool outline = true;

    Color bgColor;

    ToggleColor fillColor;
    ToggleColor outlineColor;
    ToggleColor textColor1;
    ToggleColor textColor2;

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

    void renderTitles(std::vector<Title> titles, int y, int cellWidth)
    {
        int x = relativePosition.x + cellWidth * 0.5;
        for (int i = 0; i < titles.size(); i++) {
            int len = titles[i].text1.length() + titles[i].text2.length();
            int w = len * 8;
            int x2 = draw.text({ (int)(x + cellWidth * i - w * 0.5), y }, titles[i].text1, 8, { textColor1.color });
            draw.text({ x2, y }, titles[i].text2, 8, { textColor2.color });
        }
    }

    bool renderTitleOnTop = true;
    void renderTitles()
    {
        std::vector<Title> titles = getTitles();
        if (renderTitleOnTop) {
            int cellWidth = size.w / titles.size();
            renderTitles(titles, relativePosition.y, cellWidth);
        } else {
            // Calculate the midpoint, rounding up
            size_t midpoint = (titles.size() + 1) * 0.5; // +1 to round up
            std::vector<Title> top(titles.begin(), titles.begin() + midpoint);
            std::vector<Title> bottom(titles.begin() + midpoint, titles.end());
            int cellWidth = size.w / top.size();
            renderTitles(top, relativePosition.y, cellWidth);
            renderTitles(bottom, relativePosition.y + waveformHeight + 9 + 1, cellWidth);
        }
    }

    void updateWaveformHeight()
    {
        if (renderTitleOnTop) {
            waveformHeight = size.h - 9;
        } else {
            waveformHeight = size.h - 9 - 9;
        }
    }

public:
    BaseGraphEncoderComponent(ComponentInterface::Props props)
        : GroupColorComponent(props, { { "FILL_COLOR", &fillColor }, { "OUTLINE_COLOR", &outlineColor }, { "TEXT_COLOR1", &textColor1 }, { "TEXT_COLOR2", &textColor2 } })
        , bgColor(styles.colors.background)
        , textColor1(styles.colors.text, inactiveColorRatio)
        , textColor2(darken(styles.colors.text, 0.5), inactiveColorRatio)
        , fillColor(styles.colors.primary, inactiveColorRatio)
        , outlineColor(lighten(styles.colors.primary, 0.5), inactiveColorRatio)
    {
        updateColors();
        updateWaveformHeight();
    }
    virtual std::vector<Title> getTitles() = 0;
    virtual std::vector<Point> getPoints() = 0;

    void render() override
    {
        if (updatePosition()) {
            waveformY = relativePosition.y + 8;
            draw.filledRect(relativePosition, size, { bgColor });
            renderGraph();
            renderTitles();
        }
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

        if (strcmp(key, "RENDER_TITLE_ON_TOP") == 0) {
            renderTitleOnTop = strcmp(value, "true") == 0;
            updateWaveformHeight();
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
/*.md - `TEXT_COLOR1: color` is the color of the text1. */
/*.md - `TEXT_COLOR2: color` is the color of the text2. */
/*.md - `INACTIVE_COLOR_RATIO: 0.0 - 1.0` is the ratio of darkness for the inactive color (default: 0.5). */

#endif
