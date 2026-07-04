/** Description:
This technical blueprint defines an audio processing component called `EffectFilterMultiMode`. Its primary purpose is to apply dynamic filter effects to an incoming sound signal, often used to shape the tone or texture of audio.

**Core Functionality:**
The component utilizes an internal Multi-Mode filter engine to process sound samples. It takes audio data and modifies it according to user-defined settings, track by track.

**Key Feature (Multi-Mode Cutoff):**
What makes this filter unique is how it handles the frequency control. The filter seamlessly combines two major types of filtering:
1.  **Low Pass Filter (LPF):** Allows low frequencies to pass while blocking high ones.
2.  **High Pass Filter (HPF):** Allows high frequencies to pass while blocking low ones.

This switching is managed automatically by a single control knob. When the main control is set in the lower half of its range (0% up to the midpoint), the system acts as an LPF. Once the control crosses the midpoint (50%), it automatically transitions into an HPF.

**User Controls:**
1.  **CUTOFF:** This is the central control that determines the operating frequency and switches the filter mode (LPF to HPF).
2.  **RESONANCE:** This controls the intensity or sharpness of the filter effect at the defined cutoff frequency, often creating a pronounced peak in the sound.

In summary, this component provides a configurable, morphing filter effect vital for sound design and audio manipulation.

sha: d65c7a4a9e03900a68172e9b5907590208f62a97cc095b28bfc521fadb2d2735 
*/
#pragma once

#include "helpers/clamp.h"
#include "mapping.h"
#include "audio/MMfilter.h"
#include "plugins/audio/utils/valMMfilterCutoff.h"

/*md
## EffectFilterMultiMode

EffectFilterMultiMode plugin is used to apply filter effect on audio buffer.
Cutoff frequency will switch from low pass filter to high pass filter when reaching 50%.
*/
class EffectFilterMultiMode : public Mapping {
protected:
    MMfilter filter;

public:
    /*md **Values**: */
    /*md - `CUTOFF` to set cutoff frequency and switch between low and high pass filter. */
    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", .type = VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, valMMfilterCutoff(filter));

    /*md - `RESONANCE` to set resonance. */
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        // printf("setResonance %f >> %f\n", value, resonance.pct());
        filter.setResonance(p.val.pct());
    });

    EffectFilterMultiMode(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    };

    void sample(float* buf)
    {
        buf[track] = filter.process(buf[track]);
    }
};
