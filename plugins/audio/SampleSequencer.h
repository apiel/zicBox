#ifndef _SAMPLE_SEQUENCER_H_
#define _SAMPLE_SEQUENCER_H_

#include <limits>
#include <sndfile.h>
#include <string>

#include "audioPlugin.h"
#include "fileBrowser.h"
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
    uint64_t end = 0;
    float fEnd = 0.0f;
    uint64_t start = 0;
    float fStart = 0.0f;
    std::string filename = "---";
    SNDFILE* file = NULL;
    float stepIncrement = 1.0f;

    void setVelocity(float velocity)
    {
        this->velocity = range(velocity, 0.0, 1.0);
    }

    void setEnd(float value)
    {
        fEnd = range(value, fStart, 100.0);
        end = fEnd * sampleCount;
    }

    void setStart(float value)
    {
        // this->start = range(start, 0, sampleCount);
        fStart = range(value, 0.0, fEnd);
        start = fStart * sampleCount;
    }

    void setFilename(std::string filename, uint8_t channels)
    {
        printf(">>>>>> setFilename %s\n", filename.c_str());
        // Let's keep it easy for the moment each step has his own instance of SNDFILE
        // We gonna try to optimize later only if necessary by reusing the same SNDFILE
        if (file != NULL) {
            sf_close(file);
            file = NULL;
        }
        if (filename != "---") {
            SF_INFO sfinfo;
            // printf("Load filename %s\n", filename.c_str());
            file = sf_open(filename.c_str(), SFM_READ, &sfinfo);
            if (file) {
                printf(">>>>>> success sampleCount %ld\n", (long)sfinfo.frames);
                sampleCount = sfinfo.frames;
                if (sfinfo.channels < channels) {
                    stepIncrement = 0.5f;
                } else if (sfinfo.channels > channels) {
                    stepIncrement = 2.0f;
                } else {
                    stepIncrement = 1.0f;
                }
                setStart(fStart);
                setEnd(fEnd);
            } else {
                printf(">>>>>> failed\n");
                logWarn("SampleSequencer: Could not open step file %s [%s]", filename.c_str(), sf_strerror(file));
            }
        }
    }

    std::string serialize()
    {
        return std::to_string(enabled) + " "
            + fToString(velocity, 2) + " "
            + std::to_string(fStart) + " "
            + std::to_string(fEnd) + " "
            + filename;
    }

    void hydrate(std::string value, uint8_t channels)
    {
        // printf("hydrate %s\n", value.c_str());
        enabled = strtok((char*)value.c_str(), " ")[0] == '1';
        velocity = atof(strtok(NULL, " "));
        fStart = atof(strtok(NULL, " "));
        fEnd = atof(strtok(NULL, " "));
        setFilename(strtok(NULL, " "), channels);
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

    FileBrowser fileBrowser = FileBrowser("./samples");

    enum Status {
        MUTED = 0,
        ON = 1,
        NEXT = 2
    };

    void onStep()
    {
        stepCounter++;
        uint8_t state = status.get();
        // printf("[%d] stepCounter %d status %d\n", track, stepCounter, state);
        // If we reach the end of the sequence, we reset the step counter
        if (stepCounter >= MAX_STEPS) {
            stepCounter = 0;
            loopCounter++;
            if (props.audioPluginHandler) {
                props.audioPluginHandler->sendEvent(AudioEventType::SEQ_LOOP, track);
            }
            if (state == Status::NEXT) {
                status.set(Status::ON);
            }
        }
        if (state == Status::ON) {
            SampleStep& step = steps[stepCounter];
            if (step.file && step.enabled && step.velocity > 0.0f) {
                // printf("[%d] Play step %d\n", track, stepCounter);
                activeStep = &step;
                sampleIndex = step.start;
            }
        }
    }

public:
    /*md **Values**: */
    /*md - `STEP_VELOCITY` set selected step velocity */
    Val& stepVelocity = val(0.0f, "STEP_VELOCITY", { "Velocity" }, [&](auto p) {
        p.val.setFloat(p.value);
        selectedStepPtr->setVelocity(p.val.pct());
    });
    /*md - `STEP_START` set selected step sample start point */
    Val& stepStart = val(0.0f, "STEP_START", { "Start", .step = 0.1f, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        selectedStepPtr->setStart(p.val.pct());
    });
    /*md - `STEP_END` set selected step sample end point */
    Val& stepEnd = val(0.0f, "STEP_END", { "End", .step = 0.1f, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        selectedStepPtr->setEnd(p.val.pct());
    });
    /*md - `STEP_ENABLED` toggle selected step */
    Val& stepEnabled = val(0.0f, "STEP_ENABLED", { "Enabled", VALUE_STRING, .max = 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        selectedStepPtr->enabled = p.val.get() > 0.5;
        p.val.setString(selectedStepPtr->enabled ? (char*)"ON" : (char*)"OFF");
    });
    /*md - `STEP_FILENAME` select sample file for selected step */
    Val& stepFilename = val(0.0f, "STEP_FILENAME", { "Wavefile", VALUE_STRING, .min = 0.0f, .max = (float)fileBrowser.count }, [&](auto p) {
        p.val.setFloat(p.value);
        int position = p.val.get();
        if (position == 0) {
            p.val.setString("---");
            selectedStepPtr->setFilename("---", props.channels);
        } else {
            p.val.setString(fileBrowser.getFile(position));
            std::string filepath = fileBrowser.getFilePath(position);

            if (filepath != selectedStepPtr->filename || selectedStepPtr->file == NULL) {
                selectedStepPtr->setFilename(filepath, props.channels);
            }
        }
    });

    /*md - `SELECTED_STEP` select the step to edit */
    Val& selectedStep = val(0.0f, "SELECTED_STEP", { "Step", .min = 1.0f, .max = MAX_STEPS }, [&](auto p) {
        selectedStep.setFloat(p.value);
        uint8_t index = selectedStep.get() - 1;
        selectedStepPtr = &steps[index];
        // printf("Selected step: %d note: %d = %s\n", index, selectedStepPtr->note, (char*)MIDI_NOTES_STR[selectedStepPtr->note]);

        stepVelocity.set(selectedStepPtr->velocity * 100);
        stepStart.set(selectedStepPtr->fStart * 100);
        stepEnd.set(selectedStepPtr->fEnd * 100);
        stepEnabled.set(selectedStepPtr->enabled ? 1.0 : 0.0);

        if (fileBrowser.find(selectedStepPtr->filename)) {
            stepFilename.set(fileBrowser.position);
        } else {
            stepFilename.set(0.0);
        }
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

#define CHUNK_SIZE 128
    float sampleIndex = 0.0f;
    float chunkBuffer[CHUNK_SIZE];
    size_t chunkPosition = CHUNK_SIZE; // Start at CHUNK_SIZE to trigger initial load
    void sample(float* buf)
    {
        if (activeStep && activeStep->file && sampleIndex < activeStep->end) {
            if (chunkPosition >= CHUNK_SIZE) {
                sf_seek(activeStep->file, (int)sampleIndex, SEEK_SET);
                size_t samplesRead = sf_read_float(activeStep->file, chunkBuffer, CHUNK_SIZE);
                for (size_t i = samplesRead; i < CHUNK_SIZE; i++) {
                    chunkBuffer[i] = 0.0f;
                }
                chunkPosition = 0;
            }
            buf[track] = chunkBuffer[chunkPosition];
            chunkPosition++;
            sampleIndex += activeStep->stepIncrement;
        }
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
            steps[index].hydrate(strtok(NULL, ""), props.channels);
            return;
        }
        Mapping::hydrate(valCopy);
    }
};

#endif
