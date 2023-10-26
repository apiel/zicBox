#ifndef _SEQUENCER_H_
#define _SEQUENCER_H_

#include <math.h>
#include <stdio.h> // file
#include <sys/stat.h> // file exist
#include <time.h>

#include "audioPlugin.h"
#include "mapping.h"
#include "stepInterface.h"

int randCounter = 0;
int getRand()
{
    srand(time(NULL) + randCounter++);
    return rand();
}

struct StepCondition {
    const char* name;
    bool (*conditionMet)(uint8_t loopCounter);
} stepConditions[] = {
    { "---", [](uint8_t loopCounter) { return true; } },
    { "Pair", [](uint8_t loopCounter) { return loopCounter % 2 == 0; } },
    { "4th", [](uint8_t loopCounter) { return loopCounter % 4 == 0; } },
    { "6th", [](uint8_t loopCounter) { return loopCounter % 6 == 0; } },
    { "8th", [](uint8_t loopCounter) { return loopCounter % 8 == 0; } },
    { "Impair", [](uint8_t loopCounter) { return loopCounter % 2 == 1; } },
    { "1%", [](uint8_t loopCounter) { return (getRand() % 100) == 0; } },
    { "2%", [](uint8_t loopCounter) { return (getRand() % 100) < 2; } },
    { "5%", [](uint8_t loopCounter) { return (getRand() % 100) < 5; } },
    { "10%", [](uint8_t loopCounter) { return (getRand() % 100) < 10; } },
    { "20%", [](uint8_t loopCounter) { return (getRand() % 100) < 20; } },
    { "30%", [](uint8_t loopCounter) { return (getRand() % 100) < 30; } },
    { "40%", [](uint8_t loopCounter) { return (getRand() % 100) < 40; } },
    { "50%", [](uint8_t loopCounter) { return (getRand() % 100) < 50; } },
    { "60%", [](uint8_t loopCounter) { return (getRand() % 100) < 60; } },
    { "70%", [](uint8_t loopCounter) { return (getRand() % 100) < 70; } },
    { "80%", [](uint8_t loopCounter) { return (getRand() % 100) < 80; } },
    { "90%", [](uint8_t loopCounter) { return (getRand() % 100) < 90; } },
    { "95%", [](uint8_t loopCounter) { return (getRand() % 100) < 95; } },
    { "98%", [](uint8_t loopCounter) { return (getRand() % 100) < 98; } },
    { "99%", [](uint8_t loopCounter) { return (getRand() % 100) < 99; } },
};

const char* MIDI_NOTES_STR[132] = {
    // clang-format off
    "C-1", "C#-1", "D-1", "D#-1", "E-1", "F-1", "F#-1", "G-1", "G#-1", "A-1", "A#-1", "B-1",
    "C0", "C#0", "D0", "D#0", "E0", "F0", "F#0", "G0", "G#0", "A0", "A#0", "B0",
    "C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1", "A1", "A#1", "B1",
    "C2", "C#2", "D2", "D#2", "E2", "F2", "F#2", "G2", "G#2", "A2", "A#2", "B2",
    "C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3", "A3", "A#3", "B3",
    "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4",
    "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5", "A5", "A#5", "B5",
    "C6", "C#6", "D6", "D#6", "E6", "F6", "F#6", "G6", "G#6", "A6", "A#6", "B6",
    "C7", "C#7", "D7", "D#7", "E7", "F7", "F#7", "G7", "G#7", "A7", "A#7", "B7",
    "C8", "C#8", "D8", "D#8", "E8", "F8", "F#8", "G8", "G#8", "A8", "A#8", "B8",
    "C9", "C#9", "D9", "D#9", "E9", "F9", "F#9", "G9", "G#9", "A9", "A#9", "B9",
    // clang-format on
};

uint8_t MIDI_NOTE_COUNT = sizeof(MIDI_NOTES_STR) / sizeof(MIDI_NOTES_STR[0]);

uint8_t STEP_CONDITIONS_COUNT = sizeof(stepConditions) / sizeof(stepConditions[0]);
const uint8_t MAX_STEPS = 32;

