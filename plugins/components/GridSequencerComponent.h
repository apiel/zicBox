#ifndef _UI_COMPONENT_GRID_SEQUENCER_H_
#define _UI_COMPONENT_GRID_SEQUENCER_H_

#include "../audio/stepInterface.h"
#include "../controllers/keypadInterface.h"

#include "base/Grid.h"
#include "component.h"

class Track {
public:
    int16_t trackId = -1;
    AudioPlugin* seqPlugin;
    std::string name = "Init";
    float volume = rand() % 100 / 100.0f;
    bool active = false;
    Step* steps;
    ValueInterface* selectedStep;

    void load(int16_t id, AudioPlugin& _seqPlugin)
    {
        trackId = id;
        seqPlugin = &_seqPlugin;
        selectedStep = _seqPlugin.getValue("SELECTED_STEP");
        steps = (Step*)seqPlugin->data(0); // TODO make this configurable...
        name = "Track " + std::to_string(id + 1);
    }
};

class GridSequencerComponent : public Component {
protected:
    KeypadInterface* keypad;

    std::string prefixSampleParamsView = "SampleParams_track_";
    std::string prefixStepParamsView = "StepParams_track_";

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

    uint16_t initViewCounter = -1;

    Track tracks[trackCount]; // FIXME

    Step selectedStepCopy;

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

    std::string lastView;
    void updateSelection()
    {
        renderSelection();
        std::string view;
        if (grid.row == trackCount) {
            view = "MasterParams";
        } else if (grid.col == 0) {
            view = prefixSampleParamsView + std::to_string(grid.row + 1);
        } else {
            view = prefixStepParamsView + std::to_string(grid.row + 1);
            tracks[grid.row].selectedStep->set(grid.col - 1);

            selectedStepCopy = tracks[grid.row].steps[grid.col - 1];

            // printf("Selected step: %d enable %f = %s\n",
            //     grid.col - 1,
            //     tracks[grid.row].seqPlugin->getValue("STEP_ENABLED")->get(),
            //     tracks[grid.row].steps[grid.col - 1].enabled ? "ON" : "OFF");
        }

        if (view != lastView) {
            lastView = view;
            setView(view);
        }
    }

    void renderStep(Track& track, unsigned int step, unsigned int row)
    {
        int y = rowY[row];
        int x = firstColumnWidth + itemW * step;
        Color color = colors.step;
        // printf("................[%d] step enabled: %d\n", step, track.steps[step].enabled);
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

        uint8_t master = 14;

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

            keypad->setButton(keys.master, 40);
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

        for (int16_t i = 0; i < 12; i++) {
            tracks[i].load(i, getPlugin("Sequencer", i + 1));
        }

        jobRendering = [this](unsigned long now) {
            if (grid.row < trackCount && grid.col > 0 && !tracks[grid.row].steps[grid.col - 1].equal(selectedStepCopy)) {
                renderSelection(grid.row, grid.col, colors.selector);
                renderNext();
                selectedStepCopy = tracks[grid.row].steps[grid.col - 1];
            }
        };
    }

    void initView(uint16_t counter)
    {
        // Do not initialize if it was previously initialized
        if (initViewCounter != counter - 1) {
            renderKeypad();
        }
        initViewCounter = counter;
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
            if (id == keys.master) {
                grid.select(trackCount, 0);
                updateSelection();
                draw.renderNext();
            } else if (id == keys.up) {
                grid.up();
                updateSelection();
                draw.renderNext();
            } else if (id == keys.down) {
                grid.down();
                updateSelection();
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
                        updateSelection();
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
