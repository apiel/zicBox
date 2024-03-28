#ifndef _UI_COMPONENT_RECT_H_
#define _UI_COMPONENT_RECT_H_

#include "component.h"
#include <string>

/*md
## NavBarComponent

Navigation bar components draw a scrollable menu to be used with buttons/keys.
*/
class NavBarComponent : public Component {
protected:
    Size arrowSize = { 10, 10 };
    Size itemSize = { 0, 0 };
    int fontSize = 10;
    int textTopMargin = 2;
    int itemsMarginRight = 0;

    void drawItem(int x, int y, std::string text)
    {
        draw.filledRect({ x, y }, itemSize, colors.background);
        draw.textCentered({ x + itemSize.w / 2, y + textTopMargin }, text, colors.font, fontSize);
    }

    struct Colors {
        Color background;
        Color font;
    } colors;

public:
    NavBarComponent(ComponentInterface::Props props)
        : Component(props)
    {
        colors.background = props.draw.styles.colors.background;
        colors.font = props.draw.styles.colors.grey;
        arrowSize = { props.size.h, props.size.h };
        itemSize = { (props.size.w - 2 * (arrowSize.w + 1)) / 3 - 1, props.size.h };
        textTopMargin = (arrowSize.h - fontSize) / 2;
        itemsMarginRight = (size.w - (((itemSize.w + 1) * 3) + ((arrowSize.w + 1) * 2))) / 2;
    }

    void render()
    {
        draw.filledRect(position, arrowSize, colors.background);
        draw.aalines({ { position.x + arrowSize.w - 3, position.y + 1 },
                         { position.x + 3, position.y + (int)(arrowSize.h * 0.5) },
                         { position.x + arrowSize.w - 3, position.y + arrowSize.h - 2 } },
            colors.font);

        draw.filledRect({ position.x + size.w - arrowSize.w, position.y }, arrowSize, colors.background);
        draw.aalines({ { position.x + size.w - arrowSize.w + 3, position.y + 1 },
                         { position.x + size.w - 3, position.y + (int)(arrowSize.h * 0.5) },
                         { position.x + size.w - arrowSize.w + 3, position.y + arrowSize.h - 2 } },
            colors.font);

        for (int i = 0; i < 3; i++) {
            drawItem(position.x + (arrowSize.w + 1) + i * (itemSize.w + 1) + itemsMarginRight, position.y, "Item " + std::to_string(i));
        }
    }

    bool config(char* key, char* value)
    {
        /*md - `BACKGROUND: #333333` set the background color and activate background drawing. */
        if (strcmp(key, "BACKGROUND") == 0) {
            colors.background = draw.getColor(value);
            return true;
        }

        return false;
    }
};

#endif
