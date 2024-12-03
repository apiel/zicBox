#ifndef _UI_PIXEL_COMPONENT_STEP_EDIT_H_
#define _UI_PIXEL_COMPONENT_STEP_EDIT_H_

#include "helpers/midiNote.h"
#include "plugins/audio/stepInterface.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"
#include "utils/GroupColorComponent.h"

/*md
## StepEdit

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/stepEdit.png" />

StepEdit component is used to edit a step value.
*/

// [note, velocity, len] [probabilty, condition, motion]
// For the note length, we should show the note symbol

class StepEditComponent : public GroupColorComponent {
protected:
    AudioPlugin* plugin = NULL;
    Step* step;

    Color bgColor;
    ToggleColor text;
    ToggleColor text2;
    ToggleColor barBackground;
    ToggleColor bar;

    uint8_t dataId = -1;
    uint8_t stepIndex = -1;

public:
    StepEditComponent(ComponentInterface::Props props)
        : GroupColorComponent(props, { { "TEXT_COLOR", &text }, { "TEXT2_COLOR", &text2 }, { "BAR_BACKGROUND_COLOR", &barBackground }, { "BAR_COLOR", &bar } })
        , bgColor(styles.colors.background)
        , text(styles.colors.text, inactiveColorRatio)
        , text2(darken(styles.colors.text, 0.5), inactiveColorRatio)
        , barBackground(darken(styles.colors.tertiary, 0.5), inactiveColorRatio)
        , bar(styles.colors.tertiary, inactiveColorRatio)
    {
        updateColors();
    }

    void render() override
    {
        if (updatePosition() && step) {
            draw.filledRect(relativePosition, size, { bgColor });

            const char* note = MIDI_NOTES_STR[step->note];
            const char noteLetter[2] = { note[0], '\0' };
            const char* noteSuffix = note + 1;
            int x = draw.text({ relativePosition.x + 2, relativePosition.y }, noteLetter, 16, { text.color });
            draw.text({ x - 2, relativePosition.y + 6 }, noteSuffix, 8, { text.color });

            float centerX = relativePosition.x + size.w * 0.5;

            int barWidth = size.w * 0.40;
            int barX = (int)(centerX - barWidth * 0.5);
            draw.filledRect({ barX, relativePosition.y },
                { barWidth, 3 }, { barBackground.color });
            draw.filledRect({ barX, relativePosition.y },
                { (int)(barWidth * step->velocity), 3 }, { bar.color });

            draw.textRight({ relativePosition.x + size.w - 4, relativePosition.y + 6 }, "1/32", 8, { text2.color });
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        /*md - `DATA: plugin_name data_id step_index` set plugin target */
        if (strcmp(key, "DATA") == 0) {
            plugin = &getPlugin(strtok(value, " "), track);
            dataId = atoi(strtok(NULL, " "));
            stepIndex = atoi(strtok(NULL, " "));
            step = (Step*)plugin->data(dataId, &stepIndex);
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(value);
            return true;
        }

        /*md - `TEXT_COLOR: color` is the color of the text. */
        /*md - `BAR_COLOR: color` is the color of the velocity bar. */
        /*md - `BAR_BACKGROUND_COLOR: color` is the color of the velocity bar background. */
        return GroupColorComponent::config(key, value);
    }
};

#endif
