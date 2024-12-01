#ifndef _UI_PIXEL_COMPONENT_WAVEFORM_H_
#define _UI_PIXEL_COMPONENT_WAVEFORM_H_

#include "../../../helpers/range.h"
#include "../component.h"
#include "../utils/color.h"
#include "utils/GroupColorComponent.h"

/*md
## waveform

// <img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/DrumEnvelop.png" />

Show a representation of a waveform.
*/

class WaveformComponent : public GroupColorComponent {
protected:
    AudioPlugin* plugin = NULL;

    bool filled = true;
    bool outline = true;

    Color bgColor;

    ToggleColor fillColor;
    ToggleColor outlineColor;
    ToggleColor textColor;

    int encoderType = -1;
    int encoderShape = -1;
    int encoderMacro = -1;

    ValueInterface* valueType = NULL;

    float* waveform = NULL;
    uint16_t* waveformSize = NULL;

    void renderWaveform()
    {
        std::vector<Point> relativePoints;
        float halfHeight = size.h * 0.5;
        for (int i = 0; i < *waveformSize; i++) {
            Point point = { (int)(size.w * i / (*waveformSize - 1)), (int)(waveform[i] * halfHeight + halfHeight) };
            relativePoints.push_back({ point.x + relativePosition.x, point.y + relativePosition.y });
        }
        if (filled) {
            draw.filledPolygon(relativePoints, { fillColor.color });
        }
        if (outline) {
            draw.lines(relativePoints, { outlineColor.color });
        }
    }

public:
    WaveformComponent(ComponentInterface::Props props)
        : GroupColorComponent(props, { { "FILL_COLOR", &fillColor }, { "OUTLINE_COLOR", &outlineColor }, { "TEXT_COLOR", &textColor } })
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text, inactiveColorRatio)
        , fillColor(styles.colors.primary, inactiveColorRatio)
        , outlineColor(lighten(styles.colors.primary, 0.5), inactiveColorRatio)
    {
        updateColors();
    }

    void render() override
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { bgColor });
            if (waveform && waveformSize) {
                printf("render waveform\n");
                renderWaveform();
            }
        }
    }

    void onEncoder(int id, int8_t direction) override
    {
        if (isActive) {
            if (id == encoderType) {
                valueType->increment(direction);
                renderNext();
            } else if (id == encoderShape) {
                renderNext();
            } else if (id == encoderMacro) {
                renderNext();
            }
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        /*md - `PLUGIN: plugin_name` set plugin target */
        if (strcmp(key, "PLUGIN") == 0) {
            plugin = &getPlugin(value, track);
            valueType = watch(plugin->getValue("WAVEFORM_TYPE"));
            return true;
        }

        /*md - `WAVEFORM_DATA_ID: waveform_id size_id` is the data id to get the shape of the waveform to draw.*/
        if (strcmp(key, "WAVEFORM_DATA_ID") == 0) {
            uint8_t waveformId = atoi(strtok(value, " "));
            char* sizeIdStr = strtok(NULL, " ");
            uint8_t sizeId = sizeIdStr ? atoi(sizeIdStr) : waveformId + 1;
            waveform = (float*)plugin->data(waveformId);
            waveformSize = (uint16_t*)plugin->data(sizeId);
            return true;
        }

        /*md - `SIZE_DATA_ID: id` is the data id to get the size of the waveform to draw.*/
        if (strcmp(key, "SIZE_DATA_ID") == 0) {
            return true;
        }

        /*md - `ENCODER_TYPE: id` is the id of the encoder to select the type of the waveform. */
        if (strcmp(key, "ENCODER_TYPE") == 0) {
            encoderType = atoi(value);
            return true;
        }

        /*md - `ENCODER_SHAPE: id` is the id of the encoder to morph the shape of the waveform. */
        if (strcmp(key, "ENCODER_SHAPE") == 0) {
            encoderShape = atoi(value);
            return true;
        }

        /*md - `ENCODER_MACRO: id` is the id of the encoder to set the macro value of the waveform. Macro is arbitrary parameter depending of selected waveform type. */
        if (strcmp(key, "ENCODER_MACRO") == 0) {
            encoderMacro = atoi(value);
            return true;
        }

        /*md - `OUTLINE: true/false` is if the envelop should be outlined (default: true). */
        if (strcmp(key, "OUTLINE") == 0) {
            outline = strcmp(value, "true") == 0;
            return true;
        }

        /*md - `FILLED: true/false` is if the envelop should be filled (default: true). */
        if (strcmp(key, "FILLED") == 0) {
            filled = strcmp(value, "true") == 0;
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(value);
            return true;
        }

        /*md - `FILL_COLOR: color` is the color of the envelop. */
        /*md - `OUTLINE_COLOR: color` is the color of the envelop outline. */
        /*md - `TEXT_COLOR: color` is the color of the text. */
        /*md - `INACTIVE_COLOR_RATIO: 0.0 - 1.0` is the ratio of darkness for the inactive color (default: 0.5). */
        return GroupColorComponent::config(key, value);
    }
};

#endif
