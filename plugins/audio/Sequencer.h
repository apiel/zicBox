#ifndef _SEQUENCER_H_
#define _SEQUENCER_H_

#include <string>

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

    Step steps[MAX_STEPS];
    Step* selectedStepPtr = &steps[0];

    uint8_t stepCounter = 0;
    uint8_t loopCounter = 0;
    uint64_t* clockCounterPtr = NULL;

    AudioPlugin* targetPlugin = NULL;

    enum Status {
        OFF = 0,
        ON = 1,
        NEXT = 2
    };

    bool conditionMet(Step& step)
    {
        return stepConditions[step.condition].conditionMet(loopCounter);
    }

    uint8_t motion(Step& step)
    {
        return stepMotions[step.motion].get(loopCounter);
    }

    uint8_t getNote(Step& step)
    {
        return step.note + motion(step);
    }

    void onStep()
    {
        stepCounter++;
        uint8_t state = status.get();
        if (stepCounter >= MAX_STEPS) {
            stepCounter = 0;
            loopCounter++;
            if (state == Status::NEXT) {
                status.set(Status::ON);
            }
        }
        if (state == Status::ON) {
            for (int i = 0; i < MAX_STEPS; i++) {
                Step& step = steps[i];
                if (step.counter) {
                    step.counter--;
                    if (step.counter == 0) {
                        props.audioPluginHandler->noteOff(getNote(step), 0, { track, targetPlugin });
                    }
                }
            }
            Step& step = steps[stepCounter];
            if (step.enabled && conditionMet(step)) {
                step.counter = step.len;
                props.audioPluginHandler->noteOn(getNote(step), step.velocity, { track, targetPlugin });
            }
        }
    }

public:
    /*md **Values**: */
    /*md - `DETUNE` detuning all playing step notes by semitones */
    Val& detune = val(0.0f, "DETUNE", { "Detune", VALUE_CENTERED, -24.0f, 24.0f });

    /*md - `SELECTED_STEP` select the step to edit */
    Val& selectedStep = val(0.0f, "SELECTED_STEP", { "Step", .min = 1.0f, .max = MAX_STEPS }, [&](auto p) { setSelectedStep(p.value); });
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
    /*md - `STATUS` set status: off, on, next. Default: off
    Play/Stop will answer to global event. However, you may want to the sequencer to not listen to those events or to only start to play on the next sequence iteration. */
    Val& status = val(1.0f, "STATUS", { "Status", VALUE_STRING, .max = 2 }, [&](auto p) { setStatus(p.value); });

    Sequencer(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
        , props(props)
    {
        initValues();
    }

    void onClockTick(uint64_t* clockCounter)
    {
        clockCounterPtr = clockCounter;
        // Clock events are sent at a rate of 24 pulses per quarter note
        // (24/4 = 6)
        if (*clockCounter % 6 == 0) {
            onStep();
        }
    }

    void allOff()
    {
        for (int i = 0; i < MAX_STEPS; i++) {
            if (targetPlugin && steps[i].counter) {
                printf("should trigger note off %d\n", steps[i].note);
                targetPlugin->noteOff(steps[i].note, 0);
                steps[i].counter = 0;
            }
        }
    }

    void onEvent(AudioEventType event)
    {
        if (event == AudioEventType::STOP) {
            printf("in sequencer event STOP\n");
            stepCounter = 0;
            allOff();
        } else if (event == AudioEventType::PAUSE) {
            allOff();
        }
    }

    void sample(float* buf)
    {
    }

    void setStatus(float value)
    {
        status.setFloat(value);
        if (status.get() == Status::OFF) {
            status.setString("off");
            onEvent(AudioEventType::STOP);
        } else if (status.get() == Status::ON) {
            status.setString("on");
        } else {
            status.setString("next");
        }
    }

    // void reset()
    // {
    //     for (int i = 0; i < MAX_STEPS; i++) {
    //         steps[i].reset();
    //     }
    // }

    Sequencer& setSelectedStep(float value)
    {
        selectedStep.setFloat(value);
        uint8_t index = selectedStep.get() - 1;
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
        case 3:
            return clockCounterPtr;
        case 4: {
            uint8_t* index = (uint8_t*)userdata;
            if (*index >= MAX_STEPS) {
                return NULL;
            }
            return &steps[*index];
        }
        case 20: { // Toggle sequencer
            if (status.get() == Status::ON) {
                status.set(Status::OFF);
            } else {
                status.set(Status::ON);
            }
            return NULL;
        }
        case 21: // Step toggle
            stepEnabled.set(selectedStepPtr->enabled ? 0.0 : 1.0);
            return NULL;
        }
        return NULL;
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
        fprintf(file, "STATUS %f%s", status.get(), separator.c_str());
    }

    void hydrate(std::string value) override
    {
        std::string valCopy = value;
        char* key = strtok((char*)value.c_str(), " ");
        if (strcmp(key, "STEP") == 0) {
            int index = atoi(strtok(NULL, " "));
            steps[index].hydrate(strtok(NULL, ""));
            return;
        }
        Mapping::hydrate(valCopy);
    }
};

#endif
