#ifndef _UI_COMPONENT_SEQUENCER_H_
#define _UI_COMPONENT_SEQUENCER_H_

#include "../../helpers/midiNote.h"
#include "../audio/stepInterface.h"
#include "component.h"

#include <string>

class SequencerComponent : public Component {
protected:
    enum Mode {
        ModeStep,
        ModeVelocity,
        ModeLength,
        ModeCondition,
    } mode
        = ModeStep;

    AudioPlugin& plugin;

    Size stepSize;

    Step* steps;
    uint8_t* stepCounter;
    uint8_t previousStepCounter = 0;

    int debounceSelectedStep = 0;
    float previousSelectedStep = 0.0;

    const char* getStepText(uint8_t index)
    {
        if (mode == ModeVelocity) {
            return std::to_string(steps[index].velocity).c_str();
        } else if (mode == ModeLength) {
            return std::to_string(steps[index].len).c_str();
        } else if (mode == ModeCondition) {
            return "Impair";
        }
        return MIDI_NOTES_STR[steps[index].note];
    }

    void renderStep(uint8_t index)
    {
        uint8_t row = index / columnCount;
        uint8_t column = index % columnCount;

        int x = position.x + column * stepSize.w + stepMargin;
        int y = position.y + row * stepSize.h + stepMargin;
        int w = stepSize.w - 2 * stepMargin;
        int h = stepSize.h - 2 * stepMargin;

        ColorsStep& c = steps[index].enabled ? colorsActive : colorsInactive;

        draw.filledRect({ x, y }, { w, h }, c.stepBackground);

        draw.text({ x + 2, y + h - 12 }, std::to_string(index + 1).c_str(), c.text, 9);

        int stepIndexWidth = 12;
        Point textPosition = {
            (int)(x + stepIndexWidth + (w - stepIndexWidth) * 0.5),
            (int)(y + (h - fontSize) * 0.5)
        };

        Color textColor = steps[index].enabled ? c.textActive : c.textInactive;
        draw.textCentered(textPosition, getStepText(index), textColor, fontSize);

        if (index == *stepCounter) {
            draw.filledRect({ x, y - 3 }, { w, 2 }, colors.activePosition);
        } else if (steps[index].counter) {
            int w2 = w * (steps[index].counter / (float)steps[index].len);
            draw.filledRect({ x + w - w2, y - 3 }, { w2, 2 }, colors.activePosition);
        }
    }

    void render()
    {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            colors.background);

        for (int i = 0; i < stepCount; i++) {
            renderStep(i);
            // printf("step %d, note %d, enabled %d, len %d\n", i, steps[i].note, steps[i].enabled, steps[i].len);
        }
    }

    struct Colors {
        Color background;
        Color activePosition;
    } colors;

    struct ColorsStep {
        Color stepBackground;
        Color text;
        Color textActive;
        Color textInactive;
    } colorsActive, colorsInactive;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.75),
            styles.colors.on });
    }

    ColorsStep getColorsStepFromColor(Color color)
    {
        return ColorsStep({ draw.darken(color, 0.55),
            draw.darken(color, 0.3),
            color,
            draw.darken(color, 0.4) });
    }

    const int stepMargin = 4;

    const int margin;

    uint8_t stepCount = 32;
    uint8_t rowCount = 4;
    uint8_t columnCount = stepCount / rowCount;

    uint8_t fontSize = 9;

public:
    SequencerComponent(ComponentInterface::Props& props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
        , colorsActive(getColorsStepFromColor(styles.colors.blue))
        , colorsInactive(getColorsStepFromColor(draw.darken(styles.colors.blue, 0.5)))
        , plugin(getPlugin("Sequencer"))
        , margin(styles.margin)
    {

        stepSize = {
            (int)((props.size.w) / (float)columnCount),
            (int)((props.size.h) / (float)(rowCount + 1))
        };

        fontSize = stepSize.h * 0.5;

        steps = (Step*)plugin.data(0);
        stepCounter = (uint8_t*)plugin.data(1);
    }

    void triggerRenderer(unsigned long now) override
    {
        if (previousStepCounter != *stepCounter) {
            needRendering = true;
            // TODO could only render necessary part
            previousStepCounter = *stepCounter;
        }
        Component::triggerRenderer(now);
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
            colorsActive = getColorsStepFromColor(draw.getColor(value));
            colorsInactive = getColorsStepFromColor(draw.darken(draw.getColor(value), 0.5));
            return true;
        }

        if (strcmp(key, "ACTIVE_COLOR") == 0) {
            colorsActive = getColorsStepFromColor(draw.getColor(value));
            return true;
        }

        if (strcmp(key, "INACTIVE_COLOR") == 0) {
            colorsInactive = getColorsStepFromColor(draw.getColor(value));
            return true;
        }

        return false;
    }
};

#endif
