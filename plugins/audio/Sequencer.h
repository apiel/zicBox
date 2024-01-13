#ifndef _SEQUENCER_H_
#define _SEQUENCER_H_

#include <filesystem>
#include <string>

#include "../../helpers/fs/directoryList.h"
#include "../../helpers/midiNote.h"
#include "audioPlugin.h"
#include "mapping.h"
#include "stepInterface.h"

const uint8_t MAX_STEPS = 32;

    // { "---", [](uint8_t loopCounter) { return true; } },
    // { "Pair", [](uint8_t loopCounter) { return loopCounter % 2 == 0; } },
    // { "4th", [](uint8_t loopCounter) { return loopCounter % 4 == 0; } },
    // { "6th", [](uint8_t loopCounter) { return loopCounter % 6 == 0; } },
    // { "8th", [](uint8_t loopCounter) { return loopCounter % 8 == 0; } },
    // { "Impair", [](uint8_t loopCounter) { return loopCounter % 2 == 1; } },
    // { "1%", [](uint8_t loopCounter) { return (getRand() % 100) == 0; } },
    // { "2%", [](uint8_t loopCounter) { return (getRand() % 100) < 2; } },
    // { "5%", [](uint8_t loopCounter) { return (getRand() % 100) < 5; } },
    // { "10%", [](uint8_t loopCounter) { return (getRand() % 100) < 10; } },
    // { "20%", [](uint8_t loopCounter) { return (getRand() % 100) < 20; } },
    // { "30%", [](uint8_t loopCounter) { return (getRand() % 100) < 30; } },
    // { "40%", [](uint8_t loopCounter) { return (getRand() % 100) < 40; } },
    // { "50%", [](uint8_t loopCounter) { return (getRand() % 100) < 50; } },
    // { "60%", [](uint8_t loopCounter) { return (getRand() % 100) < 60; } },
    // { "70%", [](uint8_t loopCounter) { return (getRand() % 100) < 70; } },
    // { "80%", [](uint8_t loopCounter) { return (getRand() % 100) < 80; } },
    // { "90%", [](uint8_t loopCounter) { return (getRand() % 100) < 90; } },
    // { "95%", [](uint8_t loopCounter) { return (getRand() % 100) < 95; } },
    // { "98%", [](uint8_t loopCounter) { return (getRand() % 100) < 98; } },
    // { "99%", [](uint8_t loopCounter) { return (getRand() % 100) < 99; } },


/*md
## Sequencer

Sequencer audio module can be used to sequence another audio plugin on 32 steps. Each step can be triggered by a condition:

- `---` - always
- `Pair` - every second step
- `4th` - every fourth step
- `6th` - every sixth step
- `8th` - every eighth step
- `Impair` - every impair step
- `1%` - 1% probability, meaning that it has 1 chance over 100 to be triggered
- `2%` - 2% probability
- `5%` - 5% probability
- `10%` - 10% probability
- `20%` - 20% probability
- `30%` - 30% probability
- `40%` - 40% probability
- `50%` - 50% probability
- `60%` - 60% probability
- `70%` - 70% probability
- `80%` - 80% probability
- `90%` - 90% probability
- `95%` - 95% probability
- `98%` - 98% probability
- `99%` - 99% probability

*/
class Sequencer : public Mapping {
protected:
    AudioPlugin::Props& props;
    std::filesystem::path folder = "./patterns";
    std::vector<std::filesystem::path> patternList;

    Step steps[MAX_STEPS];
    Step* selectedStepPtr = &steps[0];

    uint8_t clockCounter = 0;
    uint8_t stepCounter = 0;
    uint8_t loopCounter = 0;

    bool active = false;

    AudioPlugin* targetPlugin = NULL;

    bool conditionMet(Step& step)
    {
        return stepConditions[step.condition].conditionMet(loopCounter);
    }

    void onStep()
    {
        if (active) {
            stepCounter++;
            if (stepCounter >= MAX_STEPS) {
                stepCounter = 0;
                loopCounter++;
            }
            for (int i = 0; i < MAX_STEPS; i++) {
                Step& step = steps[i];
                if (step.counter) {
                    step.counter--;
                    if (targetPlugin && step.counter == 0) {
                        targetPlugin->noteOff(step.note, 0);
                    }
                }
            }
            Step& step = steps[stepCounter];
            if (targetPlugin && step.enabled && conditionMet(step)) {
                step.counter = step.len;
                targetPlugin->noteOn(step.note, step.velocity);
            }
        }
    }

