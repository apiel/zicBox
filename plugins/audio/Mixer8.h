#ifndef _MIXER8_H_
#define _MIXER8_H_

#include "audioPlugin.h"
#include "mapping.h"

class Mixer8 : public Mapping {
public:
    Val& mixA = val(100.0f, "MIX_A", { "Mix A" });
    Val& mixB = val(100.0f, "MIX_B", { "Mix B" });
    Val& mixC = val(100.0f, "MIX_C", { "Mix C" });
    Val& mixD = val(100.0f, "MIX_D", { "Mix D" });
    Val& mixE = val(100.0f, "MIX_E", { "Mix E" });
    Val& mixF = val(100.0f, "MIX_F", { "Mix F" });
    Val& mixG = val(100.0f, "MIX_G", { "Mix G" });
    Val& mixH = val(100.0f, "MIX_H", { "Mix H" });
    uint16_t trackA = 0;
    uint16_t trackB = 1;
    uint16_t trackC = 2;
    uint16_t trackD = 3;
    uint16_t trackE = 4;
    uint16_t trackF = 5;
    uint16_t trackG = 6;
    uint16_t trackH = 7;
    uint16_t trackTarget = 0;
    float divider = 0.125f;

    Mixer8(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
    }

    void sample(float* buf)
    {
        buf[trackTarget] = mixA.pct() * buf[trackA] * divider
            + mixB.pct() * buf[trackB] * divider
            + mixC.pct() * buf[trackC] * divider
            + mixD.pct() * buf[trackD] * divider
            + mixE.pct() * buf[trackE] * divider
            + mixF.pct() * buf[trackF] * divider
            + mixG.pct() * buf[trackG] * divider
            + mixH.pct() * buf[trackH] * divider;
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
        if (strcmp(key, "TRACK_E") == 0) {
            trackE = atoi(value);
            return true;
        }
        if (strcmp(key, "TRACK_F") == 0) {
            trackF = atoi(value);
            return true;
        }
        if (strcmp(key, "TRACK_G") == 0) {
            trackG = atoi(value);
            return true;
        }
        if (strcmp(key, "TRACK_H") == 0) {
            trackH = atoi(value);
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
        if (strcmp(key, "VALUE_E") == 0) {
            mixE.set(atof(value));
            return true;
        }
        if (strcmp(key, "VALUE_F") == 0) {
            mixF.set(atof(value));
            return true;
        }
        if (strcmp(key, "VALUE_G") == 0) {
            mixG.set(atof(value));
            return true;
        }
        if (strcmp(key, "VALUE_H") == 0) {
            mixH.set(atof(value));
            return true;
        }
        return false;
    }
};

#endif
