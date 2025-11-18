#ifndef _UI_COMPONENT_GRID_SEQUENCER_H_
#define _UI_COMPONENT_GRID_SEQUENCER_H_

#include "../../audio/stepInterface.h"

#include "../base/KeypadLayout.h"
#include "../component.h"
#include "base/Grid.h"

class Track {
public:
    uint8_t pageCount = 2;
    uint8_t page = 0;
    int16_t trackId = -1;
    AudioPlugin* seqPlugin;
    std::string name = "Init";
    ValueInterface* volume;
    ValueInterface* status;
    ValueInterface* clip;
    Step* steps;
    ValueInterface* selectedStep;
    std::string trackView = "TrackParams_track_";
    std::string stepView = "StepParams_track_";

    void load(Component* component, int16_t id)
    {
        trackId = id;
        seqPlugin = &component->getPlugin("Sequencer", id + 1);
        // volume = component->getPlugin("Mixer", -1).getValue("TRACK_" + std::to_string(id + 1));
        volume = component->getPlugin("Volume", id + 1).getValue("VOLUME");
        selectedStep = seqPlugin->getValue("SELECTED_STEP");
        status = component->watch(seqPlugin->getValue("STATUS"));
        clip = component->watch(component->getPlugin("SerializeTrack", id + 1).getValue("CLIP"));
        steps = (Step*)seqPlugin->data(0); // TODO make this configurable...
        name = "Track " + std::to_string(id + 1);

        trackView = trackView + std::to_string(id + 1);
        stepView = stepView + std::to_string(id + 1);
    }
};

/*md
## SDL GridSequencer

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/GridSequencer.png" />

Can handle sequencer per track.
The component is expecting:
- to have a sequencer audio plugin called `Sequencer` on 12 tracks.
- to have volume audio plugin called `Volume` on each track.

> [!NOTE]
> - `TODO` make volume plugin configurable and possibility to use mixer instead of single volume effect
> - `TODO` switch clip/clip
> - `TODO` keypad action to toggle page views. E.g.: track view might have more parameter than the one visible on a single views. We need a way to be able to toggle between parameter views.
> - `TODO` make sequencer plugin name configurable
> - `TODO` make track configurable
> - `TODO` touch could be used to move up/down/left/right
> - `TODO` save single track clip under a given name
> - `TODO` load a specific track clip
> - `TODO` when sequencer status is `next`, track should blink
*/
class GridSequencerComponent : public Component {
protected:
    std::vector<KeypadLayout*> keypadLayouts;
    KeypadLayout* currentKeypadLayout = NULL;

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

    int8_t masterPage = 0;
    int8_t masterPageCount = 2;

    void progressInit()
    {
        for (unsigned int step = 0; step < stepsCount; step++) {
            int x = progressPosition.x + itemW * step;
            draw.filledRect({ x, progressPosition.y }, progressItemSize, { colors.progressBg });
        }
    }

    void renderProgress(uint8_t stepCounter)
    {
        int xPrevious = progressPosition.x + itemW * ((lastStepCounter + stepsCount) % stepsCount);
        draw.filledRect({ xPrevious, progressPosition.y }, progressItemSize, { colors.progressBg });
        int x = progressPosition.x + itemW * stepCounter;
        draw.filledRect({ x, progressPosition.y }, progressItemSize, { colors.active.on });
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
            draw.rect({ firstColumnMargin, y - 1 }, { firstColumnWidth - 6, h }, { color });
        } else {
            int selectW = itemSize.w + 2;
            int x = firstColumnWidth + selectW * (col - 1);
            draw.rect({ x - 1, y - 1 }, { selectW, h }, { color });
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
        std::string viewName;
        if (grid.row == trackCount) {
            viewName = std::string("MasterParams") + "_page_" + std::to_string(masterPage);
        } else if (grid.col == 0) {
            // if (grid.lastRow != grid.row) {
            //     tracks[grid.row].page = 0;
            // }
            viewName = tracks[grid.row].trackView + "_page_" + std::to_string(tracks[grid.row].page);
        } else {
            viewName = tracks[grid.row].stepView;
            tracks[grid.row].selectedStep->set(grid.col);

            selectedStepCopy = tracks[grid.row].steps[grid.col - 1];
        }

        // printf("View: %s\n", viewName.c_str());

        if (viewName != lastView) {
            lastView = viewName;
            view->setView(viewName);
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
        draw.filledRect({ x, y }, itemSize, { color });
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
            draw.filledRect({ 0, rowY[0] }, { size.h, rowY[0] - progressPosition.y }, { colors.background });
        }
        for (unsigned int row = 0; row < trackCount; row++) {
            renderRow(row);
        }
    }

