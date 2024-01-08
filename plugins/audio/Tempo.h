#ifndef _TEMPO_H_
#define _TEMPO_H_

#include "audioPlugin.h"
#include "mapping.h"

class Tempo : public Mapping {
protected:
    uint32_t sampleCounter = 0;
    uint32_t sampleCountTarget = 0;

public:
    Val& bpm = val(120.0f, "bpm", { "Bpm", .min = 60.0f, .max = 240.0f }, [&](auto p) { setBpm(p.value); });

    Tempo(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    // Clock events are sent at a rate of 24 pulses per quarter note
    // (24/4 = 6)
    void setBpm(float _bpm)
    {
        bpm.setFloat(_bpm);
        sampleCountTarget = (uint32_t)((float)props.sampleRate * 60.0f / bpm.get() / 4.0f);
    }

    void sample(float* buf)
    {
        sampleCounter++;
        if (sampleCounter >= sampleCountTarget) {
            sampleCounter = 0;
            props.audioPluginHandler->clockTick();
        }
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "BPM") == 0) {
            bpm.set(atof(value));
            return true;
        }
        return AudioPlugin::config(key, value);
    }
};

#endif
