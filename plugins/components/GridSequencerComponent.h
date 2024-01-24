#ifndef _UI_COMPONENT_GRID_SEQUENCER_H_
#define _UI_COMPONENT_GRID_SEQUENCER_H_

#include "../audio/stepInterface.h"

#include "base/Grid.h"
#include "base/KeypadLayout.h"
#include "component.h"

class Track {
public:
    int16_t trackId = -1;
    AudioPlugin* seqPlugin;
    std::string name = "Init";
    float volume = 1.0f; // rand() % 100 / 100.0f;
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

/*md
## GridSequencer

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/GridSequencer.png" />

Can handle sequencer per track.
*/
// TODO param color keymap is hardcoded but we might want to make this dynamic...
class GridSequencerComponent : public Component {
protected:
    std::vector<KeypadLayout*> keypadLayouts;
    KeypadLayout* currentKeypadLayout = NULL;

    std::string prefixTrackParamsView = "TrackParams_track_";
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

    uint64_t lastClockCounter = -1;
    uint8_t lastStepCounter = -1;

    unsigned long now = 0;

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
            view = prefixTrackParamsView + std::to_string(grid.row + 1);
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
        if (state == 1) {
            printf("select track %d\n", track);
            grid.select(track, 0);
            updateSelection();
            draw.renderNext();
        }
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
            grid.select(trackCount, 0);
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
        if (grid.row >= trackCount) {
            return;
        }
        if (state == 1) {
            stepPressedTime = now;
            grid.lastCol = grid.col;
            grid.col = param + 1;
            updateSelection();
            draw.renderNext();
        } else if (now - stepPressedTime < 300) {
            tracks[grid.row].seqPlugin->getValue("STEP_ENABLED")->set(tracks[grid.row].steps[param].enabled ? 0 : 1);
            currentKeypadLayout->renderKeypad();
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
    /*md **Keyboard actions**: */
    void addKeyMap(uint8_t key, int param, std::string action, uint8_t color)
    {
        if (!currentKeypadLayout) {
            printf("No keypad layout\n");
            return;
        }
        /*md - `track` to select track number: `KEYMAP: 1 track 2` will select track 2 when key 1 is pressed.*/
        if (action == "track") {
            currentKeypadLayout->mapping.push_back({ key, param, color, [&](KeypadLayout::KeyMap& keymap) { return tracks[keymap.param].active ? 40 : 0; }, [&](int8_t state, KeypadLayout::KeyMap& keymap) { updateTrackSelection(state, keymap.param); } });
            /*md - `param` to select parameter number: `KEYMAP: 1 param 2 20` will select parameter 2 when key 1 is pressed. Color must be specified, in this example color is 20. */
        } else if (action == "param") {
            currentKeypadLayout->mapping.push_back({ key, param, color, [&](KeypadLayout::KeyMap& keymap) { return keymap.color; }, [&](int8_t state, KeypadLayout::KeyMap& keymap) { updateParamSelection(state, keymap.param); } });
            /*md - `row` to select row number: `KEYMAP: 1 row -1` will decrement the current row selection when key 1 is pressed. */
        } else if (action == "row") {
            currentKeypadLayout->mapping.push_back({ key, param, color, [&](KeypadLayout::KeyMap& keymap) { return 20; }, [&](int8_t state, KeypadLayout::KeyMap& keymap) { updateRowSelection(state, keymap.param); } });
            /*md - `col` to select column number: `KEYMAP: 1 col -1` will decrement the current column selection when key 1 is pressed. */
        } else if (action == "col") {
            currentKeypadLayout->mapping.push_back({ key, param, color, [&](KeypadLayout::KeyMap& keymap) { return 20; }, [&](int8_t state, KeypadLayout::KeyMap& keymap) { updateColSelection(state, keymap.param); } });
            /*md - `master` to select master track: `KEYMAP: 1 master` will select master when key 1 is pressed. */
        } else if (action == "master") {
            currentKeypadLayout->mapping.push_back({ key, param, color, [&](KeypadLayout::KeyMap& keymap) { return 40; }, [&](int8_t state, KeypadLayout::KeyMap& keymap) { updateMasterSelection(state); } });
            /*md - `clip` to select clip: `KEYMAP: 1 clip` will select clip when key 1 is pressed. */
        } else if (action == "clip") {
            currentKeypadLayout->mapping.push_back({ key, param, color, [&](KeypadLayout::KeyMap& keymap) { return 60; }, [&](int8_t state, KeypadLayout::KeyMap& keymap) {} });
            /*md - `step` to update a step: `KEYMAP: 1 step 4` will update step 4 when key 1 is pressed. */
        } else if (action == "step") {
            currentKeypadLayout->mapping.push_back({ key, param, color, [&](KeypadLayout::KeyMap& keymap) { 
                if (grid.row >= trackCount) {
                    return 254;
                }
                return tracks[grid.row].steps[keymap.param].enabled ? 21 : 20; }, [&](int8_t state, KeypadLayout::KeyMap& keymap) { updateStepSelection(state, keymap.param); } });
            /*md - `layout` to select a layout: `KEYMAP: 1 layout 2` will select layout 2 when key 1 is pressed. The numeric id of the layout corresponds to the order of initialization. */
        } else if (action == "layout") {
            currentKeypadLayout->mapping.push_back({ key, param, color, [&](KeypadLayout::KeyMap& keymap) { return keymap.color == 255 ? 90 : keymap.color; }, [&](int8_t state, KeypadLayout::KeyMap& keymap) { updateLayout(state, keymap.param); } });
            /*md - `none` to disable keypad button: `KEYMAP: 1 none` will disable the button 1. */
        } else if (action == "none") {
            currentKeypadLayout->mapping.push_back({ key, param, color, [&](KeypadLayout::KeyMap& keymap) { return 254; }, [&](int8_t state, KeypadLayout::KeyMap& keymap) {} });
        }
    }

    GridSequencerComponent(ComponentInterface::Props props)
        : Component(props)
    {
        resize();

        for (int16_t i = 0; i < 12; i++) {
            tracks[i].load(i, getPlugin("Sequencer", i + 1));
        }

        jobRendering = [this](unsigned long _now) {
            now = _now;
            if (grid.row < trackCount && grid.col > 0 && !tracks[grid.row].steps[grid.col - 1].equal(selectedStepCopy)) {
                renderSelection(grid.row, grid.col, colors.selector);
                renderNext();
                selectedStepCopy = tracks[grid.row].steps[grid.col - 1];
            }

            if (tracks[0].seqPlugin) {
                uint64_t* clockCounterPtr = (uint64_t*)tracks[0].seqPlugin->data(3);
                if (clockCounterPtr && lastClockCounter != *clockCounterPtr) {
                    lastClockCounter = *clockCounterPtr;
                    uint8_t stepCounter = lastClockCounter / 6 % stepsCount;
                    if (stepCounter != lastStepCounter) {
                        // printf("stepCounter: %d (%ld)\n", stepCounter, lastClockCounter);
                        renderProgress(stepCounter);
                        lastStepCounter = stepCounter;
                        draw.renderNext();
                    }
                }
            }
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
        draw.filledRect(position, size, colors.background);
        progressInit();
        renderMasterVolume();
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

        /*md - `KEYPAD_LAYOUT: layout` inititates a keypad layout */
        if (strcmp(key, "KEYPAD_LAYOUT") == 0) {
            currentKeypadLayout = new KeypadLayout((KeypadInterface*)getController("Keypad"), [&](int8_t state, int param, std::string action, uint8_t color) { addKeyMap(state, param, action, color); });
            currentKeypadLayout->name = value;
            keypadLayouts.push_back(currentKeypadLayout);
            return true;
        }

        /*md - `SELECTED_LAYOUT: name` to change the selected keypad layout. By default the last initiated keypad layout is selected. */
        if (strcmp(key, "SELECTED_LAYOUT") == 0) {
            for (KeypadLayout* keypadLayout : keypadLayouts) {
                if (strcmp(keypadLayout->name.c_str(), value) == 0) {
                    currentKeypadLayout = keypadLayout;
                    return true;
                }
            }
            return true;
        }

        if (currentKeypadLayout) {
            return currentKeypadLayout->config(key, value);
        }
        return false;
    }

    void onKeyPad(int key, int8_t state)
    {
        if (currentKeypadLayout) {
            currentKeypadLayout->onKeypad(key, state);
        }
    }
};

#endif
