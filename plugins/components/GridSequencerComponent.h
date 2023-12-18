#ifndef _UI_COMPONENT_GRID_SEQUENCER_H_
#define _UI_COMPONENT_GRID_SEQUENCER_H_

#include "base/Grid.h"
#include "component.h"

class Step {
public:
    bool enabled = false;
    float velocity = 0;
    uint8_t condition = 0;
};

class Track {
public:
    std::string name = "Init";
    float volume = rand() % 100 / 100.0f;
    bool active = false;
    Step steps[32];

    void randomize()
    {
        name = "Track " + std::to_string(rand() % 100);
        active = rand() % 6 != 0;

        for (unsigned int step = 0; step < 32; step++) {
            steps[step].enabled = rand() % 4 == 0;
            steps[step].velocity = steps[step].enabled && rand() % 4 == 0 ? 1.0f : 0.7f;
            steps[step].condition = rand() % 2 ? 0 : (rand() % 4);
        }
    }
};

class GridSequencerComponent : public Component {
protected:
    int firstColumnWidth = 92;
    int firstColumnMargin = 4;

    Size itemSize;
    int itemW;
    int itemMargin = 2;
    int progressMarginY = 3;
    int topMargin = 5;

    const static uint8_t trackCount = 12;
    const static uint8_t stepsCount = 32;

    Grid grid = Grid(trackCount + 1, stepsCount + 1);

    Point progressPosition = { 0, 0 };
    Size progressItemSize = { 0, 5 };

    int rowY[trackCount + 1];

    void progressInit()
    {
        for (unsigned int step = 0; step < stepsCount; step++) {
            int x = progressPosition.x + itemW * step;
            draw.filledRect({ x, progressPosition.y }, progressItemSize, colors.progressBg);
        }
    }

    void renderProgress(uint8_t stepCounter)
    {
        int xPrevious = progressPosition.x + itemW * ((stepCounter - 1 + stepsCount) % stepsCount);
        draw.filledRect({ xPrevious, progressPosition.y }, progressItemSize, colors.progressBg);
        int x = progressPosition.x + itemW * stepCounter;
        draw.filledRect({ x, progressPosition.y }, progressItemSize, colors.active.on);
    }

    void renderSelection(int8_t row, int8_t col, Color color)
    {
        int y = rowY[row];
        uint8_t h = itemSize.h + 2;
        if (row == trackCount) { // Select volume/master
            h = progressItemSize.h + 2;
            col = 0;
        }

        if (col == 0) {
            draw.rect({ firstColumnMargin, y - 1 }, { firstColumnWidth - 6, h }, color);
        } else {
            int selectW = itemSize.w + 2;
            int x = firstColumnWidth + selectW * (col - 1);
            draw.rect({ x - 1, y - 1 }, { selectW, h }, color);
        }
    }

    void renderSelection()
    {
        renderSelection(grid.lastRow, grid.lastCol, colors.background);
        renderSelection(grid.row, grid.col, colors.selector);
    }

    void renderStep(Track& track, unsigned int step, unsigned int row)
    {
        int y = rowY[row];
        int x = firstColumnWidth + itemW * step;
        Color color = colors.step;
        if (track.steps[step].enabled) {
            color = colors.activeStep;
            if (track.steps[step].condition) {
                color = colors.conditionStep;
            }
            color.a = 255 * track.steps[step].velocity;
        } else if (step % 4 == 0) {
            color = colors.firstStep;
        }
        draw.filledRect({ x, y }, itemSize, color);
    }

    Track tracks[trackCount]; // FIXME
    void renderRow(unsigned int row)
    {
        Track& track = tracks[row];
        renderTrackName(track, rowY[row]);

        for (unsigned int step = 0; step < stepsCount; step++) {
            renderStep(track, step, row);
        }
        renderSelection();
    }

    void renderRows(bool clear = false)
    {
        if (clear) {
            draw.filledRect({ 0, rowY[0] }, { size.h, rowY[0] - progressPosition.y }, colors.background);
        }
        for (unsigned int row = 0; row < trackCount; row++) {
            renderRow(row);
        }
    }

