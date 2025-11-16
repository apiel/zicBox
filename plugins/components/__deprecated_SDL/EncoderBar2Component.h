#ifndef _UI_COMPONENT_ENCODER_BAR_2_H_
#define _UI_COMPONENT_ENCODER_BAR_2_H_

#include "../component.h"
#include "../utils/color.h"
#include <math.h>
#include <string>

/*md
## SDL EncoderBar2

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/EncoderBar2.png" />

EncoderBar2 is used to display current audio plugin value for a given parameter.
*/
class EncoderBar2Component : public Component {
protected:
    std::string label;
    char labelBuffer[32];
    uint8_t type = 0;

    int fontValueSize = 14;
    int fontUnitSize = 9;
    int twoSideMargin = 2;

    bool showValue = true;
    bool showUnit = true;

    int8_t encoderId = -1;
    uint8_t valueFloatPrecision = 0;

    ValueInterface* value = NULL;

    int xCenter = 0;

    int titleSize = 9;

    void renderValue()
    {
        if (value->props().type == VALUE_STRING) {
            draw.textCentered({ position.x + size.w - 4, position.y + 1 }, value->string(), fontValueSize, { colors.value });
        } else {
            std::string valStr = std::to_string(value->get());
            valStr = valStr.substr(0, valStr.find(".") + valueFloatPrecision + (valueFloatPrecision > 0 ? 1 : 0));

            int x = position.x + size.w - 4;
            if (showUnit && value->props().unit.length() > 0) {
                x = draw.textRight({ x, position.y + 1 }, value->props().unit, fontUnitSize, { colors.unit }) - 2;
            }
            void* fontBold = draw.getFont(styles.font.bold);
            draw.textRight({ x, position.y + 1 }, valStr.c_str(), fontValueSize, { colors.value, fontBold });
        }
    }

    void renderTwoSidedValue()
    {
        int val = value->get();
        void* fontBold = draw.getFont(styles.font.bold);
        draw.text({ position.x + 4, position.y + 1 }, std::to_string((int)value->props().max - val).c_str(),
            fontValueSize, { colors.value, fontBold });
        draw.textRight({ position.x + size.w - 4, position.y + 1 }, std::to_string(val).c_str(),
            fontValueSize, { colors.value, fontBold });
    }

    void renderEncoder()
    {
        if (value->props().type == VALUE_CENTERED) {
            if (value->pct() > 0.5) {
                draw.filledRect(
                    { xCenter, position.y + margin },
                    { (int)((size.w - 2 * margin) * (value->pct() - 0.5)), size.h - 2 * margin },
                    { colors.backgroundProgress });
            } else {
                int w = (size.w - 2 * margin) * (0.5 - value->pct());
                draw.filledRect(
                    { xCenter - w, position.y + margin },
                    { w, size.h - 2 * margin },
                    { colors.backgroundProgress });
            }
            draw.textCentered({ xCenter, position.y + 1 }, label, titleSize, { colors.title });
        } else {
            draw.filledRect(
                { position.x + margin, position.y + margin },
                { (int)((size.w - 2 * margin) * value->pct()), size.h - 2 * margin },
                { colors.backgroundProgress });
            draw.text({ position.x + 4, position.y + 1 }, label, titleSize, { colors.title });
        }

        if (showValue) {
            if (value->props().type == VALUE_CENTERED && type == 1) {
                renderTwoSidedValue();
            } else {
                renderValue();
            }
        }
    }

    struct Colors {
        Color background;
        Color backgroundProgress;
        Color title;
        Color value;
        Color unit;
    } colors;

    const int margin;

public:
    EncoderBar2Component(ComponentInterface::Props props)
        : Component(props)
        , margin(styles.margin)
    {
        colors = {
            styles.colors.background,
            lighten(styles.colors.background, 0.2),
            darken({ 0x80, 0x80, 0x80, 255 }, 0.3),
            alpha(styles.colors.white, 0.6),
            alpha(styles.colors.white, 0.4),
        };

        xCenter = position.x + (size.w * 0.5);
    }

