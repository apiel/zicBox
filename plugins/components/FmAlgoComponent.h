#ifndef _UI_COMPONENT_FM_ALGO_H_
#define _UI_COMPONENT_FM_ALGO_H_

#include "component.h"
#include <string>

/*md
## FmAlgoComponent

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/fm/algo1.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/fm/algo2.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/fm/algo3.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/fm/algo4.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/fm/algo5.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/fm/algo6.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/fm/algo7.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/fm/algo8.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/fm/algo9.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/fm/algo10.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/fm/algo11.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/fm/algo12.png" />

Show a FM algorithm and change them. The larger square operator are the carrier where audio is outputted.
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
              props.draw.styles.colors.grey,
              props.draw.styles.colors.white })
    {
    }

    void render()
    {
        draw.filledRect(position, size, colors.background);
        draw.rect(position, opSize, colors.border); // 1
        draw.textCentered({ (int)(position.x + opSize.w * 0.5), position.y + 2 }, "1", colors.text, fontSize);
        draw.rect({ position.x + size.w - opSize.w, position.y }, opSize, colors.border); // 2
        draw.textCentered({ (int)(position.x + size.w - opSize.w + opSize.w * 0.5), position.y + 2 }, "2", colors.text, fontSize);
        draw.rect({ position.x, position.y + size.h - opSize.h }, opSize, colors.border); // 3
        draw.textCentered({ (int)(position.x + opSize.w * 0.5), position.y + size.h - opSize.h + 2 }, "3", colors.text, fontSize);

        draw.rect({ position.x + size.w - opSize.w, position.y + size.h - opSize.h }, opSize, colors.border); // 4
        draw.textCentered({ (int)(position.x + size.w - opSize.w + opSize.w * 0.5), position.y + size.h - opSize.h + 2 }, "4", colors.text, fontSize);

        bool(*algo)[3] = (bool(*)[3])plugin->data(dataId);

        // for (int i = 0; i < 3; i++) {
        //     for (int j = 0; j < 3; j++) {
        //         if (algo[i][j]) {
        //             printf("...................op %d to %d\n", i + 1, j + 2);
        //         }
        //     }
        // }

        // draw modulation link
        if (algo[0][0]) // 1 to 2
        {
            draw.line({ position.x + opSize.w, (int)(position.y + opSize.h * 0.5) }, { position.x + size.w - opSize.w, (int)(position.y + opSize.h * 0.5) }, colors.text);
        }
        if (algo[1][1]) // 2 to 3
        {
            draw.line({ position.x + size.w - opSize.w, position.y + opSize.h }, { position.x + opSize.w, position.y + size.h - opSize.h }, colors.text);
        }
        if (algo[2][2]) // 3 to 4
        {
            draw.line({ position.x + size.w - opSize.w, (int)(position.y + size.h - opSize.h * 0.5) }, { position.x + opSize.w, (int)(position.y + size.h - opSize.h * 0.5) }, colors.text);
        }
        if (algo[0][1]) // 1 to 3
        {
            draw.line({ (int)(position.x + opSize.w * 0.5), position.y + opSize.h }, { (int)(position.x + opSize.w * 0.5), position.y + size.h - opSize.h }, colors.text);
        }
        if (algo[0][2]) // 1 to 4
        {
            draw.line({ position.x + opSize.w, position.y + opSize.h }, { position.x + size.w - opSize.w, position.y + size.h - opSize.h }, colors.text);
        }
        if (algo[1][2]) // 2 to 4
        {
            draw.line({ (int)(position.x + size.w - opSize.w * 0.5), position.y + opSize.h }, { (int)(position.x + size.w - opSize.w * 0.5), position.y + size.h - opSize.h }, colors.text);
        }

        // draw carrier
        Size carrierSize = { opSize.w - 2, opSize.h - 2 };
        draw.rect({ position.x + size.w - opSize.w + 1, position.y + size.h - opSize.h + 1 }, carrierSize, colors.border); // 4 is always carrier
        if (!algo[0][0] && !algo[0][1] && !algo[0][2]) { // 1
            draw.rect({ position.x + 1, position.y + 1 }, carrierSize, colors.border);
        }
        if (!algo[1][0] && !algo[1][1] && !algo[1][2]) { // 2
            draw.rect({ position.x + size.w - opSize.w + 1, position.y + 1 }, carrierSize, colors.border);
        }
        if (!algo[2][0] && !algo[2][1] && !algo[2][2]) { // 3
            draw.rect({ position.x + 1, position.y + size.h - opSize.h + 1 }, carrierSize, colors.border);
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
};

#endif
