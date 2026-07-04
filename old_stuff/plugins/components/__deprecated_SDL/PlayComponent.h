/** Description:
This document defines the blueprint for a graphical user interface element called the "Play Component." Its main purpose is to manage and visually represent the playback status of a media system, such as audio or video.

This component functions as a smart button that maintains three distinct visual states:

1.  **Playing:** Displayed using a triangle icon.
2.  **Paused:** Displayed using two vertical bars.
3.  **Stopped:** Displayed using a solid square icon.

The component handles user interaction and sends commands to the media system. A quick tap on the element toggles the playback state between Play and Pause. It also features a specialized "long press" function: if the user holds the component down for more than half a second, it interprets this as a definitive command to switch the system into the Stopped state.

In addition to managing the playback logic, the component handles its own visual appearance, drawing the appropriate icon and text label ("Playing," "Paused," or "Stopped"). Its colors, including the background, icon, and text, are fully customizable through configuration settings. Overall, it serves as the essential control element for starting, stopping, and pausing media within the application interface.

sha: b99c9e839fc96da7eb57a2b967b2595e8b735236d4d346680bd78ab0d58d2e45 
*/
#ifndef _UI_COMPONENT_PLAY_H_
#define _UI_COMPONENT_PLAY_H_

#include "../utils/color.h"
#include "../component.h"
#include <chrono>
#include <math.h>
#include <string>

/*md
## SDL Play

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/Play.png" />

Play component toggle play and pause, or stop on long press.
*/
class PlayComponent : public Component {
protected:
    bool playing = false;
    bool stopped = true;

    bool isPressed = false;
    unsigned long pressedTime = -1;

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
            darken({ 0x80, 0x80, 0x80, 255 }, 0.3),
            alpha(styles.colors.white, 0.4),
            // draw.getColor((char*)"#AAAAAA"),
            { 0xAA, 0xAA, 0xAA },
            // draw.getColor((char*)"#AAAAAA"),
            { 0xAA, 0xAA, 0xAA },
        };
        colors.border = darken(colors.icon, 0.6);

        iconPosition = { (int)(position.x + (size.w * 0.5) - iconSize.w * 0.5), (int)(position.y + (size.h * 0.5) - iconSize.h * 0.5) };
        labelPosition = { (int)(position.x + (size.w * 0.5)), (int)(iconPosition.y + size.h / 3.0 - 5) }; // for size.h / 3.0 - 5 see encoder2

        jobRendering = [this](unsigned long now) {
            if (!stopped && isPressed) {
                if (pressedTime == -1) {
                    pressedTime = now;
                } else if (now - pressedTime > 500) {
                    stopped = true;
                    audioPluginHandler->sendEvent(AudioEventType::STOP, -1);
                    renderNext();
                }
            }
        };
    }

    void render()
    {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            { colors.background });

        if (stopped) {
            // Stopped
            draw.filledRect(iconPosition, iconSize, { colors.icon });
            draw.rect(iconPosition, iconSize, { colors.border });
            draw.rect({ iconPosition.x + 1, iconPosition.y + 1 }, { iconSize.w - 2, iconSize.h - 2 }, { colors.border });
            draw.textCentered(labelPosition, "Stopped", 12, { colors.title });
        } else if (playing) {
            // Playing
            std::vector<Point> points = {
                iconPosition,
                { iconPosition.x + iconSize.w, (int)(iconPosition.y + iconSize.h * 0.5) },
                { iconPosition.x, iconPosition.y + iconSize.h },
                iconPosition,
            };
            draw.filledPolygon(points, { colors.icon });
            draw.lines(points, { colors.border });
            draw.textCentered(labelPosition, "Playing", 12, { colors.title });
        } else {
            // Paused
            draw.filledRect(iconPosition, { (int)(iconSize.w * 0.3), iconSize.h }, { colors.icon });
            draw.rect(iconPosition, { (int)(iconSize.w * 0.3), iconSize.h }, { colors.border });
            draw.filledRect({ iconPosition.x + (int)(iconSize.w * 0.7), iconPosition.y }, { (int)(iconSize.w * 0.3), iconSize.h }, { colors.icon });
            draw.rect({ iconPosition.x + (int)(iconSize.w * 0.7), iconPosition.y }, { (int)(iconSize.w * 0.3), iconSize.h }, { colors.border });
            draw.textCentered(labelPosition, "Paused", 12, { colors.title });
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
            colors.title = alpha(draw.getColor(value), 0.4);
            return true;
        }

        /*md - `ICON_COLOR: #AAAAAA` set value color */
        if (strcmp(key, "ICON_COLOR") == 0) {
            colors.icon = draw.getColor(value);
            colors.border = darken(colors.icon, 0.6);
            return true;
        }

        return false;
    }

    void* data(int id, void* userdata = NULL) override
    {
        if (id == 0) {
            isPressed = false;
            if (playing && stopped) {
                playing = false;
            } else if (!playing) {
                playing = true;
                stopped = false;
                audioPluginHandler->sendEvent(AudioEventType::START, -1);
            } else {
                playing = false;
                audioPluginHandler->sendEvent(AudioEventType::PAUSE, -1);
            }
            renderNext();
            return NULL;
        } else if (id == 1) {
            pressedTime = -1;
            isPressed = true;
        }
        return NULL;
    }

    void onMotion(MotionInterface& motion)
    {
        data(1);
    }

    void onMotionRelease(MotionInterface& motion)
    {
        data(0);
    }
};

#endif
