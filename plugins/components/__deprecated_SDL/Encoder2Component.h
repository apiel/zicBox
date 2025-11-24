/** Description:
This document describes the blueprint for a graphical user interface element called the `Encoder2Component`. This component is designed specifically for digital audio workstations or control surfaces, acting as a virtual rotary knob or dial.

**Core Purpose**
The primary function of this component is to visually represent and control a single parameter—such as volume, pan, or effect intensity—managed by an audio plugin. It links to the parameter's underlying value, displays it dynamically, and facilitates user interaction.

**Visual Representation**
The encoder is presented as a circular dial. It features an outer ring that acts as a bar indicator, showing the parameter's current level via rotation. A central dot or "knob" indicates the exact position.
Below the dial, the component displays essential information:
1.  **Label/Title:** The name of the parameter being controlled.
2.  **Value:** The current numerical setting (e.g., "50.5").
3.  **Unit:** The physical measurement if applicable (e.g., "dB" or "%").

**Control and Interaction**
This component is designed to be interactive, typically responding to external physical controls (like a rotary encoder input device). When the user turns the associated physical knob, the component updates the linked audio parameter and instantly refreshes its own visual display. It supports various value types, including standard linear values and "centered" values (like stereo panning).

**Customization**
Developers can extensively configure the component's appearance and behavior, including:
*   Setting the precise link to the audio plugin value.
*   Defining colors for every element (background, value bar, knob, text).
*   Adjusting font sizes for the value and unit display.
*   Controlling whether specific elements, such as the knob or the unit text, are visible.

In essence, the `Encoder2Component` serves as a responsive, customizable digital dial for manipulating audio settings.

sha: 33561cdd0b2a609a394c59a5e9567e11e265c7d069285a171bef7e8607367ac4 
*/
#ifndef _UI_COMPONENT2_ENCODER_H_
#define _UI_COMPONENT2_ENCODER_H_

#include "../component.h"
#include "../utils/color.h"
#include <math.h>
#include <string>

/*md
## SDL Encoder2

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/Encoder2.png" />

Encoder2 is used to display current audio plugin value for a given parameter.
*/
class Encoder2Component : public Component {
protected:
    const char* name = NULL;
    std::string label;
    char labelBuffer[32];
    uint8_t type = 0;
    int radius = 20;
    int insideRadius = 15;

    int fontValueSize = 10;
    int fontUnitSize = 9;
    int twoSideMargin = 2;
    int knobMargin = 2;

    Point knobCenter;
    Point valuePosition;

    bool showKnob = true;
    bool showValue = true;
    bool showGroup = false;
    bool showUnit = true;
    bool stringValueReplaceTitle = false;

    const int marginTop = 3;

    bool encoderActive = true;
    int8_t encoderId = -1;
    uint8_t valueFloatPrecision = 0;

    ValueInterface* value = NULL;

    void renderLabel()
    {
        if (stringValueReplaceTitle && value->props().type == VALUE_STRING) {
            // draw.textCentered({ knobCenter.x, knobCenter.y + insideRadius }, value->string(), 12, { colors.title, .maxWidth = size.w - 4 });
            draw.textCentered({ knobCenter.x, knobCenter.y + insideRadius }, value->string(), 12, { colors.title, NULL, size.w - 4 });
        } else {
            draw.textCentered({ knobCenter.x, knobCenter.y + insideRadius }, label, 12, { colors.title });
        }
    }

    void renderActiveGroup()
    {
        if (showGroup && encoderActive) {
            draw.filledRect({ position.x + margin, position.y + margin }, { 12, 12 }, { colors.id });
            draw.textCentered({ position.x + margin + 6, position.y + margin }, std::to_string(encoderId + 1).c_str(), 8, { colors.background });
        }
    }

    void renderKnob()
    {
        if (showKnob) {
            int knobRadius = insideRadius - knobMargin;
            draw.filledCircle({ knobCenter.x, knobCenter.y - marginTop }, knobRadius, { colors.knob });

            // draw dot at value position
            int cx = knobCenter.x;
            int cy = knobCenter.y - marginTop;
            int r = knobRadius - 3;
            float angleDegrees = 280 * value->pct();
            float angleRadians = angleDegrees * M_PI / 180.0 - 180;
            int x = cx + r * cos(angleRadians);
            int y = cy + r * sin(angleRadians);

            draw.filledCircle({ x, y }, 2, { colors.knobDot });
        }
    }

