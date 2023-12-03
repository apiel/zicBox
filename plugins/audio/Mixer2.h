#ifndef _MIXER2_H_
#define _MIXER2_H_

#include "audioPlugin.h"
#include "mapping.h"

class Mixer2 : public Mapping {
public:
    Val& mix = val(50.0f, "MIX", { "Mix", .type = VALUE_CENTERED });
    uint16_t trackA = 0;
    uint16_t trackB = 1;
    uint16_t trackTarget = 0;

    Mixer2(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
    }

    void sample(float* buf)
    {
        buf[trackTarget] = buf[trackA] * (1.0f - mix.pct()) + buf[trackB] * mix.pct();
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "TRACK_A") == 0) {
            // By default, trackTarget is trackA unless trackTarget is specified
            if (trackA == trackTarget) {
                trackTarget = atoi(value);
            }
            trackA = atoi(value);
            return true;
        }
        if (strcmp(key, "TRACK_B") == 0) {
            trackB = atoi(value);
            return true;
        }
        if (strcmp(key, "TRACK_TARGET") == 0) {
            trackTarget = atoi(value);
            return true;
        }
        if (strcmp(key, "VALUE") == 0) {
            mix.set(atof(value));
            return true;
        }
        return false;
    }
};

#endif
