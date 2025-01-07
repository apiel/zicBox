#ifndef _UI_COMPONENT_FM_ALGO_H_
#define _UI_COMPONENT_FM_ALGO_H_

#include "plugins/components/component.h"
#include "utils/GroupColorComponent.h"

#include <string>

/*md
## FmAlgo

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo1.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo2.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo3.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo4.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo5.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo6.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo7.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo8.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo9.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo10.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo11.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/fm/algo12.png" />

Show FM algorithm and change them. The filled square are the carrier where audio is outputted and the not filled square are the operators modulating the frequency.
*/
class FmAlgoComponent : public GroupColorComponent {
protected:
    Color background;
    ToggleColor border;
    ToggleColor text;

    Size opSize = { 16, 16 };

    uint8_t fontSize = 8;
    int8_t encoderId = -1;
    ValueInterface* value = NULL;
    AudioPlugin* plugin;
    int8_t dataId = 0;

    int textMarginTop = 4;
    int textMarginLeft = 1;

public:
    FmAlgoComponent(ComponentInterface::Props props)
        : GroupColorComponent(props, { { "BORDER_COLOR", &border }, { "TEXT_COLOR", &text } })
        , background(styles.colors.background)
        , border({ 0x80, 0x80, 0x80 }, inactiveColorRatio)
        , text(styles.colors.primary, inactiveColorRatio)
    {
        if (size.w > size.h) {
            position.x += (size.w - size.h) * 0.5f;
            size.w = size.h;
        } else if (size.h > size.w) {
            position.y += (size.h - size.w) * 0.5f;
            size.h = size.w;
        }
    }

