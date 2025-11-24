/** Description:
## UI Component Analysis: GridMini Sequencer Interface

This C++ header defines the structure and logic for a crucial visual element called `GridMiniComponent`. Its primary function is to serve as a compact, interactive interface for controlling a complex digital music sequencer.

### Core Function and Structure

The component organizes sequencing data into a grid layout, typically managing 8 separate **Tracks**, each containing up to 32 rhythmic **Steps**.

1.  **Track Management:** A specialized structure within the component holds all necessary data for one musical line—its name, volume, playback status, and links to the specific sound-generating elements (audio plugins) responsible for executing the sequence.
2.  **Visualization:** The component is responsible for drawing this grid on the screen. It visually distinguishes between active steps, conditional steps, and inactive steps. It also displays volume meters for each track and renders a precise progress bar at the bottom to show the current beat being played.
3.  **Interaction:** It handles user input via a defined Keypad Layout. Users can navigate the grid to select a specific track (to adjust track-level parameters like volume) or select an individual step (to edit the note or timing data). Dedicated keys are also handled for master playback control (start/stop) and track switching.
4.  **Integration:** The component is deeply integrated with the audio engine. It constantly monitors the state of required audio plugins—specifically "Sequencer" and "Volume" plugins—to accurately reflect the sound status and sequence progress in real-time.

In essence, `GridMiniComponent` translates complex sequential music data into a navigable, visual grid, providing the user with direct control over the sequencing and playback of multiple musical parts.

sha: 9eb27b153ab51882f38c3d32edc882a134cfbcb623eba8ebee66aac6d39a4e9c 
*/
#ifndef _UI_COMPONENT_GRID_MINI_H_
#define _UI_COMPONENT_GRID_MINI_H_

#include "../../audio/stepInterface.h"

#include "../base/KeypadLayout.h"
#include "../component.h"
#include "base/Grid.h"

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
    ValueInterface* clip;
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
        clip = component->watch(component->getPlugin("SerializeTrack", id).getValue("CLIP"));
        steps = (Step*)seqPlugin->data(0); // TODO make this configurable...
        stepView = "StepParams_track_" + std::to_string(id);
    }
};

/*md
## SDL GridMini

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/GridMini.png" />

Can handle sequencer per track.
The component is expecting:
- to have a sequencer audio plugin called `Sequencer` on 8 tracks.
- to have volume audio plugin called `Volume` on each track.

*/
class GridMiniComponent : public Component {
protected:
    KeypadLayout keypadLayout;

    int firstColumnWidth = 48;

    Size itemSize;
    int itemW;
    int itemMargin = 1;

    uint8_t trackCount = 8;
    const static uint8_t stepsCount = 32;

    Grid grid = Grid(trackCount + 1, stepsCount + 1);

    Point progressPosition = { 0, 0 };
    Size progressItemSize = { 0, 5 };

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
        int y = row == trackCount ? progressPosition.y : tracks[row].y;
        uint8_t h = itemSize.h + 2;
        if (row == trackCount) { // Select volume/master
            h = progressItemSize.h + 2;
            col = 0;
        }

        if (col == 0) {
            draw.rect({ position.x, y - 1 }, { firstColumnWidth, h }, { color });
        } else {
            int selectW = itemSize.w + 1;
            int x = firstColumnWidth + selectW * (col - 1);
            draw.rect({ x - 1, y - 1 }, { selectW + 1, h }, { color });
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
        draw.filledRect({ x, y }, itemSize, { color });
    }

    void renderRow(unsigned int row)
    {
        Track& track = tracks[row];
        renderTrackName(track);

        for (unsigned int step = 0; step < stepsCount; step++) {
            renderStep(track, step, row);
        }
    }

    void renderRows(bool clear = false)
    {
        if (clear) {
            draw.filledRect({ 0, tracks[0].y }, { size.h, tracks[0].y - progressPosition.y }, { colors.background });
        }
        for (unsigned int row = 0; row < trackCount; row++) {
            renderRow(row);
        }
    }

    void renderTrackName(Track& track)
    {
        int w = firstColumnWidth - 2;
        draw.filledRect({ position.x + 1, track.y }, { w, itemSize.h }, { colors.step });

        Color trackColor = colors.firstStep;
        Color trackText = colors.track;
        if (track.status->get() == 1) {
            trackColor = colors.active.on;
            trackText = colors.active.selector;
        }
        trackColor.a = 50;
        draw.filledRect({ position.x + 1, track.y }, { w, itemSize.h }, { trackColor });
        trackColor.a = 200;
        int width = w * track.volume->pct();
        draw.filledRect({ position.x + 1, track.y }, { width, itemSize.h }, { trackColor });

        draw.text({ position.x + 2, track.y }, track.name.c_str(), 9, { trackText });
    }