    void refreshPatternList(std::filesystem::path pathNew = "")
    {
        patternList = getDirectoryList(folder);
        pattern.props().max = patternList.size() - 1;
        if (pathNew == "") {
            pattern.set(pattern.get());
        } else {
            std::vector<std::filesystem::path>::iterator it = std::find(patternList.begin(), patternList.end(), pathNew);
            int index = it - patternList.begin();
            pattern.set(index);
        }
    }

public:
    /*md ### Values */
    /*md - `DETUNE` detuning all playing step notes by semitones */
    Val& detune = val(0.0f, "DETUNE", { "Detune", VALUE_CENTERED, -24.0f, 24.0f });
    /*md - `PATTERN` select the pattern to play */
    Val& pattern = val(0.0f, "PATTERN", { "Pattern", .type = VALUE_STRING }, [&](auto p) { setPattern(p.value); });
    /*md - `PLAY_STOP` toggle play/stop event */
    Val& playStop = val(0.0f, "PLAY_STOP", { "Play/Stop", .max = 1.0f }, [&](auto p) { setPlayStop(p.value); });

    /*md - `SELECTED_STEP` select the step to edit */
    Val& selectedStep = val(0.0f, "SELECTED_STEP", { "Step", .max = MAX_STEPS }, [&](auto p) { setSelectedStep(p.value); });
    /*md - `STEP_VELOCITY` set selected step velocity */
    Val& stepVelocity = val(0.0f, "STEP_VELOCITY", { "Velocity" }, [&](auto p) { setStepVelocity(p.value); });
    /*md - `STEP_LENGTH` set selected step length */
    Val& stepLength = val(0.0f, "STEP_LENGTH", { "Len", .min = 1.0f, .max = MAX_STEPS }, [&](auto p) { setStepLength(p.value); });
    /*md - `STEP_CONDITION` set selected step condition */
    Val& stepCondition = val(1.0f, "STEP_CONDITION", { "Condition", VALUE_STRING, .min = 1.0f, .max = (float)STEP_CONDITIONS_COUNT }, [&](auto p) { setStepCondition(p.value); });
    /*md - `STEP_NOTE` set selected step note */
    Val& stepNote = val(0.0f, "STEP_NOTE", { "Note", VALUE_STRING, .min = 1.0f, .max = (float)MIDI_NOTE_COUNT }, [&](auto p) { setStepNote(p.value); });
    /*md - `STEP_ENABLED` toggle selected step */
    Val& stepEnabled = val(0.0f, "STEP_ENABLED", { "Enabled", VALUE_STRING, .max = 1 }, [&](auto p) { setStepEnabled(p.value); });

