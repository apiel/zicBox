#pragma once

#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <string>

/*md
## Play

Play components show the playing state.
*/

class PlayComponent : public Component {
protected:
    Color bgColor;
    Color color;
    Color playColor;
    Color recColor;

    Size iconSize = { 20, 20 };
    Point iconPosition;

    bool playing = false;
    bool stopped = true;
    bool recording = false;

public:
    PlayComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , color(styles.colors.primary)
        , playColor(rgb(113, 153, 95))
        , recColor(rgb(255, 154, 154))
    {
        /*md md_config:Rect */
        nlohmann::json& config = props.config;

        /// The color of the rectangle.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// The color of the icon.
        color = draw.getColor(config["color"], color); //eg: "#FFFFFF"

        /// The color of the play icon.
        playColor = draw.getColor(config["playColor"], playColor); //eg: "#00FF00"

        /// The color of the record icon.
        recColor = draw.getColor(config["recColor"], recColor); //eg: "#FF0000"

        /*md md_config_end */

        jobRendering = [this](unsigned long now) {
            bool playingState = audioPluginHandler->isPlaying();
            bool stoppedState = audioPluginHandler->isStopped();
            bool recordingState = audioPluginHandler->isRecording();
            if (playing != playingState || stopped != stoppedState || recording != recordingState) {
                playing = playingState;
                stopped = stoppedState;
                recording = recordingState;
                renderNext();
            }
        };

        resize();
    }

    void resize() override
    {
        iconPosition = { (int)(relativePosition.x + (size.w * 0.5) - iconSize.w * 0.5), (int)(relativePosition.y + (size.h * 0.5) - iconSize.h * 0.5) };
        int s = (size.w < size.h ? size.w : size.h) - 1;
        iconSize = { s, s };
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });

        if (stopped) {
            // Stopped
            draw.filledRect(iconPosition, iconSize, { color });
        } else if (recording) {
            // Recording
            draw.filledCircle({ iconPosition.x + (int)(iconSize.w * 0.5), iconPosition.y + (int)(iconSize.h * 0.5) }, (int)(iconSize.w * 0.5), { recColor });
        } else if (playing) {
            // Playing
            std::vector<Point> points = {
                iconPosition,
                { iconPosition.x + iconSize.w, (int)(iconPosition.y + iconSize.h * 0.5) },
                { iconPosition.x, iconPosition.y + iconSize.h },
                iconPosition,
            };
            draw.filledPolygon(points, { playColor });
        } else {
            // Paused
            draw.filledRect(iconPosition, { (int)(iconSize.w * 0.3), iconSize.h }, { color });
            draw.filledRect({ iconPosition.x + (int)(iconSize.w * 0.7), iconPosition.y }, { (int)(iconSize.w * 0.3), iconSize.h }, { color });
        }
    }
};
