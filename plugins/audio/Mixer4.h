#ifndef _MIXER4_H_
#define _MIXER4_H_

#include "audioPlugin.h"
#include "mapping.h"

class Mixer4 : public Mapping {
public:
    Val& mixA = val(100.0f, "MIX_A", { "Mix A" });
    Val& mixB = val(100.0f, "MIX_B", { "Mix B" });
    Val& mixC = val(100.0f, "MIX_C", { "Mix C" });
    Val& mixD = val(100.0f, "MIX_D", { "Mix D" });
    uint16_t trackA = 0;
    uint16_t trackB = 1;
    uint16_t trackC = 2;
    uint16_t trackD = 3;
    uint16_t trackTarget = 0;
    float divider = 0.25f;

    Mixer4(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
    }

    void sample(float* buf)
    {
        buf[trackTarget] = mixA.pct() * buf[trackA] * divider
            + mixB.pct() * buf[trackB] * divider
            + mixC.pct() * buf[trackC] * divider
            + mixD.pct() * buf[trackD] * divider;
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
        if (strcmp(key, "TRACK_C") == 0) {
            trackC = atoi(value);
            return true;
        }
        if (strcmp(key, "TRACK_D") == 0) {
            trackD = atoi(value);
            return true;
        }
        if (strcmp(key, "TRACK_TARGET") == 0) {
            trackTarget = atoi(value);
            return true;
        }
        if (strcmp(key, "DIVIDER") == 0) {
            divider = atof(value);
            return true;
        }
        if (strcmp(key, "VALUE_A") == 0) {
            mixA.set(atof(value));
            return true;
        }
        if (strcmp(key, "VALUE_B") == 0) {
            mixB.set(atof(value));
            return true;
        }
        if (strcmp(key, "VALUE_C") == 0) {
            mixC.set(atof(value));
            return true;
        }
        if (strcmp(key, "VALUE_D") == 0) {
            mixD.set(atof(value));
            return true;
        }
        return AudioPlugin::config(key, value);
    }
};

#endif
