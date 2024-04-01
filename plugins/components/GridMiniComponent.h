#ifndef _UI_COMPONENT_GRID_MINI_H_
#define _UI_COMPONENT_GRID_MINI_H_

#include "../audio/stepInterface.h"

#include "base/Grid.h"
#include "base/KeypadLayout.h"
#include "component.h"

#include <vector>

class Track {
public:
    int y = 0;
    uint8_t pageCount = 2;
    uint8_t page = 0;
    int16_t id = -1;
    AudioPlugin* seqPlugin;
    std::string name = "Init";
    ValueInterface* volume;
    ValueInterface* status;
    ValueInterface* variation;
    Step* steps;
    ValueInterface* selectedStep;
    std::string trackView = "TrackParams_track_0";
    std::string stepView = "StepParams_track_0";

    void load(Component* component)
    {
        seqPlugin = &component->getPlugin("Sequencer", id);
        volume = component->getPlugin("Volume", id).getValue("VOLUME");
        selectedStep = seqPlugin->getValue("SELECTED_STEP");
        status = component->watch(seqPlugin->getValue("STATUS"));
        variation = component->watch(component->getPlugin("SerializeTrack", id).getValue("VARIATION"));
        steps = (Step*)seqPlugin->data(0); // TODO make this configurable...
        stepView = "StepParams_track_" + std::to_string(id);
    }
};

/*md
## GridMini

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/GridMini.png" />

Can handle sequencer per track.
The component is expecting:
- to have a sequencer audio plugin called `Sequencer` on 8 tracks.
- to have volume audio plugin called `Volume` on each track.

*/
class GridMiniComponent : public Component {
protected:
    std::vector<KeypadLayout*> keypadLayouts;
    KeypadLayout* currentKeypadLayout = NULL;

    int firstColumnWidth = 40;

    Size itemSize;
    int itemW;
    int itemMargin = 1;
    int progressMarginY = 3;

    uint8_t trackCount = 8;
    const static uint8_t stepsCount = 32;

    Grid grid = Grid(trackCount + 1, stepsCount + 1);

    Point progressPosition = { 0, 0 };
    Size progressItemSize = { 0, 5 };

    int rowY[99];
    // int rowY[trackCount + 1];
    // std::vector<int> rowY;

    uint16_t initViewCounter = -1;

    std::vector<Track> tracks;

    Step selectedStepCopy;

    uint64_t lastClockCounter = -1;
    uint8_t lastStepCounter = -1;

    unsigned long now = 0;

    int8_t masterPage = 0;
    int8_t masterPageCount = 2;

    void progressInit()
    {
        for (unsigned int step = 0; step < stepsCount; step++) {
            int x = progressPosition.x + itemW * step;
            draw.filledRect({ x, progressPosition.y }, progressItemSize, colors.progressBg);
        }
    }

    void renderProgress(uint8_t stepCounter)
    {
        int xPrevious = progressPosition.x + itemW * ((lastStepCounter + stepsCount) % stepsCount);
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
            // draw.rect({ firstColumnMargin, y - 1 }, { firstColumnWidth - 6, h }, color);
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
        // renderSelection();
        // std::string view;
        // if (grid.row == trackCount) {
        //     view = std::string("MasterParams") + "_page_" + std::to_string(masterPage);
        // } else if (grid.col == 0) {
        //     // if (grid.lastRow != grid.row) {
        //     //     tracks[grid.row].page = 0;
        //     // }
        //     view = tracks[grid.row].trackView + "_page_" + std::to_string(tracks[grid.row].page);
        // } else {
        //     view = tracks[grid.row].stepView;
        //     tracks[grid.row].selectedStep->set(grid.col);

        //     selectedStepCopy = tracks[grid.row].steps[grid.col - 1];
        // }

        // // printf("View: %s\n", view.c_str());

        // if (view != lastView) {
        //     lastView = view;
        //     setView(view);
        // }
    }

