#ifndef _UI_COMPONENT_GRID_SEQUENCER_H_
#define _UI_COMPONENT_GRID_SEQUENCER_H_

#include "base/Grid.h"
#include "component.h"

class GridSequencerComponent : public Component {
protected:
    uint8_t trackCount = 12;
    uint8_t stepsCount = 32;

    Grid grid = Grid(trackCount + 1, stepsCount + 1);

    Point progressPosition = { 0, 0 };
    Size progressItemSize = { 10, 5 };

    void progressInit()
    {
        for (unsigned int step = 0; step < stepsCount; step++) {
            int x = progressPosition.x + 12 * step;
            draw.filledRect({ x, progressPosition.y }, progressItemSize, colors.progress.background);
        }
    }

    void progressRender(uint8_t stepCounter)
    {
        int xPrevious = progressPosition.x + 12 * ((stepCounter - 1 + stepsCount) % stepsCount);
        draw.filledRect({ xPrevious, progressPosition.y }, progressItemSize, colors.progress.background);
        int x = progressPosition.x + 12 * stepCounter;
        draw.filledRect({ x, progressPosition.y }, progressItemSize, colors.progress.on);
    }

    struct ColorsProgress {
        Color background;
        Color on;
    };

    struct Colors {
        Color background;
        ColorsProgress progress;
    } colors = {
        .background = { 0x21, 0x25, 0x2b, 255 }, // #21252b
        .progress = {
            .background = { 0x38, 0x3a, 0x3d, 255 }, // #383a3d
            .on = { 0x00, 0xb3, 0x00, 255 }, // #00b300
        }
    };

public:
    GridSequencerComponent(ComponentInterface::Props props)
        : Component(props)
    {
        progressPosition = { 92, props.position.y + props.size.h - progressItemSize.h - 5 };
    }

    void render()
    {
        draw.filledRect(position, size, colors.background);
        progressInit();
    }

    bool config(char* key, char* value)
    {

        return false;
    }
};

#endif
