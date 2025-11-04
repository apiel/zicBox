#pragma once

#include "audioPlugin.h"
#include "host/constants.h"
#include "audio/Clock.h"
#include "mapping.h"
#include "log.h"

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
        , clock(props.sampleRate, props.channels)
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