    void renderMasterVolume(bool selected = false)
    {
        int w = firstColumnWidth - 1;
        draw.filledRect({ position.x, progressPosition.y - 1 }, { firstColumnWidth, progressItemSize.h + 2 }, { colors.background });
        Color color = colors.active.on;
        color.a = 100;
        draw.filledRect({ position.x, progressPosition.y }, { w, progressItemSize.h }, { color });
        color.a = 200;
        int width = w * 0.9; // FIXME !!
        draw.filledRect({ position.x, progressPosition.y }, { width, progressItemSize.h }, { color });
        if (selected) {
            draw.rect({ position.x, progressPosition.y - 1 }, { 86, progressItemSize.h + 2 }, { colors.active.selector });
        }
    }

    void resize()
    {
        progressPosition = { firstColumnWidth, position.y + size.h - progressItemSize.h };
        itemSize.w = (size.w - firstColumnWidth) / stepsCount - itemMargin;
        itemW = itemSize.w + itemMargin;
        progressItemSize.w = itemSize.w;
        itemSize.h = (progressPosition.y - position.y) / trackCount - itemMargin;

        for (unsigned int i = 0; i < tracks.size(); i++) {
            tracks[i].y = position.y + (itemSize.h + itemMargin) * i;
        }

        // Ajust progress bar size base itemSize.h round.
        progressPosition.y = tracks[tracks.size() - 1].y + itemSize.h + itemMargin;
    }

    ComponentInterface* componentParam = NULL;

    void updateTrackSelection(int8_t state, uint8_t track)
    {
        if (state == 1) {
            if (grid.row == track && grid.col == 0) {
                printf("track %d next page\n", track);
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

    void updateRowSelection(int8_t state, int8_t direction)
    {
        if (state == 1) {
            grid.selectNextRow(-direction);
            updateSelection();
            draw.renderNext();
        }
    }
    void updateColSelection(int8_t state, int8_t direction)
    {
        if (state == 1) {
            grid.selectNextCol(direction);
            updateSelection();
            draw.renderNext();
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

    unsigned long playPressedTime = 0;
    void handlePlay(int8_t state)
    {
        if (grid.row == trackCount) {
            if (state == 0) {
                sendAudioEvent(now - playPressedTime > 500 ? AudioEventType::STOP : AudioEventType::TOGGLE_PLAY_PAUSE, -1);
            } else {
                playPressedTime = now;
            }
        } else {
            // toggle track
            // Long press should let select in how many loop it should start / stop
            // Actually this could as well work for master stop / pause
            // Short press just toggle right away
        }
    }

    void handleKeyTrackController(int8_t state, int8_t keyId)
    {
        if (keyId == 5) {
            updateMasterSelection(state);
        } else if (keyId > 5) {
            updateTrackSelection(state, keyId - 2); // from 6 to 9, need to remove 2, since master was in between...
        } else {
            updateTrackSelection(state, keyId - 1); // from 1 to 4, need to remove 1
        }
    }

    int8_t pressedKey = -1;
    void handleKey10Controller(int8_t state, int8_t keyId)
    {
        if (state == 0 && pressedKey == keyId) {
            pressedKey = -1;
            return;
        }
        if (pressedKey != -1) {
            switch (pressedKey) {
            case 0:
                handleKeyTrackController(state, keyId);
                return;
            }
        }

        switch (keyId) {
        case 0:
            pressedKey = 0;
            break;
        case 1:
            updateRowSelection(state, 1);
            break;

        case 2:
            break;

        case 3:
            handlePlay(state);
            break;

        case 4:
            break;

            // next row
        case 5:
            updateColSelection(state, -1);
            break;

        case 6:
            updateRowSelection(state, -1);
            break;

        case 7:
            updateColSelection(state, 1);
            break;

        case 8:
            break;

        case 9:
            break;
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
    GridMiniComponent(ComponentInterface::Props props)
        : Component(props)
        , keypadLayout(this, [&](std::string action) {
            std::function<void(KeypadLayout::KeyMap&)> func = NULL;
            /*md **Keyboard actions**: */
            /*md - `key10` is used to assign one of the 10 keys for the 10 buttons control of GridMini. */
            if (action.rfind("key10:") == 0) {
                int* id = new int(atoi(action.substr(6).c_str()));
                func = [&, id](KeypadLayout::KeyMap& keymap) { handleKey10Controller(keymap.pressedTime != -1, *id); };
            }
            return func;
        })
    {
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
        // Do not initialize if it was previously initialized
        // if (initViewCounter != counter - 1) {
        keypadLayout.renderKeypadColor();
        // }
        initViewCounter = counter;
    }

    void render()
    {
        draw.filledRect(position, { size.w, size.h - 5 }, { colors.background });
        progressInit();
        renderMasterVolume();
        renderRows();
        renderSelection();
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

        if (keypadLayout.config(key, value)) {
            return true;
        }

        return false;
    }

    void onKey(uint16_t id, int key, int8_t state, unsigned long now)
    {
        keypadLayout.onKey(id, key, state, now);
    }
};

#endif
