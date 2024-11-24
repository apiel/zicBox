#ifndef _UI_COMPONENT_FM_ALGO_H_
#define _UI_COMPONENT_FM_ALGO_H_

#include "../component.h"
#include <string>

/*md
## SDL FmAlgo

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/fm/algo1.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/fm/algo2.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/fm/algo3.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/fm/algo4.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/fm/algo5.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/fm/algo6.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/fm/algo7.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/fm/algo8.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/fm/algo9.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/fm/algo10.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/fm/algo11.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/fm/algo12.png" />

Show FM algorithm and change them. The filled square are the carrier where audio is outputted and the not filled square are the operators modulating the frequency.
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
    int8_t encoderId = -1;
    ValueInterface* value = NULL;
    AudioPlugin* plugin;
    int8_t dataId = 10;

public:
    FmAlgoComponent(ComponentInterface::Props props)
        : Component(props)
        , colors({ props.draw.styles.colors.background,
              { 0x80, 0x80, 0x80, 255 },
              props.draw.styles.colors.white })
    {
    }

    void render()
    {
        draw.filledRect(position, size, { colors.background });

        bool(*algo)[3] = (bool(*)[3])plugin->data(dataId);

        // draw modulation link
        if (algo[0][0]) // 1 to 2
        {
            Point start = { position.x + opSize.w, (int)(position.y + opSize.h * 0.5) };
            Point dest = { position.x + size.w - opSize.w, (int)(position.y + opSize.h * 0.5) };
            draw.line(start, dest, { colors.text });

            draw.line({ dest.x - 4, dest.y - 4 }, dest, { colors.text });
            draw.line({ dest.x - 4, dest.y + 4 }, dest, { colors.text });
        }
        if (algo[1][1]) // 2 to 3
        {
            Point start = { position.x + size.w - opSize.w, position.y + opSize.h };
            Point dest = { position.x + opSize.w, position.y + size.h - opSize.h };
            draw.line(start, dest, { colors.text });

            draw.line({ dest.x, dest.y - 5 }, dest, { colors.text });
            draw.line({ dest.x + 5, dest.y }, dest, { colors.text });
        }
        if (algo[2][2]) // 3 to 4
        {
            Point start = { position.x + opSize.w, (int)(position.y + size.h - opSize.h * 0.5) };
            Point dest = { position.x + size.w - opSize.w, (int)(position.y + size.h - opSize.h * 0.5) };
            draw.line(start, dest, { colors.text });

            draw.line({ dest.x - 4, dest.y - 4 }, dest, { colors.text });
            draw.line({ dest.x - 4, dest.y + 4 }, dest, { colors.text });
        }
        if (algo[0][1]) // 1 to 3
        {
            Point start = { (int)(position.x + opSize.w * 0.5), position.y + opSize.h };
            Point dest = { (int)(position.x + opSize.w * 0.5), position.y + size.h - opSize.h };
            draw.line(start, dest, { colors.text });

            draw.line({ dest.x - 4, dest.y - 4 }, dest, { colors.text });
            draw.line({ dest.x + 4, dest.y - 4 }, dest, { colors.text });
        }
        if (algo[0][2]) // 1 to 4
        {
            Point start = { position.x + opSize.w, position.y + opSize.h };
            Point dest = { position.x + size.w - opSize.w, position.y + size.h - opSize.h };
            draw.line(start, dest, { colors.text });

            draw.line({ dest.x, dest.y - 5 }, dest, { colors.text });
            draw.line({ dest.x - 5, dest.y }, dest, { colors.text });
        }
        if (algo[1][2]) // 2 to 4
        {
            Point start = { (int)(position.x + size.w - opSize.w * 0.5), position.y + opSize.h };
            Point end = { (int)(position.x + size.w - opSize.w * 0.5), position.y + size.h - opSize.h };
            draw.line(start, end, { colors.text });

            draw.line({ end.x - 4, end.y - 4 }, end, { colors.text });
            draw.line({ end.x + 4, end.y - 4 }, end, { colors.text });
        }

        // draw operators and carriers
        // 4 is always carrier
        draw.filledRect({ position.x + size.w - opSize.w, position.y + size.h - opSize.h }, opSize, { colors.border }); // 4
        draw.textCentered({ (int)(position.x + size.w - opSize.w + opSize.w * 0.5), position.y + size.h - opSize.h + 2 }, "4", fontSize, { colors.background });

        if (!algo[0][0] && !algo[0][1] && !algo[0][2]) { // 1
            draw.filledRect(position, opSize, { colors.border }); // 1
            draw.textCentered({ (int)(position.x + opSize.w * 0.5), position.y + 2 }, "1", fontSize, { colors.background });
        } else {
            draw.rect(position, opSize, { colors.border }); // 1
            draw.textCentered({ (int)(position.x + opSize.w * 0.5), position.y + 2 }, "1", fontSize, { colors.text });
        }

        if (!algo[1][0] && !algo[1][1] && !algo[1][2]) { // 2
            draw.filledRect({ position.x + size.w - opSize.w, position.y }, opSize, { colors.border }); // 2
            draw.textCentered({ (int)(position.x + size.w - opSize.w + opSize.w * 0.5), position.y + 2 }, "2", fontSize, { colors.background });
        } else {
            draw.rect({ position.x + size.w - opSize.w, position.y }, opSize, { colors.border }); // 2
            draw.textCentered({ (int)(position.x + size.w - opSize.w + opSize.w * 0.5), position.y + 2 }, "2", fontSize, { colors.text });
        }

        if (!algo[2][0] && !algo[2][1] && !algo[2][2]) { // 3
            draw.filledRect({ position.x, position.y + size.h - opSize.h }, opSize, { colors.border }); // 3
            draw.textCentered({ (int)(position.x + opSize.w * 0.5), position.y + size.h - opSize.h + 2 }, "3", fontSize, { colors.background });
        } else {
            draw.rect({ position.x, position.y + size.h - opSize.h }, opSize, { colors.border }); // 3
            draw.textCentered({ (int)(position.x + opSize.w * 0.5), position.y + size.h - opSize.h + 2 }, "3", fontSize, { colors.text });
        }
    }

    bool config(char* key, char* params)
    {
        /*md - `COLOR: #FFFFFF` set the text color. */
        if (strcmp(key, "COLOR") == 0) {
            colors.text = draw.getColor(params);
            return true;
        }

        /*md - `BACKGROUND_COLOR: #333333` set the background color. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            colors.background = draw.getColor(params);
            return true;
        }

        /*md - `VALUE: pluginName keyName` is used to set the value to control. */
        if (strcmp(key, "VALUE") == 0) {
            char* pluginName = strtok(params, " ");
            char* keyValue = strtok(NULL, " ");
            plugin = &getPlugin(pluginName, track);
            value = watch(plugin->getValue(keyValue));
            return true;
        }

        /*md - `ENCODER_ID: 0` is used to set the encoder id that will interract with this component. */
        if (strcmp(key, "ENCODER_ID") == 0) {
            encoderId = atoi(params);
            return true;
        }

        /*md - `DATA_ID: 0` is used to set the data id that will return the current algorithm layout. */
        if (strcmp(key, "DATA_ID") == 0) {
            dataId = atoi(params);
            return true;
        }

        return false;
    }

    void onEncoder(int id, int8_t direction)
    {
        if (id == encoderId) {
            value->increment(direction);
        }
    }
};

#endif