    void renderTrackName(Track& track, int y)
    {
        int w = firstColumnWidth - 8;
        draw.filledRect({ 5, y }, { w, itemSize.h }, colors.step);

        Color trackColor = colors.firstStep;
        Color trackText = colors.track;
        if (track.active) {
            trackColor = colors.active.on;
            trackText = colors.active.selector;
        }
        trackColor.a = 50;
        draw.filledRect({ 5, y }, { w, itemSize.h }, trackColor);
        trackColor.a = 200;
        int width = w * track.volume;
        draw.filledRect({ 5, y }, { width, itemSize.h }, trackColor);

        draw.text({ 8, y }, track.name.c_str(), trackText, 10);
    }

    void renderMasterVolume(bool selected = false)
    {
        int w = firstColumnWidth - 8;
        draw.filledRect({ firstColumnMargin, progressPosition.y - 1 }, { firstColumnWidth - 6, progressItemSize.h + 2 }, colors.background);
        Color color = colors.active.on;
        color.a = 100;
        draw.filledRect({ 5, progressPosition.y }, { w, progressItemSize.h }, color);
        color.a = 200;
        //  int width = w * master.getVolume() / APP_MAX_VOLUME;
        int width = w * 0.9;
        draw.filledRect({ 5, progressPosition.y }, { width, progressItemSize.h }, color);
        if (selected) {
            draw.rect({ firstColumnMargin, progressPosition.y - 1 }, { 86, progressItemSize.h + 2 }, colors.active.selector);
        }
    }

    void resize()
    {
        progressPosition = { firstColumnWidth, position.y + size.h - progressItemSize.h - progressMarginY };
        itemSize.w = (size.w - firstColumnWidth) / stepsCount - itemMargin;
        itemW = itemSize.w + itemMargin;
        progressItemSize.w = itemSize.w;
        itemSize.h = (progressPosition.y - position.y - progressMarginY - topMargin) / trackCount - itemMargin;

        firstColumnMargin = size.w - firstColumnWidth - itemW * stepsCount;

        for (unsigned int track = 0; track < trackCount; track++) {
            rowY[track] = topMargin + position.y + (itemSize.h + itemMargin) * track;
        }

        // Ajust progress bar size base itemSize.h round.
        progressPosition.y = rowY[trackCount - 1] + itemSize.h + itemMargin + progressMarginY;
        // progressItemSize.h = position.y + size.h - progressMarginY - progressPosition.y;

        rowY[trackCount] = progressPosition.y;
    }

    struct ColorsActive {
        Color on;
        Color selector;
    };

    struct Colors {
        Color background;
        Color selector;
        Color step;
        Color firstStep;
        Color activeStep;
        Color conditionStep;
        Color track;
        Color progressBg;
        ColorsActive active;
    } colors = {
        .background = { 0x21, 0x25, 0x2b, 255 }, // #21252b
        .selector = { 0xBB, 0xBB, 0xBB, 255 }, // #bbbbbb
        .step = { 0x2b, 0x2c, 0x2e, 255 }, // #2b2c2e
        .firstStep = { 0x38, 0x3a, 0x3d, 255 }, // #383a3d
        .activeStep = { 0x37, 0x61, 0xa1, 255 }, // #3761a1
        .conditionStep = { 0x37, 0x91, 0xa1, 255 }, // #3791a1
        .track = { 0x88, 0x88, 0x88, 255 }, // #888888
        .progressBg = { 0x38, 0x3a, 0x3d, 255 }, // #383a3d
        .active = {
            .on = { 0x00, 0xb3, 0x00, 255 }, // #00b300
            .selector = { 0xFF, 0xFF, 0xFF, 255 }, // #ffffff
        }
    };

public:
    GridSequencerComponent(ComponentInterface::Props props)
        : Component(props)
    {
        resize();

        tracks[0].randomize();
        tracks[1].randomize();
        tracks[2].randomize();
    }

    void render()
    {
        draw.filledRect(position, size, colors.background);
        progressInit();
        renderMasterVolume();
        renderRows();
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "FIRST_COLUMN_WIDTH") == 0) {
            firstColumnWidth = atoi(value);
            resize();
            return true;
        }
        return false;
    }
};

#endif
