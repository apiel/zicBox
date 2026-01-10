/** Description:
This C++ header defines the **Sequencer** module, a powerful tool used in audio production to create rhythmic patterns and trigger musical events in other sound generators (plugins).

**Core Functionality:**
The Sequencer operates like a highly customizable, tempo-synced drum machine. It uses the global music timing (clock) to advance through a series of steps, checking at each step whether a specific musical event should be played.

**Timing and Conditions:**
The sequence can run up to 32 steps (or more, depending on configuration). For each step, the user can define a note, its intensity (velocity), and how long it lasts. Crucially, each step also has a **trigger condition** that determines if it actually plays. These conditions go beyond simple playback, including:
1.  **Rhythmic Patterns:** Playing only on odd steps, every fourth step, or every eighth step.
2.  **Probability:** Introducing controlled randomness, such as a 5%, 50%, or 99% chance of the note being triggered.

**Key Controls and Features:**
*   **Detune:** A control to globally shift the pitch of all playing notes.
*   **Status:** Defines the sequencer’s state (Muted, Actively On, or waiting to start on the Next loop iteration).
*   **Note Repeat:** A dedicated function for rapidly re-triggering a note at precise, tempo-synced musical subdivisions (like 1/16th or 1/32nd notes).
*   **Recording and Looping:** The module can actively capture incoming notes played by the user and convert them into new sequences. This allows the user to record multiple alternative loops, which can be instantly swapped in for playback, alongside the manually programmed steps.

In essence, this module manages complex rhythmic scheduling, ensuring that musical data is sent to a target plugin accurately, allowing for both precise patterns and controlled randomized grooves.

sha: acf6be1fba5ded7c3bf3eaa94d22d0ad0d13635ac50eb2d80ee0ca74452157c1
*/
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
#include "plugins/audio/utils/densifySteps.h"
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
    std::vector<Step> stepsBackup;
    bool stepsOverridden = false;

    bool isPlaying = false;
    uint16_t loopCounter = 0;

    bool timelineMode = false;

    AudioPlugin* targetPlugin = NULL;

    enum Status {
        MUTED = 0,
        ON = 1,
        NEXT = 2, // Play on the next loop iteration
        ONCE = 3, // Play only once
        COUNT,
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
    uint16_t seqPos = 0;

    void onStep() override
    {
        uint8_t state = status.get();

        seqPos = stepCounter % stepCount;
        // If we reach the end of the sequence, we reset the step counter
        if (seqPos == 0) {
            loopCounter++;
            props.audioPluginHandler->sendEvent(AudioEventType::SEQ_LOOP, track);
            if (state == Status::ONCE) {
                status.set(Status::MUTED);
                logDebug("status set to muted\n");
            } else if (state == Status::NEXT) {
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
            if ((state == Status::ON || state == Status::ONCE) && step.enabled
                && step.len && seqPos == step.position
                && conditionMet(step) && step.velocity > 0.0f && noteRepeat == -1) {
                step.counter = step.len;
                props.audioPluginHandler->noteOn(getNote(step), step.velocity, { track, targetPlugin });
                // logDebug("should trigger note on %d track %d len %d velocity %.2f\n", step.note, track, step.len, step.velocity);
            }
        }
    }

    int repeatModeVal = 6;
    void onClock() override
    {
        // repeatMode: 12, 6, 3, 2
        if (noteRepeat != -1) {
            // FIXME
            // if (clockCounter % repeatMode != 0) {
            //     return;
            // }
            props.audioPluginHandler->noteOn(noteRepeat, 1.0f, { track, targetPlugin });
        }
    }

    void restoreSteps()
    {
        if (stepsOverridden) {
            steps = stepsBackup;
            stepsBackup.clear();
            stepsOverridden = false;
        }
    }

public:
    /*md **Values**: */
    /*md - `DETUNE` detuning all playing step notes by semitones */
    Val& detune = val(0.0f, "DETUNE", { "Detune", VALUE_CENTERED, -24.0f, 24.0f });

    /*md - `STATUS` set status: off, on, next. Default: off
    Play/Stop will answer to global event. However, you may want to the sequencer to not listen to those events or to only start to play on the next sequence iteration. */
    Val& status = val(1.0f, "STATUS", { "Status", VALUE_STRING, .max = Status::COUNT - 1 }, [&](auto p) { setStatus(p.value); });

    Val& stepCountVal = val(DEFAULT_MAX_STEPS, "STEP_COUNT", { "Step Count", VALUE_BASIC, .min = 1, .max = 2048 }, [&](auto p) {
        p.val.setFloat(p.value);
        stepCount = p.val.get();
    });

    /*md - `DENSITY` modifies step pattern dynamically, reversible */
    Val& variation = val(0.0f, "DENSITY", { "Density", VALUE_CENTERED, -100.0f, 100.0f, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        float v = p.val.pct() * 2.0f - 1.0f;

        // ---- restore original ----
        if (std::abs(v) < 0.001f) {
            restoreSteps();
            return;
        }

        // ---- first activation ----
        if (!stepsOverridden) {
            stepsBackup = steps;
            stepsOverridden = true;
        }

        // ---- apply density ----
        steps = densifySteps(stepsBackup, stepCount, v);
    });

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

    Val& playingLoops = val(0.0f, "PLAYING_LOOPS", { "Records", VALUE_STRING, 0.0f, .max = 10.0f, .incType = INC_ONE_BY_ONE }, [&](auto p) {
        p.val.setFloat(p.value);
        allOff();

        if (p.val.get() == 0.0f) {
            p.val.setString("Saved");
            restoreSteps();
        } else {
            if (!stepsOverridden) {
                stepsBackup = steps;
                stepsOverridden = true;
            }
            steps.clear();

            int index = (recordedLoops.size() - 1) - ((int)p.val.get() - 1); // newest in first position
            if (index < recordedLoops.size()) {
                std::vector<RecordedNote>& loop = recordedLoops[index];
                for (auto& step : loop) {
                    steps.push_back({
                        .enabled = true,
                        .velocity = step.velocity,
                        .position = step.startStep,
                        .len = step.len,
                        .note = step.note,
                    });
                }
                p.val.setString("Rec " + std::to_string((int)p.val.get()));
            } else {
                p.val.setString("Empty");
                restoreSteps();
            }
        }

        p.val.props().unit = steps.size() > 0 ? std::to_string((int)steps.size()) + " steps" : "";
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

        //md - `"defaultStepCount": 32` set the number of steps
        stepCountVal.setFloat(config.json.value("defaultStepCount", DEFAULT_MAX_STEPS));

        //md - `"recordingEnabled": true` if true, noteOn/noteOff will be recorded
        recordingEnabled = config.json.value("recordingEnabled", recordingEnabled);
        recording = recordingEnabled; // if recording is disabled, recording must be activated manually

        //md - `"maxRecordLoops": 10` maximum number of loops to record
        maxRecordLoops = config.json.value("maxRecordLoops", maxRecordLoops);
        playingLoops.props().max = maxRecordLoops;

        //md - `"timelineMode": true` if true, timeline will be used
        timelineMode = config.json.value("timelineMode", timelineMode);
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

    bool recordingEnabled = true;
    bool recording = true;
    uint16_t maxRecordLoops = 10;

    std::vector<std::vector<RecordedNote>> recordedLoops;
    std::unordered_map<uint8_t, ActiveNote> activeNotes;

    // -----------------------
    // End recording structs
    // -----------------------

    void noteOn(uint8_t note, float velocity, void* userdata) override
    {
        if (!isPlaying || !recording)
            return;

        auto it = activeNotes.find(note);
        if (it != activeNotes.end()) return;

        ActiveNote an;
        an.note = note;
        an.startLoop = loopCounter;
        an.startStep = stepCounter;
        an.velocity = velocity;
        activeNotes[note] = an;
    }

    void noteOff(uint8_t note, float velocity, void* userdata = NULL) override
    {
        if (!isPlaying || !recording)
            return;

        auto it = activeNotes.find(note);
        if (it == activeNotes.end())
            return;

        ActiveNote& an = it->second;
        uint16_t len = CLAMP(stepCounter - an.startStep, (uint16_t)1, (uint16_t)(stepCount + 1));

        int indexToPush = -1;
        for (int i = 0; i < recordedLoops.size(); i++) {
            if (recordedLoops[i][0].loop == an.startLoop) {
                indexToPush = i;
                break;
            }
        }

        if (indexToPush == -1) {
            recordedLoops.emplace_back();
            if (recordedLoops.size() > maxRecordLoops) {
                recordedLoops.erase(recordedLoops.begin());
            }
            indexToPush = recordedLoops.size() - 1;
        }

        uint16_t pos = an.startStep % stepCount;
        recordedLoops[indexToPush].push_back({ an.startLoop, an.note, pos, len, an.velocity });

        activeNotes.erase(it);
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
        for (auto& step : steps) {
            if (step.counter) {
                props.audioPluginHandler->noteOff(getNote(step), 0, { track, targetPlugin });
            }
            step.counter = 0;
        }

        for (auto& kv : activeNotes) {
            props.audioPluginHandler->noteOff(kv.first, 0, { track, targetPlugin });
        }
        activeNotes.clear();
    }

    void onEvent(AudioEventType event, bool playing) override
    {
        isPlaying = playing;

        if (event == AudioEventType::STOP || event == AudioEventType::PAUSE) {
            if (!recordingEnabled) {
                recording = false;
            }
            callEventCallbacks();
            allOff();
        } else if (event == AudioEventType::START) {
            callEventCallbacks();
        } else if (event == AudioEventType::RECORD) {
            recording = true;
            callEventCallbacks();
        }
        // No need to handle TOGGLE_PLAY_STOP or TOGGLE_PLAY_PAUSE, it is already handled by host/AudioPluginHandler.h
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

    DataFn dataFunctions[9] = {
        { "STEPS", [this](void* userdata) { return &steps; } },
        { "STEP_COUNTER", [this](void* userdata) { return &stepCounter; } },
        { "IS_PLAYING", [this](void* userdata) { return &isPlaying; } },
        { "STEP_COUNT", [this](void* userdata) { return &stepCount; } },
        { "SEQ_POSITION", [this](void* userdata) { return &seqPos; } },
        { "REGISTER_CALLBACK", [this](void* userdata) {
             auto callback = static_cast<std::function<void(bool)>*>(userdata);
             if (callback) eventCallbacks.push_back(*callback);
             return (void*)NULL;
         } },
        { "SAVE_RECORD", [this](void* userdata) {
             if (playingLoops.get() > 0 && stepsOverridden) {
                 stepsBackup = steps;
                 playingLoops.set(0);
                 //  stepsOverridden = false;
             }
             return (void*)NULL;
         } },
        { "CLEAR_STEPS", [this](void* userdata) {
             steps.clear();
             logDebug("Cleared steps");
             return (void*)NULL;
         } },
        { "RESTORE_STEPS", [this](void* userdata) {
             restoreSteps();
             return (void*)NULL;
         } }
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
        json["STEP_COUNT"] = stepCountVal.get();

        // Do not save density!
    }

    void hydrateJson(nlohmann::json& json) override
    {
        if (timelineMode) {
            status.set(Status::ONCE);
        } else if (json.contains("STATUS")) {
            status.setFloat(json["STATUS"]);
        }

        logDebug("hydrating sequencer timeline mode %d status %d", timelineMode, status.get());

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

        if (json.contains("STEP_COUNT")) {
            stepCountVal.set(json["STEP_COUNT"]);
        }
    }
};
