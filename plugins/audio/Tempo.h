#ifndef _TEMPO_H_
#define _TEMPO_H_

#include "audioPlugin.h"
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
    uint32_t sampleCounter = 0;
    uint32_t sampleCountTarget = 0;

public:
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
        sampleCountTarget = (uint32_t)(((float)props.sampleRate * 60.0f / bpm.get()) / 12.0f);
    }

    void sample(float* buf)
    {
        sampleCounter++;
        if (sampleCounter >= sampleCountTarget) {
            sampleCounter = 0;
            props.audioPluginHandler->clockTick();
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
        return AudioPlugin::config(key, value);
    }

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

#endif
