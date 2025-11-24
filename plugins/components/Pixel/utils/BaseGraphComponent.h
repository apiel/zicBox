/** Description:
This code defines the core blueprint for a visual component—a customizable container designed specifically for drawing graphs, waveforms, or any sequential visual data within an application, likely an audio plugin interface.

This component handles all the visual styling and rendering mechanics, while leaving the task of generating the actual data points to its specialized children.

### How it Works

1.  **Foundation and Customization:** The component acts as a canvas that manages its appearance. It initializes default colors (background, primary fill, and outline) but allows these to be overridden by external configuration settings. It also controls whether the graph should be displayed as a solid shape (filled) or just the border (outline).
2.  **Connecting Data:** It establishes a link to a specific `AudioPlugin` source to ensure the graph can visualize data coming from the right place in the audio system.
3.  **Drawing Process:** When the component is asked to draw itself, it first paints a solid background rectangle. Then, it follows a simple process to draw the graph:
    *   It requests the raw data (the sequence of points defining the graph's shape) from the specialized class that extends this blueprint.
    *   It translates these raw points to the correct location on the screen.
    *   It draws the resulting shape: first drawing the solid area using the fill color (if enabled), and then drawing the border lines using the outline color (if enabled).

In essence, this class provides the framework and the paintbrush, while relying on derived components to supply the specific picture to be drawn.

sha: 41e4fc76ff7a5c794ac92b33445a66a731afac738d1018d7ff83f11ff9235380 
*/
#pragma once

#include "helpers/clamp.h"
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
        draw.filledRect(relativePosition, size, { bgColor });
        renderGraph();
    }
};
