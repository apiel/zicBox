#ifndef _UI_COMPONENT_GRID_SEQUENCER_H_
#define _UI_COMPONENT_GRID_SEQUENCER_H_

#include "../controllers/keypadInterface.h"

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
    KeypadInterface* keypad;

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

    void updateSelection()
    {
        renderSelection();
        if (grid.row == trackCount) {
            setVisibility(2);
        } else if (grid.col == 0) {
            setVisibility(0);
        } else {
            setVisibility(1);
        }
    }

    void updateTrackSelection()
    {
        // FIXME if track is over the number of tracks, it will throw
        // instead we should return null
        //
        // or set active track to 0

        // FIXME when assigValue / assignValues, we should delete the old assigned plugin...

        setActiveTrack(grid.row);
        updateSelection();
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

    struct Keys {
        // uint8_t menu = 11;

        uint8_t tracks[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
        uint8_t clips[15] = { 15, 16, 17, 18, 19, 27, 28, 29, 30, 31, 39, 40, 41, 42, 43 };

        uint8_t up = 25;
        uint8_t down = 37;
        uint8_t left = 36;
        uint8_t right = 38;
        uint8_t leftJump = 24;
        uint8_t rightJump = 26;

        uint8_t enc1 = 20;
        uint8_t enc2 = 21;
        uint8_t enc3 = 22;
        uint8_t enc4 = 23;

        uint8_t enc5 = 32;
        uint8_t enc6 = 33;
        uint8_t enc7 = 34;
        uint8_t enc8 = 35;

        uint8_t enc9 = 44;
        uint8_t enc10 = 45;
        uint8_t enc11 = 46;
        uint8_t enc12 = 47;

        // TODO: 15 button from the middle should be clips/variations
    } keys;

    uint8_t columnColor[4] = { 50, 20, 60, 90 }; // or 90

    void renderKeypad()
    {
        if (keypad) {
            keypad->setKeyColor(254, 254); // set all key off
            keypad->setButton(254, 254); // set all button off

            // keypad->setButton(keys.menu, 0);

            for (int i = 0; i < 12; i++) {
                keypad->setButton(keys.tracks[i], tracks[i].active ? 40 : 0);
            }

            for (int i = 0; i < 15; i++) {
                // keypad->setButton(keys.clips[i], clips[i].active ? 70 : 60);
                keypad->setButton(keys.clips[i], 60);
            }

            keypad->setButton(keys.up, 20);
            keypad->setButton(keys.down, 20);
            keypad->setButton(keys.left, 20);
            keypad->setButton(keys.right, 20);

            keypad->setButton(keys.leftJump, 0);
            keypad->setButton(keys.rightJump, 0);

            keypad->setButton(keys.enc1, columnColor[0]);
            keypad->setButton(keys.enc2, columnColor[1]);
            keypad->setButton(keys.enc3, columnColor[2]);
            keypad->setButton(keys.enc4, columnColor[3]);

            keypad->setButton(keys.enc5, columnColor[0]);
            keypad->setButton(keys.enc6, columnColor[1]);
            keypad->setButton(keys.enc7, columnColor[2]);
            keypad->setButton(keys.enc8, columnColor[3]);

            keypad->setButton(keys.enc9, columnColor[0]);
            keypad->setButton(keys.enc10, columnColor[1]);
            keypad->setButton(keys.enc11, columnColor[2]);
            keypad->setButton(keys.enc12, columnColor[3]);

            // TODO use first row of 8 (or even 2 row of 8) to select the clip
        }
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
        keypad = (KeypadInterface*)getController("Keypad");

        resize();

        tracks[0].randomize();
        tracks[1].randomize();
        tracks[2].randomize();
    }

    void initView()
    {
        renderKeypad();
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

    void onKeyPad(int id, int8_t state)
    {
        // printf("onKeypad(%d, %d)\n", id, state);
        if (state == 1) {
            if (id == keys.up) {
                grid.up();
                updateTrackSelection();
                draw.renderNext();
            } else if (id == keys.down) {
                grid.down();
                updateTrackSelection();
                draw.renderNext();
            } else if (id == keys.left) {
                grid.left();
                updateSelection();
                draw.renderNext();
            } else if (id == keys.right) {
                grid.right();
                updateSelection();
                draw.renderNext();
            } else if (id == keys.leftJump) {
                int8_t step = ((grid.col - 1) % 4);
                if (step < 1) {
                    step = 4;
                }
                grid.selectNextCol(-step);
                updateSelection();
                draw.renderNext();
            } else if (id == keys.rightJump) {
                int8_t step = 4 - ((grid.col - 1) % 4);
                grid.selectNextCol(step);
                updateSelection();
                draw.renderNext();
            } else {
                for (int i = 0; i < 12; i++) {
                    if (keys.tracks[i] == id) {
                        grid.select(i, 0);
                        updateTrackSelection();
                        draw.renderNext();
                        return;
                    }
                }

                printf("unknown key %d\n", id);
            }
        }
    }
};

#endif
