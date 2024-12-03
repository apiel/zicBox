#ifndef _UI_PIXEL_COMPONENT_DRUM_CLICK_H_
#define _UI_PIXEL_COMPONENT_DRUM_CLICK_H_

#include "../../../helpers/range.h"
#include "../component.h"
#include "../utils/color.h"
#include "utils/GroupColorComponent.h"

/*md
## DrumClickComponent
*/

class DrumClickComponent : public GroupColorComponent {
protected:
    AudioPlugin* plugin = NULL;

    bool filled = true;
    bool outline = true;

    Color bgColor;

    ToggleColor fillColor;
    ToggleColor outlineColor;
    ToggleColor textColor;

    int encoderLevel = -1;
    int encoderDuration = -1;
    int encoderFilter = -1;
    int encoderResonance = -1;

    ValueInterface* valueLevel = NULL;
    ValueInterface* valueDuration = NULL;
    ValueInterface* valueFilter = NULL;
    ValueInterface* valueResonance = NULL;

    uint8_t waveformDataId = 10;

    int waveformHeight = 30;
    int waveformY = 0;

    void renderWaveform()
    {
        float* waveform = (float*)plugin->data(waveformDataId, &size.w);
        if (waveform) {
            std::vector<Point> relativePoints;
            float halfHeight = waveformHeight * 0.5;
            relativePoints.push_back({ relativePosition.x, (int)(waveformY + halfHeight) });
            for (int i = 0; i < size.w; i++) {
                Point point = { i, (int)(waveform[i] * halfHeight + halfHeight) };
                relativePoints.push_back({ point.x + relativePosition.x, point.y + waveformY });
            }
            relativePoints.push_back({ relativePosition.x + size.w, (int)(waveformY + halfHeight) });
            if (filled) {
                draw.filledPolygon(relativePoints, { fillColor.color });
            }
            if (outline) {
                draw.lines(relativePoints, { outlineColor.color });
            }
        }
    }

    void renderTitles()
    {
        int cellWidth = size.w / 4;
        int x = relativePosition.x + cellWidth * 0.5;
        valueLevel && draw.textCentered({ x, relativePosition.y }, std::to_string(valueLevel->get()), 8, { textColor.color });
        valueDuration && draw.textCentered({ x + cellWidth, relativePosition.y }, std::to_string(valueDuration->get()), 8, { textColor.color });
        valueFilter && draw.textCentered({ x + cellWidth * 2, relativePosition.y }, valueFilter->string(), 8, { textColor.color });
        valueResonance && draw.textCentered({ x + cellWidth * 3, relativePosition.y }, std::to_string(valueResonance->get()), 8, { textColor.color });
    }

public:
    DrumClickComponent(ComponentInterface::Props props)
        : GroupColorComponent(props, { { "FILL_COLOR", &fillColor }, { "OUTLINE_COLOR", &outlineColor }, { "TEXT_COLOR", &textColor } })
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text, inactiveColorRatio)
        , fillColor(styles.colors.primary, inactiveColorRatio)
        , outlineColor(lighten(styles.colors.primary, 0.5), inactiveColorRatio)
    {
        updateColors();
        waveformHeight = size.h - 9;
    }

    void render() override
    {
        if (updatePosition()) {
            waveformY = relativePosition.y + 8;
            draw.filledRect(relativePosition, size, { bgColor });
            renderWaveform();
            renderTitles();
        }
    }

    void onEncoder(int id, int8_t direction) override
    {
        if (isActive) {
            if (id == encoderLevel) {
                valueLevel->increment(direction);
                renderNext();
            } else if (id == encoderDuration) {
                valueDuration->increment(direction);
                renderNext();
            } else if (id == encoderFilter) {
                valueFilter->increment(direction);
                renderNext();
            } else if (id == encoderResonance) {
                valueResonance->increment(direction);
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
            valueLevel = watch(plugin->getValue("CLICK"));
            valueDuration = watch(plugin->getValue("CLICK_DURATION"));
            valueFilter = watch(plugin->getValue("CLICK_CUTOFF"));
            valueResonance = watch(plugin->getValue("CLICK_RESONANCE"));
            return true;
        }

        /*md - `WAVEFORM_DATA_ID: waveform_id size_id` is the data id to get the shape of the waveform to draw.*/
        if (strcmp(key, "WAVEFORM_DATA_ID") == 0) {
            waveformDataId = atoi(value);
            return true;
        }

        /*md - `ENCODER_LEVEL: id` is the id of the encoder to select the clicking level. */
        if (strcmp(key, "ENCODER_LEVEL") == 0) {
            encoderLevel = atoi(value);
            return true;
        }

        /*md - `ENCODER_DURATION: id` is the id of the encoder to define the clicking duration. */
        if (strcmp(key, "ENCODER_DURATION") == 0) {
            encoderDuration = atoi(value);
            return true;
        }

        /*md - `ENCODER_FILTER: id` is the id of the encoder to set the clicking filter amount. */
        if (strcmp(key, "ENCODER_FILTER") == 0) {
            encoderFilter = atoi(value);
            return true;
        }

        /*md - `ENCODER_RESONANCE: id` is the id of the encoder to set the clicking resonance amount. */
        if (strcmp(key, "ENCODER_RESONANCE") == 0) {
            encoderResonance = atoi(value);
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