    void render()
    {
        if (updatePosition()) {
            // printf("updated position %d value=%f\n", dataId, value->get());
            draw.filledRect(relativePosition, size, { background });

            bool(*algo)[3] = (bool(*)[3])plugin->data(dataId);

            // draw modulation link
            if (algo[0][0]) // 1 to 2
            {
                Point start = { relativePosition.x + opSize.w, (int)(relativePosition.y + opSize.h * 0.5) };
                Point dest = { relativePosition.x + size.w - opSize.w, (int)(relativePosition.y + opSize.h * 0.5) };
                draw.line(start, dest, { text.color });

                draw.line({ dest.x - 4, dest.y - 4 }, dest, { text.color });
                draw.line({ dest.x - 4, dest.y + 4 }, dest, { text.color });
            }
            if (algo[1][1]) // 2 to 3
            {
                Point start = { relativePosition.x + size.w - opSize.w, relativePosition.y + opSize.h };
                Point dest = { relativePosition.x + opSize.w, relativePosition.y + size.h - opSize.h };
                draw.line(start, dest, { text.color });

                draw.line({ dest.x, dest.y - 5 }, dest, { text.color });
                draw.line({ dest.x + 5, dest.y }, dest, { text.color });
            }
            if (algo[2][2]) // 3 to 4
            {
                Point start = { relativePosition.x + opSize.w, (int)(relativePosition.y + size.h - opSize.h * 0.5) };
                Point dest = { relativePosition.x + size.w - opSize.w, (int)(relativePosition.y + size.h - opSize.h * 0.5) };
                draw.line(start, dest, { text.color });

                draw.line({ dest.x - 4, dest.y - 4 }, dest, { text.color });
                draw.line({ dest.x - 4, dest.y + 4 }, dest, { text.color });
            }
            if (algo[0][1]) // 1 to 3
            {
                Point start = { (int)(relativePosition.x + opSize.w * 0.5), relativePosition.y + opSize.h };
                Point dest = { (int)(relativePosition.x + opSize.w * 0.5), relativePosition.y + size.h - opSize.h };
                draw.line(start, dest, { text.color });

                draw.line({ dest.x - 4, dest.y - 4 }, dest, { text.color });
                draw.line({ dest.x + 4, dest.y - 4 }, dest, { text.color });
            }
            if (algo[0][2]) // 1 to 4
            {
                Point start = { relativePosition.x + opSize.w, relativePosition.y + opSize.h };
                Point dest = { relativePosition.x + size.w - opSize.w, relativePosition.y + size.h - opSize.h };
                draw.line(start, dest, { text.color });

                draw.line({ dest.x, dest.y - 5 }, dest, { text.color });
                draw.line({ dest.x - 5, dest.y }, dest, { text.color });
            }
            if (algo[1][2]) // 2 to 4
            {
                Point start = { (int)(relativePosition.x + size.w - opSize.w * 0.5), relativePosition.y + opSize.h };
                Point end = { (int)(relativePosition.x + size.w - opSize.w * 0.5), relativePosition.y + size.h - opSize.h };
                draw.line(start, end, { text.color });

                draw.line({ end.x - 4, end.y - 4 }, end, { text.color });
                draw.line({ end.x + 4, end.y - 4 }, end, { text.color });
            }

            // draw operators and carriers
            // 4 is always carrier
            draw.filledRect({ relativePosition.x + size.w - opSize.w, relativePosition.y + size.h - opSize.h }, opSize, { border.color }); // 4
            draw.textCentered({ (int)(relativePosition.x + size.w - opSize.w + opSize.w * 0.5 + textMarginLeft), relativePosition.y + size.h - opSize.h + textMarginTop }, "4", fontSize, { background });

            if (!algo[0][0] && !algo[0][1] && !algo[0][2]) { // 1
                draw.filledRect(position, opSize, { border.color }); // 1
                draw.textCentered({ (int)(relativePosition.x + opSize.w * 0.5 + textMarginLeft), relativePosition.y + textMarginTop }, "1", fontSize, { background });
            } else {
                draw.rect(position, opSize, { border.color }); // 1
                draw.textCentered({ (int)(relativePosition.x + opSize.w * 0.5 + textMarginLeft), relativePosition.y + textMarginTop }, "1", fontSize, { text.color });
            }

            if (!algo[1][0] && !algo[1][1] && !algo[1][2]) { // 2
                draw.filledRect({ relativePosition.x + size.w - opSize.w, relativePosition.y }, opSize, { border.color }); // 2
                draw.textCentered({ (int)(relativePosition.x + size.w - opSize.w + opSize.w * 0.5 + textMarginLeft), relativePosition.y + textMarginTop }, "2", fontSize, { background });
            } else {
                draw.rect({ relativePosition.x + size.w - opSize.w, relativePosition.y }, opSize, { border.color }); // 2
                draw.textCentered({ (int)(relativePosition.x + size.w - opSize.w + opSize.w * 0.5 + textMarginLeft), relativePosition.y + textMarginTop }, "2", fontSize, { text.color });
            }

            if (!algo[2][0] && !algo[2][1] && !algo[2][2]) { // 3
                draw.filledRect({ relativePosition.x, relativePosition.y + size.h - opSize.h }, opSize, { border.color }); // 3
                draw.textCentered({ (int)(relativePosition.x + opSize.w * 0.5 + textMarginLeft), relativePosition.y + size.h - opSize.h + textMarginTop }, "3", fontSize, { background });
            } else {
                draw.rect({ relativePosition.x, relativePosition.y + size.h - opSize.h }, opSize, { border.color }); // 3
                draw.textCentered({ (int)(relativePosition.x + opSize.w * 0.5 + textMarginLeft), relativePosition.y + size.h - opSize.h + textMarginTop }, "3", fontSize, { text.color });
            }
        }
    }

    bool config(char* key, char* params)
    {
        /*md - `BACKGROUND_COLOR: #333333` set the background color. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            background = draw.getColor(params);
            return true;
        }

        /*md - `VALUE: pluginName keyName` is used to set the value to control. */
        if (strcmp(key, "VALUE") == 0) {
            char* pluginName = strtok(params, " ");
            char* keyValue = strtok(NULL, " ");
            plugin = &getPlugin(pluginName, track);
            value = watch(plugin->getValue(keyValue));
            dataId = plugin->getDataId("ALGO");
            return true;
        }

        /*md - `ENCODER_ID: 0` is used to set the encoder id that will interract with this component. */
        if (strcmp(key, "ENCODER_ID") == 0) {
            encoderId = atoi(params);
            return true;
        }

        /*md - `DATA_ID: 0` is used to set the data id that will return the current algorithm layout. */
        if (strcmp(key, "DATA_ID") == 0) {
            dataId = plugin->getDataId(params);
            return true;
        }

        /*md - `TEXT_COLOR: #FFFFFF` set the text color. */
        /*md - `BORDER_COLOR: #888888` set the border color. */
        return GroupColorComponent::config(key, params);
    }

    void onEncoder(int id, int8_t direction)
    {
        if (value && isActive && id == encoderId) {
            value->increment(direction);
        }
    }
};

#endif
