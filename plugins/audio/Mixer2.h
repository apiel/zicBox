#ifndef _MIXER2_H_
#define _MIXER2_H_

#include "audioPlugin.h"
#include "mapping.h"

class Mixer2 : public Mapping {
public:
    Val& mix = val(50.0f, "MIX", { "Mix", .type = VALUE_CENTERED });
    uint8_t trackA = 0;
    uint8_t trackB = 1;

    Mixer2(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
    }

    std::vector<uint8_t> trackDependencies() override
    {
        return { trackA, trackB };
    }

    void sample(float* buf)
    {
        buf[track] = buf[trackA] * (1.0f - mix.pct()) + buf[trackB] * mix.pct();
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "TRACK_A") == 0) {
            trackA = atoi(value);
            return true;
        }
        if (strcmp(key, "TRACK_B") == 0) {
            trackB = atoi(value);
            return true;
        }
        if (strcmp(key, "VALUE") == 0) {
            mix.set(atof(value));
            return true;
        }
        return AudioPlugin::config(key, value);
    }
};

#endif
