/** Description:
This blueprint describes an audio processing component named "Mixer 2." Its primary function is to combine, or blend, the audio content from two separate input sources, similar to a crossfader used by a DJ.

**Key Functionality**
The Mixer 2 takes audio from two designated input tracks, labeled Track A and Track B. It generates a single, combined output signal by calculating a weighted average between these two sources during every moment of playback.

**User Control**
The crucial element is the "Mix" control. This setting determines the blend ratio:
*   A Mix setting of 0% allows only Track A to pass through.
*   A Mix setting of 100% allows only Track B to pass through.
*   A setting of 50% results in an equal, balanced combination of both tracks.

This Mix control is defined as a "centered value," indicating that 50% is the standard or neutral position.

**Setup and Configuration**
When the audio system initializes, the Mixer reads stored settings to identify which specific physical or virtual tracks it should listen to for its inputs (A and B). It must ensure that these dependent tracks are available before it can perform its mixing calculations successfully. If the user changes the Mix percentage, the system saves that new value for the next session.

sha: 5b661e882b10328243eed454951896c9c59d5835cb2fdbe8eb158dbbc49e0a64 
*/
#pragma once

#include "audioPlugin.h"
#include "mapping.h"

class Mixer2 : public Mapping {
public:
    Val& mix = val(50.0f, "MIX", { "Mix", .type = VALUE_CENTERED });
    uint8_t trackA = 1;
    uint8_t trackB = 2;

    Mixer2(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        trackA = config.json.value("trackA", trackA);
        trackB = config.json.value("trackB", trackB);
        mix.set(config.json.value("mix", mix.get()));
    }

    std::set<uint8_t> trackDependencies() override
    {
        return { trackA, trackB };
    }

    void sample(float* buf)
    {
        buf[track] = buf[trackA] * (1.0f - mix.pct()) + buf[trackB] * mix.pct();
    }
};
