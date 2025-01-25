#ifndef _SAMPLE_SEQUENCER_H_
#define _SAMPLE_SEQUENCER_H_

#include <sndfile.h>
#include <string>
#include <limits>

#include "audioPlugin.h"
#include "helpers/midiNote.h"
#include "log.h"
#include "mapping.h"
#include "stepInterface.h"

const uint8_t MAX_STEPS = 32;

class SampleStep {
public:
    bool enabled = false;
    float velocity = 0.8f;
    uint64_t sampleCount = 0;
    uint64_t len = 0;
    uint64_t start = 0;
    std::string filename;
    SNDFILE* file = NULL;

    void setVelocity(float velocity)
    {
        this->velocity = range(velocity, 0.0, 1.0);
    }

    void setLength(uint64_t len)
    {
        this->len = range(len, start, sampleCount);
    }

    void setStart(uint64_t start)
    {
        this->start = range(start, 0, sampleCount);
    }

    std::string serialize()
    {
        return std::to_string(enabled) + " "
            + fToString(velocity, 2) + " "
            + std::to_string(sampleCount)
            + " " + filename;
    }

    void hydrate(std::string value)
    {
        // printf("hydrate %s\n", value.c_str());
        enabled = strtok((char*)value.c_str(), " ")[0] == '1';
        velocity = atof(strtok(NULL, " "));
        sampleCount = atoi(strtok(NULL, " "));
        filename = strtok(NULL, " ");
    }
};

class SampleSequencer : public Mapping {
protected:
    AudioPlugin::Props& props;

    SampleStep steps[MAX_STEPS];
    SampleStep* activeStep = NULL;
    SampleStep* selectedStepPtr = &steps[0];

    uint8_t stepCounter = 0;
    bool isPlaying = false;
    uint8_t loopCounter = 0;
    uint64_t* clockCounterPtr = NULL;

    AudioPlugin* targetPlugin = NULL;

    enum Status {
        MUTED = 0,
        ON = 1,
        NEXT = 2
    };

    void onStep()
    {
        stepCounter++;
        // printf("[%d] stepCounter %d\n", track, stepCounter);
        uint8_t state = status.get();
        // If we reach the end of the sequence, we reset the step counter
        if (stepCounter >= MAX_STEPS) {
            stepCounter = 0;
            loopCounter++;
            props.audioPluginHandler->sendEvent(AudioEventType::SEQ_LOOP, track);
            if (state == Status::NEXT) {
                status.set(Status::ON);
            }

            if (state == Status::ON) {
                SampleStep& step = steps[stepCounter];
                if (step.file && step.enabled && step.velocity > 0.0f) {
                    activeStep = &step;
                }
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
    Val& stepVelocity = val(0.0f, "STEP_VELOCITY", { "Velocity" }, [&](auto p) {
        p.val.setFloat(p.value);
        selectedStepPtr->velocity = p.val.pct();
    });
    /*md - `STEP_LENGTH` set selected step length */
    Val& stepLength = val(0.0f, "STEP_LENGTH", { "Len", .min = 1.0f, .max = std::numeric_limits<float>::max() }, [&](auto p) { 
        p.val.setFloat(p.value);
        selectedStepPtr->sampleCount = p.val.get();
    });
    /*md - `STEP_ENABLED` toggle selected step */
    Val& stepEnabled = val(0.0f, "STEP_ENABLED", { "Enabled", VALUE_STRING, .max = 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        selectedStepPtr->enabled = p.val.get() > 0.5;
        p.val.setString(selectedStepPtr->enabled ? (char*)"ON" : (char*)"OFF");
    });
    /*md - `STATUS` set status: off, on, next. Default: off
    Play/Stop will answer to global event. However, you may want to the sequencer to not listen to those events or to only start to play on the next sequence iteration. */
    Val& status = val(1.0f, "STATUS", { "Status", VALUE_STRING, .max = 2 }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() == Status::MUTED) {
            p.val.setString("muted");
        } else if (p.val.get() == Status::ON) {
            p.val.setString("on");
        } else {
            p.val.setString("next");
        }
    });

    SampleSequencer(AudioPlugin::Props& props, char* _name)
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
        activeStep = NULL;
    }

