#ifndef _MIXER8_H_
#define _MIXER8_H_

#include "audioPlugin.h"
#include "mapping.h"

class Mixer8 : public Mapping<Mixer8> {
public:
    Val<Mixer8>& mixA = val(this, 100.0f, "MIX_A", &Mixer8::setMixA, { "Mix A" });
    Val<Mixer8>& mixB = val(this, 100.0f, "MIX_B", &Mixer8::setMixB, { "Mix B" });
    Val<Mixer8>& mixC = val(this, 100.0f, "MIX_C", &Mixer8::setMixC, { "Mix C" });
    Val<Mixer8>& mixD = val(this, 100.0f, "MIX_D", &Mixer8::setMixD, { "Mix D" });
    Val<Mixer8>& mixE = val(this, 100.0f, "MIX_E", &Mixer8::setMixE, { "Mix E" });
    Val<Mixer8>& mixF = val(this, 100.0f, "MIX_F", &Mixer8::setMixF, { "Mix F" });
    Val<Mixer8>& mixG = val(this, 100.0f, "MIX_G", &Mixer8::setMixG, { "Mix G" });
    Val<Mixer8>& mixH = val(this, 100.0f, "MIX_H", &Mixer8::setMixH, { "Mix H" });
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

    Mixer8& setMixA(float value)
    {
        mixA.setFloat(value);
        return *this;
    }

    Mixer8& setMixB(float value)
    {
        mixB.setFloat(value);
        return *this;
    }

    Mixer8& setMixC(float value)
    {
        mixC.setFloat(value);
        return *this;
    }

    Mixer8& setMixD(float value)
    {
        mixD.setFloat(value);
        return *this;
    }

    Mixer8& setMixE(float value)
    {
        mixE.setFloat(value);
        return *this;
    }

    Mixer8& setMixF(float value)
    {
        mixF.setFloat(value);
        return *this;
    }

    Mixer8& setMixG(float value)
    {
        mixG.setFloat(value);
        return *this;
    }

    Mixer8& setMixH(float value)
    {
        mixH.setFloat(value);
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