    void render()
    {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            { colors.background });

        if (value != NULL) {
            renderEncoder();
        }
    }

    bool config(char* key, char* params)
    {
        /*md - `VALUE: pluginName keyName` is used to set the value to control */
        if (strcmp(key, "VALUE") == 0) {
            char* pluginName = strtok(params, " ");
            char* keyValue = strtok(NULL, " ");
            value = watch(getPlugin(pluginName, track).getValue(keyValue));
            if (value != NULL) {
                valueFloatPrecision = value->props().floatingPoint;
                if (label.empty()) {
                    label = value->label();
                }
            }
            return true;
        }

        /*md - `ENCODER_ID: 0` is used to set the encoder id that will interract with this component */
        if (strcmp(key, "ENCODER_ID") == 0) {
            encoderId = atoi(params);
            return true;
        }

        /*md
- `TYPE: TWO_SIDED` is used to set the encoder type
    - `TYPE: TWO_SIDED` when there is a centered value like PAN and you want to show both side value: 20%|80%
        */
        if (strcmp(key, "TYPE") == 0) {
            if (strcmp(params, "TWO_SIDED") == 0) {
                type = 1;
            } else {
                type = atoi(params);
            }
            return true;
        }

        /*md - `LABEL: custom_label` overwrite the value label */
        if (strcmp(key, "LABEL") == 0) {
            strcpy(labelBuffer, params);
            label = labelBuffer;
            return true;
        }

        /*md - `BACKGROUND: #000000` set the background color */
        if (strcmp(key, "BACKGROUND") == 0) {
            colors.background = draw.getColor(params);
            colors.backgroundProgress = lighten(styles.colors.background, 0.5);
            return true;
        }

        /*md - `COLOR: #555555` set the background progress bar color. To be set after BACKGROUND parameter, else it will be overwritten. */
        if (strcmp(key, "COLOR") == 0) {
            colors.backgroundProgress = draw.getColor(strtok(params, " "));
            char* alphaVal = strtok(NULL, " ");
            if (alphaVal != NULL) {
                colors.backgroundProgress = alpha(colors.backgroundProgress, atof(alphaVal));
            }
            return true;
        }

        /*md - `TEXT_COLOR: #ffffff` set the text color */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            colors.title = alpha(draw.getColor(params), 0.5);
            colors.value = alpha(draw.getColor(params), 0.8);
            colors.unit = alpha(draw.getColor(params), 0.4);
            return true;
        }

        /*md - `FLOAT_PRECISION: 2` set how many digits after the decimal point (by default none) */
        if (strcmp(key, "FLOAT_PRECISION") == 0) {
            valueFloatPrecision = atoi(params);
            return true;
        }

        /*md - `SHOW_VALUE: TRUE` show value (default TRUE) */
        if (strcmp(key, "SHOW_VALUE") == 0) {
            showValue = (strcmp(params, "TRUE") == 0);
            return true;
        }

        /*md - `SHOW_UNIT: TRUE` show unit (default TRUE) */
        if (strcmp(key, "SHOW_UNIT") == 0) {
            showUnit = (strcmp(params, "TRUE") == 0);
            return true;
        }

        /*md - `FONT_UNIT_SIZE: 12` set the unit font size */
        if (strcmp(key, "FONT_UNIT_SIZE") == 0) {
            fontUnitSize = atoi(params);
            return true;
        }

        /*md - `FONT_VALUE_SIZE: 12` set the value font size */
        if (strcmp(key, "FONT_VALUE_SIZE") == 0) {
            fontValueSize = atoi(params);
            return true;
        }

        return false;
    }

    void onEncoder(int8_t id, int8_t direction)
    {
        if (id == encoderId) {
            value->increment(direction);
        }
    }

    void* data(int id, void* userdata = NULL) override
    {
        if (id == 0) {
            int8_t* direction = (int8_t*)userdata;
            value->increment(*direction);
            return NULL;
        }
        return NULL;
    }
};

#endif