    void renderStep(Track& track, unsigned int step, unsigned int row)
    {
        int y = track.y;
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
        renderTrackName(track);

        for (unsigned int step = 0; step < stepsCount; step++) {
            renderStep(track, step, row);
        }
        // renderSelection();
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

    void renderTrackName(Track& track)
    {
        int w = firstColumnWidth - 1;
        draw.filledRect({ position.x, track.y }, { w, itemSize.h }, colors.step);

        Color trackColor = colors.firstStep;
        Color trackText = colors.track;
        if (track.status->get() == 1) {
            trackColor = colors.active.on;
            trackText = colors.active.selector;
        }
        trackColor.a = 50;
        draw.filledRect({ position.x, track.y }, { w, itemSize.h }, trackColor);
        trackColor.a = 200;
        int width = w * track.volume->pct();
        draw.filledRect({ position.x, track.y }, { width, itemSize.h }, trackColor);

        draw.text({ position.x + 2, track.y }, track.name.c_str(), trackText, 9);
    }

    // void renderMasterVolume(bool selected = false)
    // {
    //     int w = firstColumnWidth - 8;
    //     draw.filledRect({ firstColumnMargin, progressPosition.y - 1 }, { firstColumnWidth - 6, progressItemSize.h + 2 }, colors.background);
    //     Color color = colors.active.on;
    //     color.a = 100;
    //     draw.filledRect({ 5, progressPosition.y }, { w, progressItemSize.h }, color);
    //     color.a = 200;
    //     //  int width = w * master.getVolume() / APP_MAX_VOLUME;
    //     int width = w * 0.9;
    //     draw.filledRect({ 5, progressPosition.y }, { width, progressItemSize.h }, color);
    //     if (selected) {
    //         draw.rect({ firstColumnMargin, progressPosition.y - 1 }, { 86, progressItemSize.h + 2 }, colors.active.selector);
    //     }
    // }

    void resize()
    {
        progressPosition = { firstColumnWidth, position.y + size.h - progressItemSize.h - progressMarginY };
        itemSize.w = (size.w - firstColumnWidth) / stepsCount - itemMargin;
        itemW = itemSize.w + itemMargin;
        progressItemSize.w = itemSize.w;
        itemSize.h = (progressPosition.y - position.y - progressMarginY) / trackCount - itemMargin;

        for (unsigned int i = 0; i < tracks.size(); i++) {
            tracks[i].y = position.y + (itemSize.h + itemMargin) * i;
        }

        // Ajust progress bar size base itemSize.h round.
        progressPosition.y = tracks[tracks.size() - 1].y + itemSize.h + itemMargin + progressMarginY;
        // progressItemSize.h = position.y + size.h - progressMarginY - progressPosition.y;

        rowY[trackCount] = progressPosition.y;
    }

    int8_t paramKeyPressed = -1;
    ComponentInterface* componentParam = NULL;

    void onParamKeyPressed(uint8_t param)
    {
        std::string paramId = "Param" + std::to_string(param) + "_";
        std::vector<ComponentInterface*> components = getViewComponents();
        for (ComponentInterface* component : components) {
            if (component->id.find(paramId) == 0) {
                paramKeyPressed = param;
                // id end by _toggle
                if (component->id.find("_button") != -1) {
                    // printf("toggle %s\n", component->id.c_str());
                    component->data(1);
                } else if (component->id.find("_enc") != -1) {
                    // printf("enc %s\n", component->id.c_str());
                    componentParam = component;
                }
            }
        }
    }

    void onParamKeyReleased(uint8_t param)
    {
        std::string paramId = "Param" + std::to_string(param) + "_";
        std::vector<ComponentInterface*> components = getViewComponents();
        for (ComponentInterface* component : components) {
            if (component->id.find(paramId) == 0) {
                // id end by _toggle
                if (component->id.find("_button") != -1) {
                    // printf("toggle %s\n", component->id.c_str());
                    component->data(0);
                }
            }
        }
        paramKeyPressed = -1;
    }

    void updateTrackSelection(int8_t state, uint8_t track)
    {
        // if (shift) {
        //     if (state == 1) {
        //         tracks[track].seqPlugin->data(20);
        //     }
        // } else if (state == 1) {
        //     if (grid.row == track && grid.col == 0) {
        //         tracks[grid.row].page = (tracks[grid.row].page + 1) % tracks[grid.row].pageCount;
        //         // printf("page %d from %d\n", tracks[grid.row].page, tracks[grid.row].pageCount);
        //     } else {
        //         printf("select track %d\n", track);
        //         grid.select(track, 0);
        //     }
        //     updateSelection();
        //     draw.renderNext();
        // }
    }

    void updateParamSelection(int8_t state, uint8_t param)
    {
        if (state == 1) {
            onParamKeyPressed(param);
        } else if (paramKeyPressed == param) {
            onParamKeyReleased(paramKeyPressed);
        }
    }

    void updateRowSelection(int8_t state, int8_t direction)
    {
        if (state == 1) {
            if (paramKeyPressed == -1) {
                grid.selectNextRow(-direction);
                updateSelection();
                draw.renderNext();
            } else {
                int8_t step = direction > 0 ? 1 : -1;
                componentParam->data(0, &step);
            }
        }
    }
    void updateColSelection(int8_t state, int8_t direction)
    {
        if (state == 1) {
            if (paramKeyPressed == -1) {
                grid.selectNextCol(direction);
                updateSelection();
                draw.renderNext();
            } else {
                int8_t step = direction > 0 ? 5 : -5;
                componentParam->data(0, &step);
            }
        }
    }

    void updateMasterSelection(int8_t state)
    {
        if (state == 1) {
            if (grid.row == trackCount) {
                masterPage = (masterPage + 1) % masterPageCount;
            } else {
                grid.select(trackCount, 0);
            }
            updateSelection();
            draw.renderNext();
        }
    }

    void updateLayout(int8_t state, uint8_t param)
    {
        if (state == 1) {
            if (param < keypadLayouts.size()) {
                currentKeypadLayout = keypadLayouts[param];
                currentKeypadLayout->renderKeypad();
            }
        }
    }

    long stepPressedTime = 0;
    void updateStepSelection(int8_t state, int8_t param)
    {
        // if (grid.row >= trackCount) {
        //     return;
        // }
        // if (state == 1) {
        //     stepPressedTime = now;
        //     grid.lastCol = grid.col;
        //     grid.col = param + 1;
        //     updateSelection();
        //     draw.renderNext();
        // } else if (!shift && now - stepPressedTime < 300) {
        //     tracks[grid.row].seqPlugin->getValue("STEP_ENABLED")->set(tracks[grid.row].steps[param].enabled ? 0 : 1);
        //     currentKeypadLayout->renderKeypad();
        // }
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
    /*md **Keyboard actions**: */
    void addKeyMap(KeypadInterface* controller, uint16_t controllerId, uint8_t key, int param, std::string action, uint8_t color)
    {
        if (!currentKeypadLayout) {
            printf("No keypad layout\n");
            return;
        }
        // /*md - `track` to select track number: `KEYMAP: Keypad 1 track 2` will select track 2 when key 1 is pressed.*/
        // if (action == "track") {
        //     currentKeypadLayout->mapping.push_back({ controller, controllerId, key, param, [&](int8_t state, KeypadLayout::KeyMap& keymap) { updateTrackSelection(state, keymap.param); }, color, [&](KeypadLayout::KeyMap& keymap) { return tracks[keymap.param].status->get() == 1 ? 40 : 0; } });
        //     /*md - `param` to select parameter number: `KEYMAP: Keypad 1 param 2 20` will select parameter 2 when key 1 is pressed. Color must be specified, in this example color is 20. */
        // } else if (action == "param") {
        //     currentKeypadLayout->mapping.push_back({ controller, controllerId, key, param, [&](int8_t state, KeypadLayout::KeyMap& keymap) { updateParamSelection(state, keymap.param); }, color, [&](KeypadLayout::KeyMap& keymap) { return keymap.color; } });
        //     /*md - `row` to select row number: `KEYMAP: Keypad 1 row -1` will decrement the current row selection when key 1 is pressed. */
        // } else if (action == "row") {
        //     currentKeypadLayout->mapping.push_back({ controller, controllerId, key, param, [&](int8_t state, KeypadLayout::KeyMap& keymap) { updateRowSelection(state, keymap.param); }, color, [&](KeypadLayout::KeyMap& keymap) { return 20; } });
        //     /*md - `col` to select column number: `KEYMAP: Keypad 1 col -1` will decrement the current column selection when key 1 is pressed. */
        // } else if (action == "col") {
        //     currentKeypadLayout->mapping.push_back({ controller, controllerId, key, param, [&](int8_t state, KeypadLayout::KeyMap& keymap) { updateColSelection(state, keymap.param); }, color, [&](KeypadLayout::KeyMap& keymap) { return 20; } });
        //     /*md - `master` to select master track: `KEYMAP: Keypad 1 master` will select master when key 1 is pressed. */
        // } else if (action == "master") {
        //     currentKeypadLayout->mapping.push_back({ controller, controllerId, key, param, [&](int8_t state, KeypadLayout::KeyMap& keymap) { updateMasterSelection(state); }, color, [&](KeypadLayout::KeyMap& keymap) { return 40; } });
        //     /*md - `variation` to select variation: `KEYMAP: Keypad 1 variation` will select variation when key 1 is pressed. */
        // } else if (action == "variation") {
        //     currentKeypadLayout->mapping.push_back({ controller, controllerId, key, param, [&](int8_t state, KeypadLayout::KeyMap& keymap) { tracks[grid.row].variation->set(keymap.param); }, color, [&](KeypadLayout::KeyMap& keymap) { return 60; } });
        //     /*md - `step` to update a step: `KEYMAP: Keypad 1 step 4` will update step 4 when key 1 is pressed. */
        // } else if (action == "step") {
        //     currentKeypadLayout->mapping.push_back({ controller, controllerId, key, param, [&](int8_t state, KeypadLayout::KeyMap& keymap) { updateStepSelection(state, keymap.param); }, color, [&](KeypadLayout::KeyMap& keymap) {
        //         if (grid.row >= trackCount) {
        //             return 254;
        //         }
        //         return tracks[grid.row].steps[keymap.param].enabled ? 21 : 20; } });
        //     /*md - `layout` to select a layout: `KEYMAP: Keypad 1 layout 2` will select layout 2 when key 1 is pressed. The numeric id of the layout corresponds to the order of initialization. */
        // } else if (action == "layout") {
        //     currentKeypadLayout->mapping.push_back({ controller, controllerId, key, param, [&](int8_t state, KeypadLayout::KeyMap& keymap) { updateLayout(state, keymap.param); }, color, [&](KeypadLayout::KeyMap& keymap) { return keymap.color == 255 ? 90 : keymap.color; } });
        //     /*md - `none` to disable keypad button: `KEYMAP: Keypad 1 none` will disable the button 1. */
        // } else if (action == "none") {
        //     currentKeypadLayout->mapping.push_back({ controller, controllerId, key, param, [&](int8_t state, KeypadLayout::KeyMap& keymap) {}, color, [&](KeypadLayout::KeyMap& keymap) { return 254; } });
        // }
    }

    GridMiniComponent(ComponentInterface::Props props)
        : Component(props)
    {
        jobRendering = [this](unsigned long _now) {
            now = _now;
            // if (grid.row < trackCount && grid.col > 0 && !tracks[grid.row].steps[grid.col - 1].equal(selectedStepCopy)) {
            //     renderSelection(grid.row, grid.col, colors.selector);
            //     renderNext();
            //     selectedStepCopy = tracks[grid.row].steps[grid.col - 1];
            // }

            // if (tracks[0].seqPlugin) {
            //     uint64_t* clockCounterPtr = (uint64_t*)tracks[0].seqPlugin->data(3);
            //     if (clockCounterPtr && lastClockCounter != *clockCounterPtr) {
            //         lastClockCounter = *clockCounterPtr;
            //         uint8_t stepCounter = lastClockCounter / 6 % stepsCount;
            //         if (stepCounter != lastStepCounter) {
            //             // printf("stepCounter: %d (%ld)\n", stepCounter, lastClockCounter);
            //             renderProgress(stepCounter);
            //             lastStepCounter = stepCounter;
            //             draw.renderNext();
            //         }
            //     }
            // }
        };
    }

    void initView(uint16_t counter)
    {
        if (currentKeypadLayout) {
            // Do not initialize if it was previously initialized
            // if (initViewCounter != counter - 1) {
            currentKeypadLayout->renderKeypad();
            // }
            initViewCounter = counter;
        }
    }

    void render()
    {
        draw.filledRect(position, { size.w, size.h - 5 }, colors.background);
        // progressInit();
        // renderMasterVolume();
        renderRows();
    }

    /*md **Configs**: */
    bool config(char* key, char* value)
    {
        /*md - `FIRST_COLUMN_WIDTH: width` to specify the width of the first column (track column) */
        if (strcmp(key, "FIRST_COLUMN_WIDTH") == 0) {
            firstColumnWidth = atoi(value);
            resize();
            return true;
        }

        /*md - `ITEM: track_id name param_view page_count [step_view]` to change the name of a track `ITEM: 1 Synth synth_1 4`. */
        if (strcmp(key, "ITEM") == 0) {
            Track track; // = new Track();
            track.id = atoi(strtok(value, " "));
            track.name = strtok(NULL, " ");
            track.pageCount = atoi(strtok(NULL, " "));
            track.trackView = strtok(NULL, " ");
            track.load(this);
            char* stepView = strtok(NULL, " ");
            if (stepView) {
                track.stepView = stepView;
            }
            tracks.push_back(track);
            resize();
            return true;
        }

        /*md - `MASTER_PAGE_COUNT: count` to specify the number of master pages. */
        if (strcmp(key, "MASTER_PAGE_COUNT") == 0) {
            masterPageCount = atoi(value);
            return true;
        }

        if (currentKeypadLayout) {
            return currentKeypadLayout->config(key, value);
        }
        return false;
    }

    void onKey(uint16_t id, int key, int8_t state)
    {
        if (currentKeypadLayout) {
            currentKeypadLayout->onKey(id, key, state);
        }
    }
};

#endif
