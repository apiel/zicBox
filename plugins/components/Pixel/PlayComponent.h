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

    uint8_t mode = 0;

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

        /// The mode of the component.
        std::string modeStr = config.value("mode", "DEFAULT"); //eg: "DEFAULT"
        if (modeStr == "DEFAULT") {
            mode = 0;
        } else if (modeStr == "TOGGLE_PLAY_PAUSE") {
            mode = 1;
        } else if (modeStr == "TOGGLE_PLAY_STOP") {
            mode = 2;
        } else if (modeStr == "TOGGLE_RECORD_STOP") {
            mode = 3;
        }

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

        if (mode == 1) { // TOGGLE_PLAY_PAUSE
            if (playing) {
                renderPaused();
            } else {
                renderPlaying();
            }
        } else if (mode == 2) { // TOGGLE_PLAY_STOP
            if (playing) {
                renderStopped();
            } else {
                renderPlaying();
            }
        } else if (mode == 3) { // TOGGLE_RECORD_STOP
            if (recording) {
                renderStopped();
            } else {
                renderRecording();
            }
        } else { // DEFAULT
            if (stopped) {
                renderStopped();
            } else if (recording) {
                renderRecording();
            } else if (playing) {
                renderPlaying();
            } else {
                renderPaused();
            }
        }
    }

    void renderStopped()
    {
        draw.filledRect(iconPosition, iconSize, { color });
    }

    void renderRecording()
    {
        draw.filledCircle({ iconPosition.x + (int)(iconSize.w * 0.5), iconPosition.y + (int)(iconSize.h * 0.5) }, (int)(iconSize.w * 0.5), { recColor });
    }

    void renderPlaying()
    {
        std::vector<Point> points = {
            iconPosition,
            { iconPosition.x + iconSize.w, (int)(iconPosition.y + iconSize.h * 0.5) },
            { iconPosition.x, iconPosition.y + iconSize.h },
            iconPosition,
        };
        draw.filledPolygon(points, { playColor });
    }

    void renderPaused()
    {
        draw.filledRect(iconPosition, { (int)(iconSize.w * 0.3), iconSize.h }, { color });
        draw.filledRect({ iconPosition.x + (int)(iconSize.w * 0.7), iconPosition.y }, { (int)(iconSize.w * 0.3), iconSize.h }, { color });
    }
};
