#pragma once

#include <string>
#include <vector>

#include "Tempo.h"
#include "audioPlugin.h"
#include "helpers/midiNote.h"
#include "log.h"
#include "mapping.h"
#include "stepInterface.h"

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
class Sequencer : public Mapping, public UseClock {
protected:
    AudioPlugin::Props& props;

    uint16_t stepCount = MAX_STEPS;
    std::vector<Step> steps;

    uint16_t stepCounter = 0;
    bool isPlaying = false;
    uint16_t loopCounter = 0;

    AudioPlugin* targetPlugin = NULL;

    enum Status {
        MUTED = 0,
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

    void onStep() override
    {
        stepCounter++;
        // printf("[%d] ------------------- seq stepCounter %d\n", track, stepCounter);
        uint8_t state = status.get();
        // If we reach the end of the sequence, we reset the step counter
        if (stepCounter >= stepCount) {
            stepCounter = 0;
            loopCounter++;
            props.audioPluginHandler->sendEvent(AudioEventType::SEQ_LOOP, track);
            if (state == Status::NEXT) {
                status.set(Status::ON);
            }
        }

        for (auto& step : steps) {
            if (step.counter) {
                step.counter--;
                if (step.counter == 0) {
                    props.audioPluginHandler->noteOff(getNote(step), 0, { track, targetPlugin });
                }
            }
            // here might want to check for state == Status::ON
            if (state == Status::ON && step.enabled && step.len && stepCounter == step.position && conditionMet(step) && step.velocity > 0.0f) {
                step.counter = step.len;
                props.audioPluginHandler->noteOn(getNote(step), step.velocity, { track, targetPlugin });
                // printf("should trigger note on %d track %d len %d velocity %.2f\n", step.note, track, step.len, step.velocity);
            }
        }
    }

public:
    /*md **Values**: */
    /*md - `DETUNE` detuning all playing step notes by semitones */
    Val& detune = val(0.0f, "DETUNE", { "Detune", VALUE_CENTERED, -24.0f, 24.0f });

    /*md - `STATUS` set status: off, on, next. Default: off
    Play/Stop will answer to global event. However, you may want to the sequencer to not listen to those events or to only start to play on the next sequence iteration. */
    Val& status = val(1.0f, "STATUS", { "Status", VALUE_STRING, .max = 2 }, [&](auto p) { setStatus(p.value); });

    Sequencer(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , props(props)
    {
        initValues();

        //md **Config**:
        //md - `"target": "pluginName"` the plugin to send notes to
        if (config.json.contains("target")) {
            targetPlugin = &props.audioPluginHandler->getPlugin(config.json["target"].get<std::string>(), track);
        }

        //md - `"stepCount": 32` set the number of steps
        stepCount = config.json.value("stepCount", stepCount);
    }

    void sample(float* buf) override
    {
        UseClock::sample(buf);
    }

    void noteOn(uint8_t note, float velocity, void* userdata) override
    {
        if (userdata) {
            props.audioPluginHandler->noteOn(note, velocity, { track, targetPlugin });
        }
    }

    void noteOff(uint8_t note, float velocity, void* userdata = NULL) override
    {
        if (userdata) {
            props.audioPluginHandler->noteOff(note, 0, { track, targetPlugin });
        }
    }

    void allOff()
    {
        for (auto& step : steps) {
            if (step.counter) {
                props.audioPluginHandler->noteOff(getNote(step), 0, { track, targetPlugin });
            }
            step.counter = 0;
        }
    }

    void onEvent(AudioEventType event, bool playing) override
    {
        isPlaying = playing;
        // if (event != AudioEventType::AUTOSAVE) logDebug("[%d] event %d seq is playing %d", track, event, isPlaying);
        if (event == AudioEventType::STOP) {
            logTrace("in sequencer event STOP\n");
            stepCounter = 0;
            allOff();
        } else if (event == AudioEventType::PAUSE) {
            allOff();
        }
    }

    void setStatus(float value)
    {
        status.setFloat(value);
        if (status.get() == Status::MUTED) {
            status.setString("muted");
        } else if (status.get() == Status::ON) {
            status.setString("on");
        } else {
            status.setString("next");
        }
    }

    enum DATA_ID {
        STEPS,
        STEP_COUNTER,
        IS_PLAYING,
        CLOCK_COUNTER,
        GET_STEP,
        STEP_COUNT,
        // STEP_TOGGLE,
    };

    /*md **Data ID**: */
    uint8_t getDataId(std::string name) override
    {
        /*md - `STEPS` return steps */
        if (name == "STEPS")
            return DATA_ID::STEPS;
        /*md - `STEP_COUNTER` return current played step */
        if (name == "STEP_COUNTER")
            return DATA_ID::STEP_COUNTER;
        /*md - `IS_PLAYING` return is playing */
        if (name == "IS_PLAYING")
            return DATA_ID::IS_PLAYING;
        /*md - `CLOCK_COUNTER` return clock counter */
        if (name == "CLOCK_COUNTER")
            return DATA_ID::CLOCK_COUNTER;
        /*md - `GET_STEP` get step by index */
        if (name == "GET_STEP")
            return DATA_ID::GET_STEP;
        /*md - `STEP_COUNT` return step count */
        if (name == "STEP_COUNT")
            return DATA_ID::STEP_COUNT;
        // /*md - `STEP_TOGGLE` toggle selected step */
        // if (name == "STEP_TOGGLE")
        //     return DATA_ID::STEP_TOGGLE;
        return atoi(name.c_str());
    }

    void* data(int id, void* userdata = NULL)
    {
        switch (id) {
        case DATA_ID::STEPS: {
            return &steps;
        }
        case DATA_ID::STEP_COUNTER:
            return &stepCounter;
        case DATA_ID::IS_PLAYING:
            return &isPlaying;
        case DATA_ID::CLOCK_COUNTER:
            return &clockCounter;
        case DATA_ID::STEP_COUNT:
            return &stepCount;
        case DATA_ID::GET_STEP: {
            uint16_t* position = (uint16_t*)userdata;
            for (auto& step : steps) {
                if (step.position == *position) {
                    return &step;
                }
            }
            return NULL;
        }
        }
        return NULL;
    }

    void serialize(FILE* file, std::string separator) override
    {
        for (auto& step : steps) {
            fprintf(file, "STEP %s %s", step.serialize().c_str(), separator.c_str());
        }

        fprintf(file, "STATUS %f%s", status.get(), separator.c_str());
    }

    void hydrate(std::string value) override
    {
        std::string valCopy = value;
        char* key = strtok((char*)value.c_str(), " ");
        if (strcmp(key, "STEP") == 0) {
            Step step;
            step.hydrate(strtok(NULL, ""));
            // steps[step.position] = step;
            steps.push_back(step);
            return;
        }
        Mapping::hydrate(valCopy);
    }
};
