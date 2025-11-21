#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <unordered_map>
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

    uint16_t stepCount = DEFAULT_MAX_STEPS;
    std::vector<Step> steps;
    std::vector<Step> stepsPreview;
    std::vector<Step>* playingSteps = &steps;

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

    int noteRepeat = -1;
    int repeatMode = 0;

    void onStep() override
    {
        stepCounter++;

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

        for (auto& step : *playingSteps) {
            if (step.counter) {
                step.counter--;
                if (step.counter == 0) {
                    props.audioPluginHandler->noteOff(getNote(step), 0, { track, targetPlugin });
                }
            }
            // here might want to check for state == Status::ON
            if (state == Status::ON && step.enabled
                && step.len && stepCounter == step.position
                && conditionMet(step) && step.velocity > 0.0f && noteRepeat == -1) {
                step.counter = step.len;
                props.audioPluginHandler->noteOn(getNote(step), step.velocity, { track, targetPlugin });
                // printf("should trigger note on %d track %d len %d velocity %.2f\n", step.note, track, step.len, step.velocity);
            }
        }
    }

    int repeatModeVal = 6;
    void onClock() override
    {
        // repeatMode: 12, 6, 3, 2
        if (noteRepeat != -1) {
            if (clockCounter % repeatMode != 0) {
                return;
            }
            props.audioPluginHandler->noteOn(noteRepeat, 1.0f, { track, targetPlugin });
        }
    }

    void copySteps(std::vector<Step>& from, std::vector<Step>& to)
    {
        to.resize(from.size());
        std::copy(from.begin(), from.end(), to.begin());
    }

