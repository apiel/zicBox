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

class Sequencer : public Mapping<Sequencer> {
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
    Val<Sequencer>& detune = val(this, 0.0f, "DETUNE", &Sequencer::setDetune, { "Detune", VALUE_CENTERED, -24.0f, 24.0f });
    Val<Sequencer>& pattern = val(this, 0.0f, "PATTERN", &Sequencer::setPattern, { "Pattern", .type = VALUE_STRING });

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

    void onStatus(Status status)
    {
        switch (status) {
        case AudioPlugin::Status::STOP: {
            active = false;
            for (int i = 0; i < MAX_STEPS; i++) {
                if (targetPlugin && steps[i].counter) {
                    targetPlugin->noteOff(steps[i].note, 0);
                    steps[i].counter = 0;
                }
            }
            break;
        }
        case AudioPlugin::Status::START:
            clockCounter = 0;
            stepCounter = 0;
            loopCounter = 0;
            active = true;
            break;

        case AudioPlugin::Status::PAUSE:
            active = !active;
            break;
        }
    }

    void sample(float* buf)
    {
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

    Sequencer& setDetune(float value)
    {
        detune.setFloat(value);
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
            targetPlugin = &props.audioPluginHandler->getPlugin(value);
            return true;
        }
        return false;
    }
};

#endif
