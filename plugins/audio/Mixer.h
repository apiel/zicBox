#ifndef _MIXER_H_
#define _MIXER_H_

#include "audioPlugin.h"
#include "mapping.h"

/*md
## Mixer

Mixer audio plugin is used to mix tracks together.

- `Mixer4` is a 4-track mixer.
- `Mixer8` is a 8-track mixer.
- `Mixer12` is a 12-track mixer.

*/
template <uint8_t TRACK_COUNT>
class Mixer : public Mapping {
public:
    Val* mix[TRACK_COUNT];
    Val* mutes[TRACK_COUNT];

    uint8_t tracks[TRACK_COUNT];
    float divider = 1.0f / (float)TRACK_COUNT;

    Mixer(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        /*md And input tracks start at 1, then 2, 3, ....*/
        for (uint8_t i = 0; i < TRACK_COUNT; i++) {
            // Start tracks at 1 and leave 0 for master track (trackTarget)
            tracks[i] = i + 1;
            /*md **Value**: */
            /*md - `TRACK_1` to set volume on track 1, min = 0.0, max = 100.*/
            /*md - `TRACK_2` to set volume on track 2.*/
            /*md - ...*/
            mix[i] = &val(100.0f, "TRACK_" + std::to_string(i + 1), { "Track " + std::to_string(i + 1), .unit = "%" });
            // mix[i] = &val(100.0f, "TRACK_" + std::to_string(i + 1), { "Track " + std::to_string(i + 1) }, [&, i](auto p) { setTrack(p.value, i); });
            /*md - `MUTE_1` to mute track 1, `0.0` to unmute, `1.0` to mute.*/
            /*md - `MUTE_2` to mute track 2.*/
            /*md - ...*/
            mutes[i] = &val(0.0f, "MUTE_" + std::to_string(i + 1), { "Mute " + std::to_string(i + 1), .max = 1.0f });
        }
    }

    std::set<uint8_t> trackDependencies() override
    {
        std::set<uint8_t> dependencies = {};
        for (uint16_t i = 0; i < TRACK_COUNT; i++) {
            dependencies.insert(tracks[i]);
        }
        return dependencies;
    }

    void sample(float* buf) override
    {
        float out = 0;
        for (uint16_t i = 0; i < TRACK_COUNT; i++) {
            if (mutes[i]->get()) {
                continue;
            }
            out += mix[i]->pct() * buf[tracks[i]] * divider;
        }
        buf[track] = out;
    }

    /*md **Config**: */
    bool config(char* key, char* value) override
    {
        /*md - `TRACK_START: 7` to set track 7 as first track, then 8, 9, ...*/
        if (strcmp(key, "TRACK_START") == 0) {
            uint16_t trackStart = atoi(value);
            // printf("------------------------------------ trackStart: %d\n", trackStart);
            for (uint16_t i = 0; i < TRACK_COUNT; i++) {
                tracks[i] = trackStart + i;
                // printf("- tracks[%d]: %d\n", i, tracks[i]);
            }
            return true;
        }

        /*md - `DIVIDER: 0.5` to set a custom divider. By default, divider equals 1 divided by the number of tracks.*/
        if (strcmp(key, "DIVIDER") == 0) {
            divider = atof(value);
            return true;
        }

        /*md - `VALUE_1: 0.5` to set track 1 volume to 50%.*/
        if (strncmp(key, "VALUE_", strlen("VALUE_")) == 0) {
            uint16_t i = atoi(key + strlen("VALUE_")) - 1;
            if (i < TRACK_COUNT) {
                mix[i]->set(atof(value));
            }
            return true;
        }

        /*md - `TRACK_1: 1` to set track input 1 on track buffer 1.*/
        /*md - `TRACK_2: 2` to set track input 2 on track buffer 2.*/
        /*md - ...*/
        if (strncmp(key, "TRACK_", strlen("TRACK_")) == 0) {
            uint16_t i = atoi(key + strlen("TRACK_")) - 1;
            if (i < TRACK_COUNT) {
                tracks[i] = atoi(value);
            }
            return true;
        }

        return AudioPlugin::config(key, value);
    }
};

#endif