public:
    /*md **Values**: */
    /*md - `DETUNE` detuning all playing step notes by semitones */
    Val& detune = val(0.0f, "DETUNE", { "Detune", VALUE_CENTERED, -24.0f, 24.0f });

    /*md - `STATUS` set status: off, on, next. Default: off
    Play/Stop will answer to global event. However, you may want to the sequencer to not listen to those events or to only start to play on the next sequence iteration. */
    Val& status = val(1.0f, "STATUS", { "Status", VALUE_STRING, .max = 2 }, [&](auto p) { setStatus(p.value); });

    // in 4/4 time signature
    // 92 tick = 1 note = 1 bar
    // 48 tick = 1 half note = 1/2 bar
    // 24 tick = 1 quarter note = 1/4 bar
    // 12 tick = Eighth Note = 1/8 bar
    // 6 tick = Sixteenth Note = 1/16 bar
    // 3 tick = Thirty-Second Note = 1/32 bar

    /*md - `NOTE_REPEAT` set note repeat mode: 1/32 bar, 1/16 bar, 1/8 bar, 1/4 bar, 1/2 bar, 1 bar */
    Val& noteRepeatVal = val(1.0f, "NOTE_REPEAT", { "Note Repeat", VALUE_STRING, .max = 5 }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() == 0.0f) {
            repeatModeVal = 3;
            p.val.setString("1/32 bar");
        } else if (p.val.get() == 1.0f) {
            repeatModeVal = 6;
            p.val.setString("1/16 bar");
        } else if (p.val.get() == 2.0f) {
            repeatModeVal = 12;
            p.val.setString("1/8 bar");
        } else if (p.val.get() == 3.0f) {
            repeatModeVal = 24;
            p.val.setString("1/4 bar");
        } else if (p.val.get() == 4.0f) {
            repeatModeVal = 48;
            p.val.setString("1/2 bar");
        } else if (p.val.get() == 5.0f) {
            repeatModeVal = 92;
            p.val.setString("1 bar");
        }
    });

    Val& playingLoops = val(0.0f, "PLAYING_LOOPS", { "Loop", VALUE_STRING, 0.0f, .max = 10.0f, .incType = INC_ONE_BY_ONE }, [&](auto p) {
        p.val.setFloat(p.value);
        allOff();
        if (p.val.get() == 0.0f) {
            p.val.setString("Current");
            playingSteps = &steps;
        } else {
            stepsPreview.clear();
            // int index = playingLoops.get() - 1; // oldest in first position
            int index = (recordedLoops.size() - 1) - (playingLoops.get() - 1); // newest in first position
            if (index < recordedLoops.size()) {
                copySteps(steps, stepsPreview);

                // Copy new recorded loop
                std::vector<RecordedNote>& loop = recordedLoops[index];
                for (auto& step : loop) {
                    stepsPreview.push_back({
                        .enabled = true,
                        .velocity = step.velocity,
                        .position = step.startStep,
                        .len = step.len,
                        .note = step.note,
                    });
                }
            }
            playingSteps = &stepsPreview;
            p.val.setString(playingSteps->size() > 0 ? "Rec " + std::to_string((int)p.val.get()) : "Empty");
        }
        p.val.props().unit = playingSteps->size() > 0 ? std::to_string((int)playingSteps->size()) + " steps" : "";
    });

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

        //md - `"recordingEnabled": true` if true, noteOn/noteOff will be recorded
        recordingEnabled = config.json.value("recordingEnabled", recordingEnabled);

        //md - `"maxRecordLoops": 10` maximum number of loops to record
        maxRecordLoops = config.json.value("maxRecordLoops", maxRecordLoops);
        playingLoops.props().max = maxRecordLoops;
    }

    void sample(float* buf) override
    {
        UseClock::sample(buf);
    }

    // -----------------------
    // RECORDING DATA STRUCTS
    // -----------------------
    struct RecordedNote {
        uint16_t loop; // loop index
        uint8_t note; // MIDI note
        uint16_t startStep; // step position within the loop [0, stepCount-1]
        uint16_t len; // length in steps (>=1)
        float velocity;
    };

    struct ActiveNote {
        uint8_t note;
        uint16_t startLoop; // absolute loopCounter when note started
        uint16_t startStep; // stepCounter when note started (0..stepCount-1)
        float velocity;
    };

    bool recordingEnabled = true; // if true, noteOn/noteOff without userdata will be recorded
    uint16_t maxRecordLoops = 10; // maximum number of loops to record (circular buffer)

    // recordedLoops[i] = notes recorded for loop index (loopCounter % maxRecordLoops == i)
    std::vector<std::vector<RecordedNote>> recordedLoops;

    // Active notes keyed by MIDI note number (only for recording path)
    std::unordered_map<uint8_t, ActiveNote> activeNotes;

    // -----------------------
    // End recording structs
    // -----------------------

    void noteOn(uint8_t note, float velocity, void* userdata) override
    {
        if (userdata != NULL) {
            props.audioPluginHandler->noteOn(note, velocity, { track, targetPlugin });

            if (!isPlaying || !recordingEnabled)
                return;

            bool record = (bool)userdata;
            if (record) {
                // Avoid duplicate active note entry (re-trigger) — if already active, we return.
                auto it = activeNotes.find(note);
                if (it != activeNotes.end()) {
                    return;
                }

                // store active note with current absolute loop and step
                ActiveNote an;
                an.note = note;
                an.startLoop = loopCounter;
                an.startStep = stepCounter;
                an.velocity = velocity;
                activeNotes[note] = an;
            }
        }
    }

    void noteOff(uint8_t note, float velocity, void* userdata = NULL) override
    {
        if (userdata != NULL) {
            props.audioPluginHandler->noteOff(note, 0, { track, targetPlugin });

            if (!isPlaying || !recordingEnabled)
                return;

            bool record = (bool)userdata;
            if (record) {
                auto it = activeNotes.find(note);
                if (it == activeNotes.end())
                    return;

                ActiveNote& an = it->second;

                // Determine duration in steps.
                uint16_t len = 1;
                if (loopCounter == an.startLoop) {
                    len = stepCounter - an.startStep;
                } else if (loopCounter > an.startLoop) {
                    len = stepCount - an.startStep;
                    len += stepCounter;
                }
                // Max length is stepCount + 1, so if a note is on more than a full loop,
                // the note will play for ever
                len = std::clamp(len, (uint16_t)1, (uint16_t)(stepCount + 1));

                int indexToPush = -1;
                for (int i = 0; i < recordedLoops.size(); i++) {
                    if (recordedLoops[i][0].loop == an.startLoop) {
                        indexToPush = i;
                        break;
                    }
                }

                if (indexToPush == -1) {
                    recordedLoops.emplace_back();
                    // if we exceed the max number of loops, remove the first one
                    if (recordedLoops.size() > maxRecordLoops) {
                        recordedLoops.erase(recordedLoops.begin());
                    }
                    indexToPush = recordedLoops.size() - 1;
                }

                recordedLoops[indexToPush].push_back({ an.startLoop, an.note, an.startStep, len, an.velocity });

                // logDebug("Record step: loop %d, note %d, startStep %d, len %d", an.startLoop, an.note, an.startStep, len);

                activeNotes.erase(it);
            }
        }
    }

    void noteRepeatOn(uint8_t note, uint8_t mode) override
    {
        repeatMode = mode == 0 ? repeatModeVal : mode;
        noteRepeat = note;
    }

    void noteRepeatOff(uint8_t note) override
    {
        props.audioPluginHandler->noteOff(noteRepeat, 0, { track, targetPlugin });
        noteRepeat = -1;
    }

    void allOff()
    {
        for (auto& step : *playingSteps) {
            if (step.counter) {
                props.audioPluginHandler->noteOff(getNote(step), 0, { track, targetPlugin });
            }
            step.counter = 0;
        }
        // Also all off for recording active notes
        for (auto& kv : activeNotes) {
            props.audioPluginHandler->noteOff(kv.first, 0, { track, targetPlugin });
        }
        activeNotes.clear();
    }

    void onEvent(AudioEventType event, bool playing) override
    {
        isPlaying = playing;
        // if (event != AudioEventType::AUTOSAVE) logDebug("[%d] event %d seq is playing %d", track, event, isPlaying);
        if (event == AudioEventType::STOP) {
            logTrace("in sequencer event STOP");
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
        { "STEP_COUNT", [this](void* userdata) {
             return &stepCount;
         } },
        { "REGISTER_CALLBACK", [this](void* userdata) {
             // Cast userdata to the correct function pointer type
             auto callback = static_cast<std::function<void(bool)>*>(userdata);
             if (callback) {
                 eventCallbacks.push_back(*callback);
             }
             return (void*)NULL;
         } },
        { "SAVE_RECORD", [this](void* userdata) {
             if (playingLoops.get() > 0) {
                 copySteps(stepsPreview, steps);
                 playingLoops.set(0);
             }
             return (void*)NULL;
         } },
        { "CLEAR_STEPS", [this](void* userdata) {
             steps.clear();
             logDebug("Cleared steps");
             return (void*)NULL;
         } }
    };
    DEFINE_GETDATAID_AND_DATA

    void serializeJson(nlohmann::json& json) override
    {
        json["STATUS"] = status.get();

        nlohmann::json stepsJson;
        for (auto& step : *playingSteps) {
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
