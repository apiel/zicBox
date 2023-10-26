#ifndef _MIXER4_H_
#define _MIXER4_H_

#include "audioPlugin.h"
#include "mapping.h"

class Mixer4 : public Mapping<Mixer4> {
public:
    Val<Mixer4>& mixA = val(this, 100.0f, "MIX_A", &Mixer4::setMixA, { "Mix A" });
    Val<Mixer4>& mixB = val(this, 100.0f, "MIX_B", &Mixer4::setMixB, { "Mix B" });
    Val<Mixer4>& mixC = val(this, 100.0f, "MIX_C", &Mixer4::setMixC, { "Mix C" });
    Val<Mixer4>& mixD = val(this, 100.0f, "MIX_D", &Mixer4::setMixD, { "Mix D" });
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

    Mixer4& setMixA(float value)
    {
        mixA.setFloat(value);
        return *this;
    }

    Mixer4& setMixB(float value)
    {
        mixB.setFloat(value);
        return *this;
    }

    Mixer4& setMixC(float value)
    {
        mixC.setFloat(value);
        return *this;
    }

    Mixer4& setMixD(float value)
    {
        mixD.setFloat(value);
        return *this;
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
        return false;
    }
};

#endif
