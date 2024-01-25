#ifndef _MIXER12_H_
#define _MIXER12_H_

#include "audioPlugin.h"
#include "mapping.h"

class Mixer12 : public Mapping {
protected:
    static const uint16_t trackCount = 12;

public:
    Val* mix[trackCount] = {
        &val(100.0f, "TRACK_1", { "Track 1" }),
        &val(100.0f, "TRACK_2", { "Track 2" }),
        &val(100.0f, "TRACK_3", { "Track 3" }),
        &val(100.0f, "TRACK_4", { "Track 4" }),
        &val(100.0f, "TRACK_5", { "Track 5" }),
        &val(100.0f, "TRACK_6", { "Track 6" }),
        &val(100.0f, "TRACK_7", { "Track 7" }),
        &val(100.0f, "TRACK_8", { "Track 8" }),
        &val(100.0f, "TRACK_9", { "Track 9" }),
        &val(100.0f, "TRACK_10", { "Track 10" }),
        &val(100.0f, "TRACK_11", { "Track 11" }),
        &val(100.0f, "TRACK_12", { "Track 12" })
    };

    uint16_t tracks[trackCount] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    uint16_t trackTarget = 0;
    float divider = 1.0f / (float)trackCount;

    Mixer12(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
    }

    void sample(float* buf)
    {
        float out = 0;
        for (uint16_t i = 0; i < trackCount; i++) {
            out += mix[i]->pct() * buf[tracks[i]] * divider;
        }
        buf[trackTarget] = out;
    }

    bool config(char* key, char* value)
    {
        if (strncmp(key, "TRACK_", strlen("TRACK_")) == 0) {
            uint16_t i = atoi(key + strlen("TRACK_")) - 1;
            // By default, trackTarget is track 1 unless trackTarget is specified
            if (i == 0 && tracks[i] == trackTarget) {
                trackTarget = atoi(value);
            }

            if (i < trackCount) {
                tracks[i] = atoi(value);
            }
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

        if (strncmp(key, "VALUE_", strlen("VALUE_")) == 0) {
            uint16_t i = atoi(key + strlen("VALUE_")) - 1;
            if (i < trackCount) {
                mix[i]->set(atof(value));
            }
            return true;
        }

        return AudioPlugin::config(key, value);
    }
};

#endif
