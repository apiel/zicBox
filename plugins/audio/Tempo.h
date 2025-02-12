#ifndef _TEMPO_H_
#define _TEMPO_H_

#include "audioPlugin.h"
#include "host/constants.h"
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
    uint32_t sampleCounter = 0;
    uint32_t sampleCountTarget = 0;

    uint32_t clockCounter = 0;
    uint16_t clockTrack = CLOCK_TRACK;

public:
    int16_t getType() override
    {
        return AudioPlugin::Type::TEMPO;
    }

    /*md **Values**: */
    /*md - `BPM` in beats per minute*/
    Val& bpm = val(120.0f, "BPM", { "Bpm", .min = 60.0f, .max = 240.0f }, [&](auto p) { setBpm(p.value); });

    Tempo(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    // Clock events are sent at a rate of 24 pulses per quarter note
    // The is 4 beats per quarter
    // (24/4 = 6)
    void setBpm(float _bpm)
    {
        bpm.setFloat(_bpm);
        sampleCountTarget = (uint32_t)(((float)props.sampleRate * 60.0f / bpm.get()) / 24.0f) * props.channels;
        logDebug("Tempo: %d bpm (sample rate: %d, sample count: %d)", (int)bpm.get(), props.sampleRate, sampleCountTarget);
    }

    // 6 clock ticks per beat
    // we want 4096 because it is multiple of 4, 8, 16, 32, 64, 128, ...
    // and we can do stuff like 4096 % 32 == 0 to know if we reached the end of a pattern.
    const int endClockCounter = 4096 * 6;
    void sample(float* buf)
    {
        if (props.audioPluginHandler->isPlaying()) {
            sampleCounter++;
            if (sampleCounter >= sampleCountTarget) {
                sampleCounter = 0;
                clockCounter = clockCounter + 1.0;
                // 
                if (clockCounter > endClockCounter) {
                    clockCounter = 1.0;
                }
                buf[clockTrack] = clockCounter;
            }
        }
    }

    void onEvent(AudioEventType event, bool playing) override
    {
        if (event == AudioEventType::STOP) {
            clockCounter = 0;
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value)
    {
        /*md - `BPM: 120.0` to set default beat per minute */
        if (strcmp(key, "BPM") == 0) {
            bpm.set(atof(value));
            return true;
        }

        /*md - `CLOCK_TRACK: 32` set the track for clock */
        if (strcmp(key, "CLOCK_TRACK") == 0) {
            clockTrack = atoi(value);
            return true;
        }

        return AudioPlugin::config(key, value);
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
            onClock();
            if (clockValue % 6 == 0) {
                onStep();
                if (clockValue % (getStepCount() * 6) == 0) {
                    onPatternLoop();
                }
            }
        }
    }

    virtual void onClock() { }
    virtual void onStep() { }
    virtual void onPatternLoop() { }
    virtual uint16_t getStepCount()
    {
        return MAX_STEPS;
    }
};

#endif
