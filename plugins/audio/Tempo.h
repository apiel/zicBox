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
    uint16_t clockTrack = CLOCK_TRACK;

public:
    int16_t getType() override
    {
        return AudioPlugin::Type::TEMPO;
    }

    /*md **Values**: */
    /*md - `BPM` in beats per minute*/
    Val& bpm = val(120.0f, "BPM", { "Bpm", .min = 60.0f, .max = 240.0f }, [&](auto p) { setBpm(p.value); });

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

        //md - `"clockTrack": 32` set the track for clock
        clockTrack = config.json.value("clockTrack", clockTrack);
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

    // 6 clock ticks per beat
    // we want 4096 because it is multiple of 4, 8, 16, 32, 64, 128, ...
    // and we can do stuff like 4096 % 32 == 0 to know if we reached the end of a pattern.
    const int endClockCounter = 4096 * 6;
    void sample(float* buf)
    {
        if (props.audioPluginHandler->isPlaying()) {
            buf[clockTrack] = clock.getClock();
        }
    }

    void onEvent(AudioEventType event, bool playing) override
    {
        if (event == AudioEventType::STOP) {
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
public:
    uint32_t clockCounter = 0;

    void sample(float* buf)
    {
        // Converting float to uint32 is not really exact, it become more or less uint24
        // Which mean:
        // ≈ 47 hours @ 250 BPM
        // ≈ 97 hours (4 days) @ 120 BPM
        // that is ok ...
        uint32_t clockValue = (uint32_t)buf[CLOCK_TRACK];
        if (clockValue != 0) {
            clockCounter = clockValue;
            if (clockValue % 6 == 0) {
                onStep();
            }
            onClock();
        }
    }

    virtual void onClock() { }
    virtual void onStep() { }
};
