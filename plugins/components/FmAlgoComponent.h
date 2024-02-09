#ifndef _UI_COMPONENT_FM_ALGO_H_
#define _UI_COMPONENT_FM_ALGO_H_

#include "component.h"
#include <string>

/*md
## FmAlgoComponent

Show a FM algorithm and change them.
*/
class FmAlgoComponent : public Component {
protected:
    struct Colors {
        Color background;
        Color border;
        Color text;
    } colors;

    Size opSize = { 16, 16 };

    uint8_t fontSize = 10;

public:
    FmAlgoComponent(ComponentInterface::Props props)
        : Component(props)
        , colors({ props.draw.styles.colors.background,
              props.draw.styles.colors.grey,
              props.draw.styles.colors.white })
    {
    }

    void render()
    {
        draw.filledRect(position, size, colors.background);
        draw.rect(position, opSize, colors.border);
        draw.textCentered({ (int)(position.x + opSize.w * 0.5), position.y + 2 }, "1", colors.text, fontSize);
        draw.rect({ position.x + size.w - opSize.w, position.y }, opSize, colors.border);
        draw.textCentered({ (int)(position.x + size.w - opSize.w + opSize.w * 0.5), position.y + 2 }, "2", colors.text, fontSize);
        draw.rect({ position.x, position.y + size.h - opSize.h }, opSize, colors.border);
        draw.textCentered({ (int)(position.x + opSize.w * 0.5), position.y + size.h - opSize.h + 2 }, "3", colors.text, fontSize);
        draw.rect({ position.x + size.w - opSize.w, position.y + size.h - opSize.h }, opSize, colors.border);
        draw.textCentered({ (int)(position.x + size.w - opSize.w + opSize.w * 0.5), position.y + size.h - opSize.h + 2 }, "4", colors.text, fontSize);
    }

    bool config(char* key, char* value)
    {
        /*md - `COLOR: #FFFFFF` set the text color. */
        if (strcmp(key, "COLOR") == 0) {
            colors.text = draw.getColor(value);
            return true;
        }

        /*md - `BACKGROUND_COLOR: #333333` set the background color. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            colors.background = draw.getColor(value);
            return true;
        }

        return false;
    }
};

#endif