    void renderBar()
    {
        int val = 280 * value->pct();

        if (val < 280) {
            draw.filledPie({ knobCenter.x, knobCenter.y - marginTop }, radius, 130, 50, { colors.barBackground });
        }
        if (val > 0) {
            int endAngle = 130 + val;
            if (endAngle > 360) {
                endAngle = endAngle - 360;
            }
            draw.filledPie({ knobCenter.x, knobCenter.y - marginTop }, radius, 130, endAngle, { colors.bar });
        }
        draw.filledCircle({ knobCenter.x, knobCenter.y - marginTop }, insideRadius, { colors.background });
    }

    void renderCenteredBar()
    {
        int val = 280 * value->pct();

        draw.filledPie({ knobCenter.x, knobCenter.y - marginTop }, radius, 130, 50, { colors.barBackground });
        if (val > 140) {
            int endAngle = 130 + val;
            if (endAngle > 360) {
                endAngle = endAngle - 360;
            }

            draw.filledPie({ knobCenter.x, knobCenter.y - marginTop }, radius, 270, endAngle, { colors.bar });
        } else if (val < 140) {
            draw.filledPie({ knobCenter.x, knobCenter.y - marginTop }, radius, 270 - (140 - val), 270, { colors.bar });
        }
        draw.filledCircle({ knobCenter.x, knobCenter.y - marginTop }, insideRadius, { colors.background });
    }

    void renderUnit()
    {
        if (showUnit && value->props().unit.length() > 0) {
            draw.textCentered({ valuePosition.x, valuePosition.y + fontValueSize - 5 }, value->props().unit, fontUnitSize, { colors.unit });
        }
    }

    void renderValue()
    {
        if (!stringValueReplaceTitle && value->props().type == VALUE_STRING) {
            draw.textCentered({ valuePosition.x, valuePosition.y - 5 }, value->string(), fontValueSize, { colors.value });
        } else {
            std::string valStr = std::to_string(value->get());
            valStr = valStr.substr(0, valStr.find(".") + valueFloatPrecision + (valueFloatPrecision > 0 ? 1 : 0));

            draw.textCentered({ valuePosition.x, valuePosition.y - 5 }, valStr.c_str(), fontValueSize, { colors.value });
        }
    }

    void renderTwoSidedValue()
    {
        int val = value->get();
        // FIXME use floating point...
        draw.textRight({ valuePosition.x - twoSideMargin, valuePosition.y - 5 }, std::to_string((int)value->props().max - val).c_str(),
            fontValueSize - 3, { colors.value });
        draw.text({ valuePosition.x + twoSideMargin, valuePosition.y - 5 }, std::to_string(val).c_str(),
            fontValueSize - 3, { colors.value });

        draw.line({ valuePosition.x, valuePosition.y - 10 }, { valuePosition.x, valuePosition.y + 10 }, { colors.barTwoSide });
        draw.line({ valuePosition.x - 1, valuePosition.y - 10 }, { valuePosition.x - 1, valuePosition.y + 10 }, { colors.barTwoSide });
    }

    void renderEncoder()
    {
        renderActiveGroup();

        renderLabel();
        if (value->props().type == VALUE_CENTERED) {
            renderCenteredBar();
        } else {
            renderBar();
        }

        renderKnob();

        if (showValue) {
            renderUnit();
            if (value->props().type == VALUE_CENTERED && type == 1) {
                renderTwoSidedValue();
            } else {
                renderValue();
            }
        }
    }

    struct Colors {
        Color background;
        Color id;
        Color title;
        Color value;
        Color unit;
        Color bar;
        Color barBackground;
        Color barTwoSide;
        Color knob;
        Color knobDot;
    } colors;

    const int margin;

