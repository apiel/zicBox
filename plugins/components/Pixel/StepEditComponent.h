#ifndef _UI_PIXEL_COMPONENT_STEP_EDIT_H_
#define _UI_PIXEL_COMPONENT_STEP_EDIT_H_

#include "../component.h"
#include "../utils/color.h"
#include "utils/GroupColorComponent.h"

/*md
## StepEdit

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/stepEdit.png" />

StepEdit component is used to edit a step value.
*/

// [note, velocity, len] [probabilty, condition, motion]

class StepEditComponent : public GroupColorComponent {
    Color bgColor;
    ToggleColor text;
    ToggleColor text2;
    ToggleColor barBackground;
    ToggleColor bar;

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
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { bgColor });

            int x = draw.text({ relativePosition.x + 2, relativePosition.y }, "C", 16, { text.color });
            draw.text({ x - 2, relativePosition.y + 6 }, "4#", 8, { text.color });

            float velocity = 0.5;
            draw.filledRect({ relativePosition.x + 32, relativePosition.y },
                { size.w - 40, 3 }, { barBackground.color });
            draw.filledRect({ relativePosition.x + 32, relativePosition.y },
                { (int)((size.w - 40) * velocity), 3 }, { bar.color });

            draw.textRight({ relativePosition.x + size.w - 4, relativePosition.y + 6 }, "1/32", 8, { text2.color });
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
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
