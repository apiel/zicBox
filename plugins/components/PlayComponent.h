#ifndef _UI_COMPONENT_PLAY_H_
#define _UI_COMPONENT_PLAY_H_

#include "component.h"
#include <chrono>
#include <math.h>
#include <string>

/*md
## Play

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Play.png" />

Play component toggle play and pause, or stop on long press.
*/
class PlayComponent : public Component {
protected:
    bool playing = false;
    bool stopped = true;

    std::chrono::_V2::system_clock::duration pressedTime = std::chrono::system_clock::now().time_since_epoch();

    Size iconSize = { 20, 20 };
    Point iconPosition;
    Point labelPosition;

    struct Colors {
        Color background;
        Color id;
        Color title;
        Color icon;
        Color border;
    } colors;

    const int margin;

public:
    PlayComponent(ComponentInterface::Props props)
        : Component(props)
        , margin(styles.margin)
    {
        colors = {
            styles.colors.background,
            draw.darken(styles.colors.grey, 0.3),
            draw.alpha(styles.colors.white, 0.4),
            draw.getColor((char*)"#AAAAAA"),
            draw.getColor((char*)"#AAAAAA"),
        };
        colors.border = draw.darken(colors.icon, 0.6);

        iconPosition = { (int)(position.x + (size.w * 0.5) - iconSize.w * 0.5), (int)(position.y + (size.h * 0.5) - iconSize.h * 0.5) };
        labelPosition = { (int)(position.x + (size.w * 0.5)), (int)(iconPosition.y + size.h / 3.0 - 5) }; // for size.h / 3.0 - 5 see encoder2
    }

    void render()
    {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            colors.background);

        if (stopped) {
            // Stopped
            draw.filledRect(iconPosition, iconSize, colors.icon);
            draw.rect(iconPosition, iconSize, colors.border);
            draw.rect({ iconPosition.x + 1, iconPosition.y + 1 }, { iconSize.w - 2, iconSize.h - 2 }, colors.border);
            draw.textCentered(labelPosition, "Stopped", colors.title, 12);
        } else if (playing) {
            // Playing
            std::vector<Point> points = {
                iconPosition,
                { iconPosition.x + iconSize.w, (int)(iconPosition.y + iconSize.h * 0.5) },
                { iconPosition.x, iconPosition.y + iconSize.h }
            };
            draw.filledPolygon(points, colors.icon);
            draw.polygon(points, colors.border);
            draw.textCentered(labelPosition, "Playing", colors.title, 12);
        } else {
            // Paused
            draw.filledRect(iconPosition, { (int)(iconSize.w * 0.3), iconSize.h }, colors.icon);
            draw.rect(iconPosition, { (int)(iconSize.w * 0.3), iconSize.h }, colors.border);
            draw.filledRect({ iconPosition.x + (int)(iconSize.w * 0.7), iconPosition.y }, { (int)(iconSize.w * 0.3), iconSize.h }, colors.icon);
            draw.rect({ iconPosition.x + (int)(iconSize.w * 0.7), iconPosition.y }, { (int)(iconSize.w * 0.3), iconSize.h }, colors.border);
            draw.textCentered(labelPosition, "Paused", colors.title, 12);
        }
    }

    bool config(char* key, char* value)
    {
        /*md - `BACKGROUND_COLOR: #000000` set background color */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            colors.background = draw.getColor(value);
            return true;
        }

        /*md - `TEXT_COLOR: #FFFFFF` set text color */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            colors.title = draw.alpha(draw.getColor(value), 0.4);
            return true;
        }

        /*md - `ICON_COLOR: #AAAAAA` set value color */
        if (strcmp(key, "ICON_COLOR") == 0) {
            colors.icon = draw.getColor(value);
            colors.border = draw.darken(colors.icon, 0.6);
            return true;
        }

        return false;
    }

    void* data(int id, void* userdata = NULL) override
    {
        if (id == 0) {
            std::chrono::_V2::system_clock::duration duration = std::chrono::system_clock::now().time_since_epoch() - pressedTime;
            if (duration.count() > 500000000) {
                playing = false;
                stopped = true;
                sendAudioEvent(AudioEventType::STOP);
            } else if (!playing) {
                playing = true;
                stopped = false;
                sendAudioEvent(AudioEventType::START);
            } else {
                playing = false;
                sendAudioEvent(AudioEventType::PAUSE);
            }
            renderNext();
            return NULL;
        } else if (id == 1) {
            pressedTime = std::chrono::system_clock::now().time_since_epoch();
            return NULL;
        }
        return NULL;
    }
};

#endif
