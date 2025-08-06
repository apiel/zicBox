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

    std::vector<std::function<void(bool)>> eventCallbacks;

    void callEventCallbacks()
    {
        for (auto& callback : eventCallbacks) {
            callback(isPlaying);
        }
    }

    void onStep() override
    {
        stepCounter++;

        // callEventCallbacks();

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
            callEventCallbacks();
            allOff();
        } else if (event == AudioEventType::PAUSE) {
            callEventCallbacks();
            allOff();
        } else if (event == AudioEventType::TOGGLE_PLAY_PAUSE || event == AudioEventType::START) {
            callEventCallbacks();
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

    DataFn dataFunctions[7] = {
        { "STEPS", [this](void* userdata) {
             return &steps;
         } },
        { "STEP_COUNTER", [this](void* userdata) {
             return &stepCounter;
         } },
        { "IS_PLAYING", [this](void* userdata) {
             return &isPlaying;
         } },
        { "CLOCK_COUNTER", [this](void* userdata) {
             return &clockCounter;
         } },
        { "STEP_COUNT", [this](void* userdata) {
             return &stepCount;
         } },
        { "GET_STEP", [this](void* userdata) {
             uint16_t* position = (uint16_t*)userdata;
             for (auto& step : steps) {
                 if (step.position == *position) {
                     return (void*)&step;
                 }
             }
             return (void*)NULL;
         } },
        { "REGISTER_CALLBACK", [this](void* userdata) {
             // Cast userdata to the correct function pointer type
             auto callback = static_cast<std::function<void(bool)>*>(userdata);
             if (callback) {
                 eventCallbacks.push_back(*callback);
             }
             return (void*)NULL;
         } },
    };
    DEFINE_GETDATAID_AND_DATA

    void serializeJson(nlohmann::json& json) override
    {
        json["STATUS"] = status.get();

        nlohmann::json stepsJson;
        for (auto& step : steps) {
            stepsJson.push_back(step.serializeJson());
        }
        json["STEPS"] = stepsJson;
    }

    void hydrateJson(nlohmann::json& json) override
    {
        if (json.contains("STATUS")) {
            status.setFloat(json["STATUS"]);
        }
        if (json.contains("STEPS")) {
            steps.clear();
            for (nlohmann::json& stepJson : json["STEPS"]) {
                Step step;
                step.hydrateJson(stepJson);
                // Only hydrate steps that are enabled
                // else get rid of them (remove garbage)
                if (step.enabled && step.len > 0) {
                    steps.push_back(step);
                }
            }
        }
    }
};