    void setRadius(int _radius)
    {
        radius = _radius;
        insideRadius = radius - 5;

        if (radius > 35) {
            fontValueSize = 15;
            fontUnitSize = 11;
            twoSideMargin = 5;
            knobMargin = 4;
        } else if (radius > 26) {
            fontValueSize = 14;
            twoSideMargin = 3;
            knobMargin = 3;
        } else if (radius > 24) {
            fontValueSize = 12;
            knobMargin = 3;
        }
    }

public:
    // margin left 15
    // margin right 10
    Encoder2Component(ComponentInterface::Props props)
        : Component(props)
        , margin(styles.margin)
    {
        if (size.h < 60) {
            printf("Encoder component height too small: %dx%d. Min height is 60.\n", size.w, size.h);
            size.h = 60;
        }

        if (size.w < 60) {
            printf("Encoder component width too small: %dx%d. Min width is 60.\n", size.w, size.h);
            size.w = 60;
        }

        colors = {
            styles.colors.background,
            darken({ 0x80, 0x80, 0x80, 255 }, 0.3),
            alpha(styles.colors.white, 0.4),
            alpha(styles.colors.white, 0.4),
            alpha(styles.colors.white, 0.2),
            styles.colors.primary,
            alpha(styles.colors.primary, 0.5),
            alpha(styles.colors.primary, 0.2),
            draw.getColor("#35373b"),
            alpha(styles.colors.white, 0.6),
        };

        knobCenter = { (int)(position.x + (size.w * 0.5)), (int)(position.y + ((size.h - 12) * 0.5)) };
        valuePosition = { knobCenter.x, knobCenter.y - marginTop - 2 };
        setRadius((size.h - 24) * 0.5);
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

        /*md - `COLOR: #3791a1` set the ring color */
        if (strcmp(key, "COLOR") == 0) {
            colors.bar = draw.getColor(params);
            colors.barBackground = alpha(colors.bar, 0.5);
            colors.barTwoSide = alpha(colors.bar, 0.2);
            return true;
        }

        /*md - `BACKGROUND_COLOR: #000000` set the background color */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            colors.background = draw.getColor(params);
            return true;
        }

        /*md - `TEXT_COLOR: #ffffff` set the text color */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            colors.title = alpha(draw.getColor(params), 0.4);
            colors.value = alpha(draw.getColor(params), 0.4);
            colors.unit = alpha(draw.getColor(params), 0.2);
            return true;
        }

        /*md - `KNOB_COLOR: #888888` set the knob color (middle circle) */
        if (strcmp(key, "KNOB_COLOR") == 0) {
            colors.knob = draw.getColor(params);
            // colors.knobDot = lighten(colors.knob, 0.7);
            return true;
        }

        /*md - `FLOAT_PRECISION: 2` set how many digits after the decimal point (by default none) */
        if (strcmp(key, "FLOAT_PRECISION") == 0) {
            valueFloatPrecision = atoi(params);
            return true;
        }

        /*md - `SHOW_KNOB: FALSE` show the knob (middle circle) (default TRUE) */
        if (strcmp(key, "SHOW_KNOB") == 0) {
            showKnob = (strcmp(params, "TRUE") == 0);
            return true;
        }

        /*md - `SHOW_GROUP: TRUE` show group if the component is part of the current active group (default FALSE) */
        if (strcmp(key, "SHOW_GROUP") == 0) {
            showGroup = (strcmp(params, "TRUE") == 0);
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

        /*md - `KNOB_CENTERED: TRUE` */
        if (strcmp(key, "KNOB_CENTERED") == 0) {
            knobCenter.y = (int)(position.y + (size.h * 0.5));
            valuePosition = { knobCenter.x, knobCenter.y - marginTop - 2 };
            setRadius(size.h / 3.0);
            return true;
        }

        /*md - `STRING_VALUE_REPLACE_TITLE: true` instead to show string value in knob, show under the knob. Can be useful for long string value. */
        if (strcmp(key, "STRING_VALUE_REPLACE_TITLE") == 0) {
            stringValueReplaceTitle = (strcmp(params, "TRUE") == 0);
            return true;
        }

        return false;
    }

    void onEncoder(int8_t id, int8_t direction)
    {
        if (encoderActive && id == encoderId) {
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
