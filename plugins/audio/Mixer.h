/** Description:
This code defines a digital **Mixer**—a fundamental component in audio processing—designed to combine multiple separate sound sources (or tracks) into a single output signal. It functions as an audio plugin that can be customized to handle various numbers of input tracks, such as 4, 8, or 12.

### How the Mixer Works

The mixer's primary role is to manage and blend these input tracks:

1.  **Inputs and Controls:** For every track it handles, the mixer provides dedicated user controls:
    *   **Volume:** A setting (often a percentage) that determines how loud that specific track will be in the final mix.
    *   **Mute:** An on/off switch that allows the user to silence a track instantly without losing its volume setting.

2.  **The Blending Process:** When audio data is processed, the mixer cycles through all its input tracks. If a track is not muted, the mixer reads its current audio level, applies the user-set volume control, and then adds that modified sound into a running total.

3.  **Volume Safeguard:** To prevent the combined sound from becoming too loud or distorted (a phenomenon called "clipping"), the mixer uses a built-in safety value called a "divider." This divider helps scale down the summed signal to a safe level before it is outputted.

4.  **Configuration:** The mixer is highly configurable. Users can define exactly which physical track numbers in the system should be used as inputs (e.g., starting the mix with track 1 or track 7), and they can also override the automatic volume divider if needed.

The final result is a single, balanced output signal containing the weighted sum of all the active input tracks.

sha: 502ad9db2a3f2743f5e07ca30feaa662c04943e7573251b01a7846607e47568b 
*/
#pragma once

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

    Mixer(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        uint16_t trackStart = 1;

        //md **Config**:
        auto& json = config.json;
        //md - `"trackStart": 7` to set track 7 as first track, then 8, 9, ...
        trackStart = json.value("trackStart", trackStart);

        //md  - `"divider": 0.5` to set a custom divider. By default, divider equals 1 divided by the number of tracks.
        divider = json.value("divider", divider);

        /*md And input tracks start at 1, then 2, 3, ....*/
        for (uint8_t i = 0; i < TRACK_COUNT; i++) {
            // Start tracks at 1 and leave 0 for master track (trackTarget)
            tracks[i] = i + trackStart;
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
};
