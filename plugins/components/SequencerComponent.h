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
        ModeNote,
        ModeVelocity,
        ModeLength,
        ModeCondition,
        ModeCount,
    } mode
        = ModeStep;

    const char* modeName[ModeCount] = {
        "Step",
        "Note",
        "Velocity",
        "Length",
        "Condition",
    };

    AudioPlugin& plugin;

    Size stepSize;

    Step* steps;
    uint8_t* stepCounter;
    uint8_t previousStepCounter = 0;

    int debounceSelectedStep = 0;
    float previousSelectedStep = 0.0;

    uint8_t stepCount = 32;
    static const uint8_t columnCount = 8;
    uint8_t rowCount = stepCount / columnCount;

    uint8_t selectedStep = 0;
    bool encoderActive = false;
    uint8_t encoderCount = 0;
    int8_t encoderIds[columnCount] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    bool roundEncoderSlection = true;

    const char* getStepText(uint8_t index)
    {
        if (mode == ModeVelocity) {
            return (std::to_string((int)(steps[index].velocity * 100)) + "%").c_str();
        } else if (mode == ModeLength) {
            return std::to_string(steps[index].len).c_str();
        } else if (mode == ModeCondition) {
            return (const char *)plugin.data(2, &steps[index].condition);
        }
        return MIDI_NOTES_STR[steps[index].note];
    }

    Xywh getStepRect(uint8_t row, uint8_t column)
    {
        return {
            position.x + column * stepSize.w + stepMargin,
            position.y + row * stepSize.h + stepMargin,
            stepSize.w - 2 * stepMargin,
            stepSize.h - 2 * stepMargin
        };
    }

    void renderStep(uint8_t index)
    {
        uint8_t row = index / columnCount;
        uint8_t column = index % columnCount;

        auto [x, y, w, h] = getStepRect(row, column);

        ColorsStep& c = steps[index].enabled ? colorsActive : colorsInactive;

        draw.filledRect({ x, y }, { w, h }, c.background);

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

        if (encoderActive) {
            uint8_t encoderRow = selectedStep / columnCount;
            if (encoderRow == row) {
                uint8_t encoderIndex = selectedStep % columnCount;
                if (encoderIndex > encoderCount) {
                    encoderIndex = columnCount - encoderCount;
                }
                if (column >= encoderIndex && column < encoderIndex + encoderCount) {
                    draw.filledRect({ x, y }, { 12, 12 }, c.id);
                    draw.textCentered({ x + 6, y }, std::to_string(encoderIds[column - encoderIndex] + 1).c_str(), colors.background, 8);
                }
            }
        }
    }

    void renderModeSelection()
    {
        for (int i = 0; i < ModeCount; i++) {
            auto [x, y, w, h] = getStepRect(rowCount, i);

            draw.filledRect({ x, y }, { w, h }, colorsMode.background);
            Point textPosition = {
                (int)(x + w * 0.5),
                (int)(y + (h - fontSize) * 0.5)
            };
            draw.textCentered(textPosition, modeName[i], mode == i ? colorsMode.text : colorsMode.textInactive, fontSize);
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
        }

        renderModeSelection();
    }

    struct Colors {
        Color background;
        Color activePosition;
    } colors;

    struct ColorsStep {
        Color background;
        Color text;
        Color textActive;
        Color textInactive;
        Color id;
    } colorsActive, colorsInactive;

    struct ColorsMode {
        Color background;
        Color text;
        Color textInactive;
    } colorsMode;

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
            draw.darken(color, 0.4),
            draw.darken(color, 0.3) });
    }

    ColorsMode getColorsModeFromColor(Color color)
    {
        return ColorsMode({ draw.darken(color, 0.50),
            color,
            draw.darken(color, 0.4) });
    }

    const int stepMargin = 4;
    const int margin;
    uint8_t fontSize;

public:
    SequencerComponent(ComponentInterface::Props& props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
        , colorsActive(getColorsStepFromColor(styles.colors.blue))
        , colorsInactive(getColorsStepFromColor(draw.darken(styles.colors.blue, 0.5)))
        , colorsMode(getColorsModeFromColor(styles.colors.grey))
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

        if (strcmp(key, "MODE_COLOR") == 0) {
            colorsMode = getColorsModeFromColor(draw.getColor(value));
            return true;
        }

        if (strcmp(key, "ENCODER_ID") == 0) {
            if (encoderCount < columnCount) {
                encoderIds[encoderCount] = atoi(value);
                encoderCount++;
            }
            return true;
        }

        if (strcmp(key, "ENCODER_SPLITTED_SELECTION") == 0) {
            roundEncoderSlection = strcmp(value, "true") == 0;
            return true;
        }

        return false;
    }

    void onGroupChanged(int8_t index) override
    {
        int8_t shouldActivate = false;
        if (group == index || group == -1) {
            shouldActivate = true;
        }
        if (shouldActivate != encoderActive) {
            encoderActive = shouldActivate;
            renderNext();
        }
    }

    void onMotionRelease(MotionInterface& motion)
    {
        if (motion.in({ position, size })) {
            int row = (motion.position.y - position.y) / stepSize.h;
            int column = (motion.position.x - position.x) / stepSize.w;

            if (row < rowCount) {
                int index = row * columnCount + column;
                if (roundEncoderSlection) {
                    index = (int)(index / encoderCount) * encoderCount;
                }
                selectedStep = index;
                renderNext();
            } else if (column < ModeCount) {
                mode = (Mode)column;
                renderNext();
            }
        }
    }
};

#endif
