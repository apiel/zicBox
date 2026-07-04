/** Description:
This document describes the technical definition for a component known as a `SampleSequencer`. This tool acts as a dedicated rhythmic pattern generator, similar to a sophisticated drum machine, whose purpose is to play back specific audio files (samples) in a timed sequence.

The Sequencer operates by managing a series of fixed rhythmic "steps." It integrates with a global timing system (clock). Every time the clock pulses, the sequencer advances to the next step. If that step is enabled and configured with an audio file, the component triggers the playback of that sound.

Users can finely control the behavior of the sequence:
1.  **Step Customization:** Controls allow selecting which specific step to edit. For that step, users can adjust the volume (velocity), precisely define the start and end points of the sample (cropping the sound), and enable or disable the step entirely.
2.  **Sample Management:** A built-in file browser facilitates assigning different audio files to each step, managing the selection of wave files from a library.
3.  **Operation Status:** The sequencer can be configured to be actively playing, muted, or set to start playback only upon the next sequence loop, allowing for synchronized transitions.
4.  **Persistence:** All settings, including the status and individual parameters for every rhythmic step, can be saved and restored, ensuring that complex patterns are remembered between user sessions.

sha: bccf10b0130e45570bc4f7bba6195e6e78ed36e8d2c26bc3cb7160884a8210d6 
*/
#pragma once

#include <limits>
#include <sndfile.h>
#include <string>

#include "Tempo.h"
#include "audioPlugin.h"
#include "helpers/midiNote.h"
#include "log.h"
#include "mapping.h"
#include "plugins/audio/SampleStep.h"
#include "stepInterface.h"
#include "audio/fileBrowser.h"

class SampleSequencer : public Mapping, public UseClock {
protected:
    AudioPlugin::Props& props;

    SampleStep steps[DEFAULT_MAX_STEPS];
    SampleStep* activeStep = NULL;
    SampleStep* selectedStepPtr = &steps[0];

    // uint8_t stepCounter = 31;
    uint8_t stepCounter = 0;
    bool isPlaying = false;
    uint8_t loopCounter = 0;

    AudioPlugin* targetPlugin = NULL;

    FileBrowser fileBrowser = FileBrowser("./samples");

    enum Status {
        MUTED = 0,
        ON = 1,
        NEXT = 2
    };

    void onStep() override
    {
        stepCounter++;
        uint8_t state = status.get();
        // If we reach the end of the sequence, we reset the step counter
        if (stepCounter >= DEFAULT_MAX_STEPS) {
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
            SampleStep* step = &steps[stepCounter];
            if (step->file && step->enabled && step->velocity > 0.0f) {
                setActiveStep(step);
            }
        }
    }

    void setActiveStep(SampleStep* step)
    {
        activeStep = step;
        sampleIndex = step->start;
    }

public:
    /*md **Values**: */
    /*md - `STEP_VELOCITY` set selected step velocity */
    Val& stepVelocity = val(0.0f, "STEP_VELOCITY", { "Velocity" }, [&](auto p) {
        p.val.setFloat(p.value);
        selectedStepPtr->setVelocity(p.val.pct());
    });
    /*md - `STEP_START` set selected step sample start point */
    Val& stepStart = val(0.0f, "STEP_START", { "Start", .step = 0.1f, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        selectedStepPtr->setStart(p.val.pct());
    });
    /*md - `STEP_END` set selected step sample end point */
    Val& stepEnd = val(0.0f, "STEP_END", { "End", .step = 0.1f, .unit = "%" }, [&](auto p) {
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
    Val& selectedStep = val(0.0f, "SELECTED_STEP", { "Step", .min = 1.0f, .max = DEFAULT_MAX_STEPS }, [&](auto p) {
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

    SampleSequencer(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , props(props)
    {
        initValues();

        if (config.json.contains("target")) {
            targetPlugin = &props.audioPluginHandler->getPlugin(config.json["target"].get<std::string>(), track);
        }
    }

    // FIXME
    // void onClockTick(uint64_t* clockCounter)
    // {
    //     // printf("[%d] sampleSeq onClockTick %ld\n", track, *clockCounter);
    //     clockCounterPtr = clockCounter;
    //     // Clock events are sent at a rate of 24 pulses per quarter note
    //     // (24/4 = 6)
    //     if (*clockCounter % 6 == 0) {
    //         onStep();
    //     }
    // }

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
        UseClock::sample(buf);
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

    void serializeJson(nlohmann::json& json) override
    {
        json["STATUS"] = status.get();

        nlohmann::json stepsJson;
        for (auto& step : steps) {
            stepsJson.push_back(step.serialize());
        }
        json["STEPS"] = stepsJson;
    }

    void hydrateJson(nlohmann::json& json) override
    {
        if (json.contains("STATUS")) {
            status.setFloat(json["STATUS"]);
        }
        if (json.contains("STEPS")) {
            for (size_t i = 0; i < json["STEPS"].size() && i < DEFAULT_MAX_STEPS; i++) {
                steps[i].hydrate(json["STEPS"][i], props.channels);
            }
        }
    }

    DataFn dataFunctions[4] = {
        { "GET_STEP", [this](void* userdata) {
             uint8_t* index = (uint8_t*)userdata;
             return &steps[*index >= DEFAULT_MAX_STEPS ? 0 : *index];
         } },
        { "NEXT_FILE", [this](void* userdata) {
             uint8_t* index = (uint8_t*)userdata;
             SampleStep* step = &steps[*index >= DEFAULT_MAX_STEPS ? 0 : *index];
             uint16_t pos = fileBrowser.next(step->filename);
             step->setFilename(fileBrowser.getFilePath(pos), props.channels);
             return (void*)NULL;
         } },
        { "PREVIOUS_FILE", [this](void* userdata) {
             uint8_t* index = (uint8_t*)userdata;
             SampleStep* step = &steps[*index >= DEFAULT_MAX_STEPS ? 0 : *index];
             uint16_t pos = fileBrowser.next(step->filename, -1);
             step->setFilename(fileBrowser.getFilePath(pos), props.channels);
             return (void*)NULL;
         } },
        { "PLAY_STEP", [this](void* userdata) {
             uint8_t* index = (uint8_t*)userdata;
             SampleStep* step = &steps[*index >= DEFAULT_MAX_STEPS ? 0 : *index];
             setActiveStep(step);
             return (void*)NULL;
         } },
    };
    DEFINE_GETDATAID_AND_DATA
};
