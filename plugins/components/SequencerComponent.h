#ifndef _UI_COMPONENT_SEQUENCER_H_
#define _UI_COMPONENT_SEQUENCER_H_

#include "../audio/stepInterface.h"
#include "component.h"

#include <string>
// #include <cstring>

class SequencerComponent : public Component {
protected:
    AudioPlugin& plugin;

    Size stepSize;

    Step* steps;
    uint8_t* stepCounter;
    uint8_t previousStepCounter = 0;

    int debounceSelectedStep = 0;
    float previousSelectedStep = 0.0;

    void renderStep(uint8_t index)
    {
        Color color = colors.stepBackground;
        // if (steps[index].enabled) {
        //     color = colors.stepEnabled;
        // } else if (index % 4 != 0) {
        //     color.a = 150;
        // }

        uint8_t row = index / columnCount;
        uint8_t column = index % columnCount;

        int x = position.x + column * stepSize.w + margin;
        int y = position.y + row * stepSize.h + margin;
        int w = stepSize.w - 2 * margin;
        int h = stepSize.h - 2 * margin;


        draw.filledRect({ x, y }, { w, h }, color);

        draw.textRight({ x + w - 1, y }, std::to_string(index + 1).c_str(), colors.textInfo, 10);

        // int sel = selectedStep->get();
        // if (index == sel)
        // {
        //     draw.filledRect({x, stepPosition.y - 3}, {stepSize.w, 2}, colors.stepBackground);
        // }

        // if (index == *stepCounter) {
        //     draw.filledRect({ x, stepPosition.y - 3 }, { stepSize.w, 2 }, colors.activePosition);
        // } else if (steps[index].counter) {
        //     int w = stepSize.w * (steps[index].counter / (float)steps[index].len);
        //     draw.filledRect({ x + stepSize.w - w, stepPosition.y - 3 }, { w, 2 }, colors.activePosition);
        // }
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
        Color stepBackground;
        Color stepEnabled;
        Color activePosition;
        Color textInfo;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.75),
            draw.darken(color, 0.55),
            color,
            styles.colors.on,
            draw.darken(color, 0.3) });
    }

    const int margin = 4;

    uint8_t stepCount = 32;
    uint8_t rowCount = 4;
    uint8_t columnCount = stepCount / rowCount;

public:
    SequencerComponent(ComponentInterface::Props& props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
        , plugin(getPlugin("Sequencer"))
    {

        stepSize = {
            (int)((props.size.w) / (float)columnCount),
            (int)((props.size.h) / (float)(rowCount + 1))
        };

        steps = (Step*)plugin.data(0);
        stepCounter = (uint8_t*)plugin.data(1);
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
            return true;
        }

        return false;
    }
};

#endif