    void renderTrackName(Track& track, int y)
    {
        int w = firstColumnWidth - 8;
        draw.filledRect({ 5, y }, { w, itemSize.h }, { colors.step });

        Color trackColor = colors.firstStep;
        Color trackText = colors.track;
        if (track.status->get() == 1) {
            trackColor = colors.active.on;
            trackText = colors.active.selector;
        }
        trackColor.a = 50;
        draw.filledRect({ 5, y }, { w, itemSize.h }, { trackColor });
        trackColor.a = 200;
        int width = w * track.volume->pct();
        draw.filledRect({ 5, y }, { width, itemSize.h }, { trackColor });

        draw.text({ 8, y }, track.name.c_str(), 10, { trackText });
    }

    void renderMasterVolume(bool selected = false)
    {
        int w = firstColumnWidth - 8;
        draw.filledRect({ firstColumnMargin, progressPosition.y - 1 }, { firstColumnWidth - 6, progressItemSize.h + 2 }, { colors.background });
        Color color = colors.active.on;
        color.a = 100;
        draw.filledRect({ 5, progressPosition.y }, { w, progressItemSize.h }, { color });
        color.a = 200;
        //  int width = w * master.getVolume() / APP_MAX_VOLUME;
        int width = w * 0.9;
        draw.filledRect({ 5, progressPosition.y }, { width, progressItemSize.h }, { color });
        if (selected) {
            draw.rect({ firstColumnMargin, progressPosition.y - 1 }, { 86, progressItemSize.h + 2 }, { colors.active.selector });
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
        std::vector<void*>* components = view->getComponents();
        for (void* c : *components) {
            ComponentInterface* component = (ComponentInterface*)c;
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
        std::vector<void*>* components = view->getComponents();
        for (void* c : *components) {
            ComponentInterface* component = (ComponentInterface*)c;
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
        if (view->contextVar[0]) {
            if (state == 1) {
                tracks[track].seqPlugin->data(20);
            }
        } else if (state == 1) {
            if (grid.row == track && grid.col == 0) {
                tracks[grid.row].page = (tracks[grid.row].page + 1) % tracks[grid.row].pageCount;
                // printf("page %d from %d\n", tracks[grid.row].page, tracks[grid.row].pageCount);
            } else {
                printf("select track %d\n", track);
                grid.select(track, 0);
            }
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
                currentKeypadLayout->renderKeypadColor();
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
        } else if (!view->contextVar[0] && now - stepPressedTime < 300) {
            tracks[grid.row].seqPlugin->getValue("STEP_ENABLED")->set(tracks[grid.row].steps[param].enabled ? 0 : 1);
            currentKeypadLayout->renderKeypadColor();
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
        resize();

        for (int16_t i = 0; i < 12; i++) {
            tracks[i].load(this, i);
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
            currentKeypadLayout->renderKeypadColor();
            // }
            initViewCounter = counter;
        }
    }

    void render()
    {
        draw.filledRect(position, { size.w, size.h - 5 }, { colors.background });
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
            currentKeypadLayout = new KeypadLayout(this, [&](std::string action) {
                std::function<void(KeypadLayout::KeyMap&)> func = NULL;
                // if (action.rfind("item:") == 0) {
                //     int* paramFn = new int(atoi(action.substr(5).c_str()));
                //     func = [this, paramFn](KeypadLayout::KeyMap& keymap) { handleButton(*paramFn, keymap.pressedTime != -1); };
                // }

                /*md -- Keyboard actions: */
                /*md -- `track` to select track number: `KEYMAP: Keypad 1 track 2` will select track 2 when key 1 is pressed.*/
                if (action.rfind("track:") == 0) {
                    // currentKeypadLayout->mapping.push_back({ controller, controllerId, key, [&](int8_t state, KeypadLayout::KeyMap& keymap) { updateTrackSelection(state, *(int*)keymap.param); }, new int(atoi(param)), .getColor = [&](KeypadLayout::KeyMap& keymap) { return tracks[*(int*)keymap.param].status->get() == 1 ? 40 : 0; } });
                    int* paramFn = new int(atoi(action.substr(6).c_str()));
                    func = [this, paramFn](KeypadLayout::KeyMap& keymap) { return tracks[*paramFn].status->get() == 1 ? 40 : 0; };
                    /*md -- `param` to select parameter number: `KEYMAP: Keypad 1 param 2 20` will select parameter 2 when key 1 is pressed. Color must be specified, in this example color is 20. */
                } else if (action.rfind("param:") == 0) {
                    int* paramFn = new int(atoi(action.substr(6).c_str()));
                    func = [this, paramFn](KeypadLayout::KeyMap& keymap) { updateParamSelection(keymap.pressedTime != -1, *paramFn); };
                    /*md -- `row` to select row number: `KEYMAP: Keypad 1 row -1` will decrement the current row selection when key 1 is pressed. */
                } else if (action.find("row:") == 0) {
                    int* paramFn = new int(atoi(action.substr(4).c_str()));
                    func = [this, paramFn](KeypadLayout::KeyMap& keymap) { updateRowSelection(keymap.pressedTime != -1, *paramFn); };
                    /*md -- `col` to select column number: `KEYMAP: Keypad 1 col -1` will decrement the current column selection when key 1 is pressed. */
                } else if (action.find("col:") == 0) {
                    int* paramFn = new int(atoi(action.substr(4).c_str()));
                    func = [this, paramFn](KeypadLayout::KeyMap& keymap) { updateColSelection(keymap.pressedTime != -1, *paramFn); };
                    /*md -- `master` to select master track: `KEYMAP: Keypad 1 master` will select master when key 1 is pressed. */
                } else if (action == "master") {
                    func = [this](KeypadLayout::KeyMap& keymap) { updateMasterSelection(keymap.pressedTime != -1); };
                    /*md -- `clip` to select clip: `KEYMAP: Keypad 1 clip` will select clip when key 1 is pressed. */
                    // } else if (action == "clip") {
                    //     currentKeypadLayout->mapping.push_back({ controller, controllerId, key, [&](int8_t state, KeypadLayout::KeyMap& keymap) { tracks[grid.row].clip->set(*(int*)keymap.param); }, new int(atoi(param)), .getColor = [&](KeypadLayout::KeyMap& keymap) { return 60; } });
                } else if (action.rfind("clip:") == 0) {
                    int* paramFn = new int(atoi(action.substr(9).c_str()));
                    func = [this, paramFn](KeypadLayout::KeyMap& keymap) { tracks[grid.row].clip->set(*paramFn); };
                    /*md -- `step` to update a step: `KEYMAP: Keypad 1 step 4` will update step 4 when key 1 is pressed. */
                } else if (action.rfind("step:") == 0) {
                    int* paramFn = new int(atoi(action.substr(5).c_str()));
                    func = [this, paramFn](KeypadLayout::KeyMap& keymap) { updateStepSelection(keymap.pressedTime != -1, *paramFn); };
                    /*md -- `layout` to select a layout: `KEYMAP: Keypad 1 layout 2` will select layout 2 when key 1 is pressed. The numeric id of the layout corresponds to the order of initialization. */
                } else if (action.rfind("layout:") == 0) {
                    int* paramFn = new int(atoi(action.substr(7).c_str()));
                    func = [this, paramFn](KeypadLayout::KeyMap& keymap) { updateLayout(keymap.pressedTime != -1, *paramFn); };
                    /*md -- `none` to disable keypad button: `KEYMAP: Keypad 1 none` will disable the button 1. */
                } else if (action == "none") {
                    func = NULL;
                }
                return func;
            });
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

        /*md - `TRACK_PAGE_COUNT: track_id count` to specify the number of track pages. By default it is 2.`*/
        if (strcmp(key, "TRACK_PAGE_COUNT") == 0) {
            uint16_t track = atoi(strtok(value, " ")) - 1;
            tracks[track].pageCount = atoi(strtok(NULL, " "));
            return true;
        }

        /*md - `TRACK_NAME: track_id name` to change the name of a track.` */
        if (strcmp(key, "TRACK_NAME") == 0) {
            uint16_t track = atoi(strtok(value, " ")) - 1;
            tracks[track].name = strtok(NULL, " ");
            return true;
        }

        /*md - `TRACK_VIEW: track_id name` to set the view name of a track. By default it is `TrackParams_track_1`, `TrackParams_track_2`, ... */
        if (strcmp(key, "TRACK_VIEW") == 0) {
            uint16_t track = atoi(strtok(value, " ")) - 1;
            tracks[track].trackView = strtok(NULL, " ");
            return true;
        }

        /*md - `STEP_VIEW: track_id name` to set the view name of step editing. By default it is `StepParams_track_1`, `StepParams_track_2`, ... */
        if (strcmp(key, "STEP_VIEW") == 0) {
            uint16_t track = atoi(strtok(value, " ")) - 1;
            tracks[track].stepView = strtok(NULL, " ");
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

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        if (currentKeypadLayout) {
            currentKeypadLayout->onKey(id, key, state, now);
        }
    }
};

#endif
