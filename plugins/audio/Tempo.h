/** Description:
This code defines the core timekeeping system for a digital audio environment, ensuring that all plugins and processes remain perfectly synchronized to the music’s tempo.

### 1. The Tempo Module (Master Clock)

The `Tempo` module acts as the central conductor. Its main function is to generate and manage the musical timing, measured in Beats Per Minute (BPM).

**How it works:**
The module converts the user-defined BPM (typically 60 to 240) into a continuous stream of highly precise timing pulses (or “ticks”). These ticks are standardized to ensure consistent timing across different tempos and are responsible for moving sequencers or other synchronized effects forward.

**Communication:**
Instead of sending these pulses through a separate message system, the `Tempo` module embeds the clock signal directly into a specific channel (or "track") of the processed audio data buffer. Any other synchronized plugin can then easily read this designated track to get the current timing information. The module also handles configuration, allowing the initial BPM and the specific communication track index to be set via configuration files.

### 2. The Clock Listener

The system includes a helper structure (`UseClock`) designed for any audio component that needs to follow the master tempo.

**Synchronization:**
This structure continuously monitors the designated clock track defined by the `Tempo` module. When it detects a timing pulse, it updates its own internal counter. By analyzing these pulses, it can translate the raw timing ticks into meaningful musical events, such as a “step” (like an eighth note or a sixteenth note), allowing components like sequencers to trigger events exactly on the beat.

sha: 73b3583649737198629fc4fb05a6c7a0f76765f94cc9f9f6b8586b53f437f3e1
*/
#pragma once

#include "audio/Clock.h"
#include "audioPlugin.h"
#include "host/constants.h"
#include "log.h"
#include "mapping.h"

/*md
## Tempo

Tempo audio module is responsible for clocking events. The main purpose is to send clock events to other plugins.
A good example is the sequencer.

> [!NOTE]
> - `TODO` select between internal vs midi clock
*/
class Tempo : public Mapping {
protected:
    Clock clock;

    uint32_t stepCounter = -1;

    static constexpr uint32_t STEPS_PER_BAR = 16;

public:
    // LOOP_START is expressed in bars and limited by float precision.
    // Clock counters are sent as float, which preserves only 24 bits of integer precision (~16.7M).
    // With 1 step every 6 clock pulses and 16 steps per bar, this allows ≈174,762 bars max.
    // This cap guarantees exact clock/step/bar values with no drift or skipped events.
    static constexpr float MAX_BARS = 174762.0f;

    int16_t getType() override
    {
        return AudioPlugin::Type::TEMPO;
    }

    /*md **Values**: */
    /*md - `BPM` in beats per minute*/
    Val& bpm = val(120.0f, "BPM", { "Bpm", .min = 60.0f, .max = 240.0f }, [&](auto p) { setBpm(p.value); });

    /*md - `LOOP_START` in bars (float) */
    Val& loopStart = val(0.0f, "LOOP_START", { "Loop start", .max = MAX_BARS, .unit = "bars" });

    /*md - `LOOP_LENGTH` in bars (float) */
    Val& loopLength = val(0.0f, "LOOP_LENGTH", { "Loop length", .max = MAX_BARS, .unit = "bars" });

    Tempo(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , clock(props.sampleRate)
    {
        initValues();

        //md **Config**:
        //md - `"bpm": 120.0` to set default beat per minute
        if (config.json.contains("bpm")) {
            bpm.set(config.json["bpm"].get<float>());
        }
    }

    // Clock events are sent at a rate of 24 pulses per quarter note
    // The is 4 beats per quarter
    // (24/4 = 6)
    void setBpm(float _bpm)
    {
        bpm.setFloat(_bpm);
        clock.setBpm(bpm.get());
        logDebug("Tempo: %d bpm (sample rate: %d)", (int)bpm.get(), props.sampleRate);
    }

    void sample(float* buf) override
    {
        if (!props.audioPluginHandler->isPlaying())
            return;

        uint32_t clockValue = clock.getClock();

        // buf[stepClockTrack] = clockValue;

        if (clockValue == 0) {
            buf[STEP_CLOCK_TRACK] = 0;
            return;
        }

        // Assuming there is 6 clock pulses per step, this could be configurable if we want to support different tempos.
        float stepClock = clockValue / 6.0f;
        float pulse = stepClock - (uint32_t)stepClock;
        if (pulse == 0.0f) {
            // stepCounter must be incremented independently from clock, so when we loop back to the given looping point
            // we don't need to change the clock value as well.
            //
            // Without loop, we could have simply do stepCounter = (uint32_t)stepClock;
            //
            stepCounter++; 

            // ---- LOOP LOGIC ----
            float loopLenBars = loopLength.get();
            if (loopLenBars > 0.0f) {
                uint32_t loopStartBars = (uint32_t)loopStart.get();
                uint32_t loopLengthBars = (uint32_t)loopLenBars;

                uint32_t loopStartStep = loopStartBars * STEPS_PER_BAR;
                uint32_t loopEndStep = loopStartStep + loopLengthBars * STEPS_PER_BAR;

                if (stepCounter >= loopEndStep) {
                    // logDebug("Looping from %d to %d", loopStartStep, loopEndStep);
                    stepCounter = loopStartStep;
                    props.audioPluginHandler->sendEvent(AudioEventType::TEMPO_LOOP);
                }
            }
        }
        buf[STEP_CLOCK_TRACK] = stepCounter + pulse;
    }

    void onEvent(AudioEventType event, bool playing) override
    {
        if (event == AudioEventType::STOP || (event == AudioEventType::TOGGLE_PLAY_STOP && !playing)) {
            stepCounter = 0;
            clock.reset();
        }
    }

    // TODO should this be removed?
    // Used to share current playing state of audio host
    int playingState = 0;
    void* data(int id, void* userdata = NULL)
    {
        if (id == 0) {
            playingState = 2;
            if (props.audioPluginHandler->isPlaying()) {
                playingState = 1;
            }
            if (props.audioPluginHandler->isStopped()) {
                playingState = 0;
            }
            return &playingState;
        }
        return NULL;
    }
};

class UseClock {
protected:
public:
    uint32_t stepCounter = 0;

    virtual void sample(float* buf)
    {
        // Converting float to uint32 is not really exact, it become more or less uint24
        float stepClock = buf[STEP_CLOCK_TRACK];

        if (stepClock == 0.0f)
            return;

        stepCounter = (uint32_t)stepClock;
        float pulse = stepClock - stepCounter;
        if (pulse == 0.0f)
            onStep();
        onClock();
    }

    virtual void onClock() { }
    virtual void onStep() { }
};
