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
#include "plugins/audio/SampleStep.h"
#include "stepInterface.h"

const uint8_t MAX_STEPS = 32;

class SampleSequencer : public Mapping {
protected:
    AudioPlugin::Props& props;

    SampleStep steps[MAX_STEPS];
    SampleStep* activeStep = NULL;
    SampleStep* selectedStepPtr = &steps[0];

    // uint8_t stepCounter = 31;
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

        stepVelocity.set(selectedStepPtr->velocity * 100);
        stepStart.set(selectedStepPtr->fStart * 100);
        stepEnd.set(selectedStepPtr->fEnd * 100);
        stepEnabled.set(selectedStepPtr->enabled ? 1.0 : 0.0);
        stepFilename.set(fileBrowser.find(selectedStepPtr->filename));
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

    void onClockTick(uint64_t* clockCounter) override
    {
        // printf("[%d] sampleSeq onClockTick %ld\n", track, *clockCounter);
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
        if (event == AudioEventType::STOP) {
            stepCounter = 0;
            allOff();
        } else if (event == AudioEventType::PAUSE) {
            allOff();
        }
    }

#define CHUNK_SIZE 256
    float sampleIndex = 0.0f;
    float chunkBuffer[CHUNK_SIZE];
    size_t chunkPosition = CHUNK_SIZE; // Start at CHUNK_SIZE to trigger initial load
    void sample(float* buf) override
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

    bool config(char* key, char* value) override
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

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

    DataFn dataFunctions[3] = {
        { "GET_STEP", [this](void* userdata) {
             uint8_t* index = (uint8_t*)userdata;
             return &steps[*index >= MAX_STEPS ? 0 : *index];
         } },
        { "NEXT_FILE", [this](void* userdata) {
             uint8_t* index = (uint8_t*)userdata;
             SampleStep* step = &steps[*index >= MAX_STEPS ? 0 : *index];
             uint16_t pos = fileBrowser.next(step->filename);
             step->setFilename(fileBrowser.getFilePath(pos), props.channels);
             return (void*)NULL;
         } },
        { "PREVIOUS_FILE", [this](void* userdata) {
             uint8_t* index = (uint8_t*)userdata;
             SampleStep* step = &steps[*index >= MAX_STEPS ? 0 : *index];
             uint16_t pos = fileBrowser.next(step->filename, -1);
             step->setFilename(fileBrowser.getFilePath(pos), props.channels);
             return (void*)NULL;
         } },
    };
    DEFINE_GETDATAID_AND_DATA
};

#endif