    Sequencer(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
        , props(props)
    {
        // steps[0].velocity = 1.0;
        // steps[0].len = 8;
        // steps[0].enabled = true;

        // steps[9].velocity = 1.0;
        // steps[9].len = 2;
        // steps[9].note = 70;
        // steps[9].enabled = true;

        // steps[16].velocity = 1.0;
        // steps[16].note = 52;
        // steps[16].len = 16;
        // steps[16].enabled = true;

        // steps[31].velocity = 1.0;
        // steps[31].len = 8;
        // steps[31].note = 40;
        // steps[31].enabled = true;
        // steps[31].condition = 1;

        refreshPatternList();

        // std::vector<std::filesystem::path> list = getDirectoryList(folder);
        // for (const auto& entry : list) {
        //     printf(" - %s\n", entry.c_str());
        // }

        // save();
        // load can be done using setPattern

        setSelectedStep(selectedStep.get());
    }

    void onClockTick()
    {
        clockCounter++;
        // Clock events are sent at a rate of 24 pulses per quarter note
        // (24/4 = 6)
        if (clockCounter >= 6) {
            clockCounter = 0;
            onStep();
        }
    }

    void onEvent(AudioEventType event)
    {
        switch (event) {
        case AudioEventType::STOP: {
            active = false;
            for (int i = 0; i < MAX_STEPS; i++) {
                if (targetPlugin && steps[i].counter) {
                    targetPlugin->noteOff(steps[i].note, 0);
                    steps[i].counter = 0;
                }
            }
            break;
        }
        case AudioEventType::START:
            clockCounter = 0;
            stepCounter = 0;
            loopCounter = 0;
            active = true;
            break;

        case AudioEventType::PAUSE:
            active = !active;
            break;
        }
    }

    void sample(float* buf)
    {
    }

    void setPlayStop(float value)
    {
        printf("setPlayStop %f\n", value);
        playStop.setFloat(value);
        if (playStop.get()) {
            onEvent(AudioEventType::START);
        } else {
            onEvent(AudioEventType::STOP);
        }
    }

    Sequencer& setPattern(float value)
    {
        pattern.setFloat(value);
        std::filesystem::path path = patternList[(int)pattern.get()];
        pattern.setString(path.filename());
        if (std::filesystem::exists(path)) {
            if (std::filesystem::file_size(path) != sizeof(Step) * MAX_STEPS) {
                printf("File %s is not the correct format (should be %ld bytes)\n", path.filename().c_str(), sizeof(Step) * MAX_STEPS);
            } else {
                // printf("Loading %s\n", patternFilename);
                FILE* file = fopen(path.c_str(), "rb");
                fread(steps, sizeof(Step), MAX_STEPS, file);
                fclose(file);
                return *this;
            }
        }

        for (int i = 0; i < MAX_STEPS; i++) {
            steps[i].reset();
        }
        return *this;
    }

    Sequencer& setSelectedStep(float value)
    {
        selectedStep.setFloat(value);
        uint8_t index = selectedStep.get();
        selectedStepPtr = &steps[index];
        // printf("Selected step: %d note: %d = %s\n", index, selectedStepPtr->note, (char*)MIDI_NOTES_STR[selectedStepPtr->note]);

        stepVelocity.set(selectedStepPtr->velocity * 100);
        stepLength.set(selectedStepPtr->len);
        stepCondition.set(selectedStepPtr->condition + 1);
        stepNote.set(selectedStepPtr->note);
        stepEnabled.set(selectedStepPtr->enabled ? 1.0 : 0.0);

        return *this;
    }

    Sequencer& setStepNote(float value)
    {
        stepNote.setFloat(value);
        selectedStepPtr->note = stepNote.get();
        stepNote.setString((char*)MIDI_NOTES_STR[selectedStepPtr->note]);
        // printf("Note: %d = %s\n", selectedStepPtr->note, (char*)MIDI_NOTES_STR[selectedStepPtr->note]);
        return *this;
    }

    Sequencer& setStepLength(float value)
    {
        stepLength.setFloat(value);
        selectedStepPtr->len = stepLength.get();
        return *this;
    }

    Sequencer& setStepVelocity(float value)
    {
        stepVelocity.setFloat(value);
        selectedStepPtr->velocity = stepVelocity.pct();
        return *this;
    }

    Sequencer& setStepCondition(float value)
    {
        stepCondition.setFloat(value);
        selectedStepPtr->condition = stepCondition.get() - 1;
        stepCondition.setString((char*)stepConditions[selectedStepPtr->condition].name);
        return *this;
    }

    Sequencer& setStepEnabled(float value)
    {
        stepEnabled.setFloat(value);
        selectedStepPtr->enabled = stepEnabled.get() > 0.5;
        stepEnabled.setString(selectedStepPtr->enabled ? (char*)"ON" : (char*)"OFF");
        return *this;
    }

    void* data(int id, void* userdata = NULL)
    {
        switch (id) {
        case 0:
            return &steps;
        case 1:
            return &stepCounter;
        case 2: { // Get step condition name
            uint8_t* index = (uint8_t*)userdata;
            return (void*)stepConditions[*index].name;
        }
        case 3: // Save pattern
            save(folder / *(std::string*)userdata);
            return NULL;
        case 4: // Rename pattern
            rename(folder / *(std::string*)userdata);
            return NULL;
        // case 5: // Toggle play/pause
        //     active = !active;
        //     return NULL;
        }
        return NULL;
    }

    void rename(std::filesystem::path pathNew)
    {
        std::filesystem::path path = patternList[(int)pattern.get()];
        std::filesystem::rename(path, pathNew);
        save(pathNew);
    }

    void save(std::filesystem::path path)
    {
        FILE* file = fopen(path.c_str(), "wb");
        fwrite(steps, sizeof(Step), MAX_STEPS, file);
        fclose(file);

        refreshPatternList(path);
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "TARGET") == 0) {
            targetPlugin = &props.audioPluginHandler->getPlugin(value, track);
            return true;
        }
        return AudioPlugin::config(key, value);
    }

    void serialize(FILE* file, std::string separator) override
    {
        for (int i = 0; i < MAX_STEPS; i++) {
            fprintf(file, "STEP %d %s %s", i, steps[i].serialize().c_str(), separator.c_str());
        }
        // AudioPlugin::serialize(file, separator);
    }

    void hydrate(std::string value) override
    {
        char* key = strtok((char*)value.c_str(), " ");
        if (strcmp(key, "STEP") == 0) {
            int index = atoi(strtok(NULL, " "));
            steps[index].hydrate(strtok(NULL, ""));
            return;
        }
        // AudioPlugin::hydrate(value);
    }
};

#endif