class Sequencer : public Mapping<Sequencer> {
protected:
    AudioPlugin::Props& props;
    const char* folder = "../zicHost/patterns/";
    char patternFilename[255];
    Step steps[MAX_STEPS];
    Step* selectedStepPtr = &steps[0];

    uint8_t clockCounter = 0;
    uint8_t stepCounter = 0;
    uint8_t loopCounter = 0;

    bool active = false;

    AudioPlugin* targetPlugin = NULL;

    void resetStep(Step& step)
    {
        step.enabled = false;
        step.velocity = 0;
        step.condition = 0;
        step.len = 1;
        step.counter = 0;
        step.note = 60;
    }

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

    bool fileExists(const char* filename)
    {
        struct stat buffer;
        return (stat(filename, &buffer) == 0);
    }

public:
    Val<Sequencer>& detune = val(this, 0.0f, "DETUNE", &Sequencer::setDetune, { "Detune", VALUE_CENTERED, -24.0f, 24.0f });
    Val<Sequencer>& pattern = val(this, 0.0f, "PATTERN", &Sequencer::setPattern, { "Pattern" });
    Val<Sequencer>& selectedStep = val(this, 0.0f, "SELECTED_STEP", &Sequencer::setSelectedStep, { "Step", .min = 1.0f, .max = MAX_STEPS });
    Val<Sequencer>& stepVelocity = val(this, 0.0f, "STEP_VELOCITY", &Sequencer::setStepVelocity, { "Velocity" });
    Val<Sequencer>& stepLength = val(this, 0.0f, "STEP_LENGTH", &Sequencer::setStepLength, { "Len", .min = 1.0f, .max = MAX_STEPS });
    Val<Sequencer>& stepCondition = val(this, 1.0f, "STEP_CONDITION", &Sequencer::setStepCondition, { "Condition", VALUE_STRING, .min = 1.0f, .max = (float)STEP_CONDITIONS_COUNT });
    Val<Sequencer>& stepNote = val(this, 0.0f, "STEP_NOTE", &Sequencer::setStepNote, { "Note", VALUE_STRING, .min = 1.0f, .max = (float)MIDI_NOTE_COUNT });
    Val<Sequencer>& stepEnabled = val(this, 0.0f, "STEP_ENABLED", &Sequencer::setStepEnabled, { "Enabled", VALUE_STRING, .max = 1 });

    Sequencer(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
        , props(props)
    {
        // steps[0].setVelocity(1.0).setLen(8).enabled = true;
        // steps[16].setVelocity(1.0).setNote(52).setLen(16).enabled = true;
        steps[0].velocity = 1.0;
        steps[0].len = 8;
        steps[0].enabled = true;

        steps[9].velocity = 1.0;
        steps[9].len = 2;
        steps[9].note = 70;
        steps[9].enabled = true;

        steps[16].velocity = 1.0;
        steps[16].note = 52;
        steps[16].len = 16;
        steps[16].enabled = true;

        steps[31].velocity = 1.0;
        steps[31].len = 8;
        steps[31].note = 40;
        steps[31].enabled = true;
        steps[31].condition = 1;

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
        pattern.set(value);
        sprintf(patternFilename, "%s/%d.bin", folder, (uint)(pattern.get()));
        if (fileExists(patternFilename)) {
            FILE* file = fopen(patternFilename, "rb");
            fread(steps, sizeof(Step), MAX_STEPS, file);
            fclose(file);
        } else {
            for (int i = 0; i < MAX_STEPS; i++) {
                resetStep(steps[i]);
            }
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

    Sequencer& setDetune(float value)
    {
        detune.set(value);
        return *this;
    }

    void* data(int id)
    {
        switch (id) {
        case 0:
            return &steps;
        case 1:
            return &stepCounter;
        }
        return NULL;
    }

    void save()
    {
        FILE* file = fopen(patternFilename, "wb");
        fwrite(steps, sizeof(Step), MAX_STEPS, file);
        fclose(file);
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