    void onEvent(AudioEventType event, bool playing) override
    {
        isPlaying = playing;
        // if (event != AudioEventType::AUTOSAVE) logDebug("[%d] event %d seq is playing %d", track, event, isPlaying);
        if (event == AudioEventType::STOP) {
            // logDebug("in sequencer event STOP\n");
            stepCounter = 0;
            allOff();
        } else if (event == AudioEventType::PAUSE) {
            allOff();
        }
    }

    void sample(float* buf)
    {
    }

    void setSelectedStep(float value)
    {
        selectedStep.setFloat(value);
        uint8_t index = selectedStep.get() - 1;
        selectedStepPtr = &steps[index];
        // printf("Selected step: %d note: %d = %s\n", index, selectedStepPtr->note, (char*)MIDI_NOTES_STR[selectedStepPtr->note]);

        stepVelocity.set(selectedStepPtr->velocity * 100);
        stepEnabled.set(selectedStepPtr->enabled ? 1.0 : 0.0);
    }

    // enum DATA_ID {
    //     STEPS,
    //     STEP_COUNTER,
    //     IS_PLAYING,
    //     STEP_CONDITION,
    //     CLOCK_COUNTER,
    //     GET_STEP,
    //     SELECTED_STEP_PTR,
    //     STEP_TOGGLE
    // };

    // /*md **Data ID**: */
    // uint8_t getDataId(std::string name) override
    // {
    //     /*md - `STEPS` return steps */
    //     if (name == "STEPS")
    //         return DATA_ID::STEPS;
    //     /*md - `STEP_COUNTER` return current played step */
    //     if (name == "STEP_COUNTER")
    //         return DATA_ID::STEP_COUNTER;
    //     /*md - `IS_PLAYING` return is playing */
    //     if (name == "IS_PLAYING")
    //         return DATA_ID::IS_PLAYING;
    //     /*md - `STEP_CONDITION` return current step condition */
    //     if (name == "STEP_CONDITION")
    //         return DATA_ID::STEP_CONDITION;
    //     /*md - `CLOCK_COUNTER` return clock counter */
    //     if (name == "CLOCK_COUNTER")
    //         return DATA_ID::CLOCK_COUNTER;
    //     /*md - `GET_STEP` get step by index */
    //     if (name == "GET_STEP")
    //         return DATA_ID::GET_STEP;
    //     /*md - `SELECTED_STEP_PTR` return selected step pointer */
    //     if (name == "SELECTED_STEP_PTR")
    //         return DATA_ID::SELECTED_STEP_PTR;
    //     /*md - `STEP_TOGGLE` toggle selected step */
    //     if (name == "STEP_TOGGLE")
    //         return DATA_ID::STEP_TOGGLE;
    //     return atoi(name.c_str());
    // }

    // void* data(int id, void* userdata = NULL)
    // {
    //     switch (id) {
    //     case DATA_ID::STEPS:
    //         return &steps;
    //     case DATA_ID::STEP_COUNTER:
    //         return &stepCounter;
    //     case DATA_ID::IS_PLAYING:
    //         return &isPlaying;
    //     case DATA_ID::STEP_CONDITION: { // Get step condition name
    //         uint8_t* index = (uint8_t*)userdata;
    //         return (void*)stepConditions[*index].name;
    //     }
    //     case DATA_ID::CLOCK_COUNTER:
    //         return clockCounterPtr;
    //     case DATA_ID::GET_STEP: {
    //         uint8_t* index = (uint8_t*)userdata;
    //         if (*index >= MAX_STEPS) {
    //             return NULL;
    //         }
    //         return &steps[*index];
    //     }
    //     case DATA_ID::SELECTED_STEP_PTR: { // Toggle sequencer
    //         if (status.get() == Status::ON) {
    //             status.set(Status::MUTED);
    //         } else {
    //             status.set(Status::ON);
    //         }
    //         return NULL;
    //     }
    //     case DATA_ID::STEP_TOGGLE: // Step toggle
    //         stepEnabled.set(selectedStepPtr->enabled ? 0.0 : 1.0);
    //         return NULL;
    //     }
    //     return NULL;
    // }

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
