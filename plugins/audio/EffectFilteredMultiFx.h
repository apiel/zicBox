/** Description:
This code defines a specialized digital audio processing unit named `EffectFilteredMultiFx`. Essentially, it acts like a sophisticated effects pedal that combines a powerful frequency filter with a flexible multi-effect unit.

The primary function of this unit is to take an incoming audio signal and apply processing in a sequential manner: first filtering, and then applying a chosen effect. Finally, it blends this newly processed sound with the original signal.

The class is built around two key internal components:
1.  **A Filter:** This component shapes the sound's frequency content, allowing you to muffle the highs or remove the lows.
2.  **A Multi-Effect Unit:** This unit contains several different standard audio effects (like chorus, distortion, or delay) that the user can select.

Users control the audio manipulation using several primary settings:
*   **CUTOFF & RESONANCE:** These controls define the filter's action, determining the specific frequency range being affected and how sharply that change occurs (the sharpness is called resonance).
*   **FX_TYPE & FX_AMOUNT:** These select which specific effect is active and how strongly that effect is applied.
*   **FEEDBACK:** This routes a portion of the processed signal back into the input, often used to create echoing or sustained sounds.
*   **MIX:** This control blends the resulting processed sound with the original, unprocessed audio signal.

This setup allows engineers and artists to not only apply an effect but also carefully sculpt the frequency characteristics of the sound *before* the effect takes place.

sha: c1b3da45c2b4a88d454f7a36d0d8140e7bcb75ff8e5a85c354139ba33d636e90 
*/
#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "audio/MultiFx.h"
#include "audio/MMfilter.h"
#include "plugins/audio/utils/valMMfilterCutoff.h"

/*md
## EffectFilteredMultiFx
*/

class EffectFilteredMultiFx : public Mapping {
protected:
    MMfilter filter;
    MultiFx multiFx;

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

    /*md - `FX_TYPE` select the effect.*/
    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MFx::FX_COUNT - 1 }, multiFx.setFxType);

    /*md - `FX_AMOUNT` set the effect amount.*/
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    /*md - `FEEDBACK` to set feedback. */
    Val& feedback = val(0.0f, "FEEDBACK", { "Feedback", .unit = "%" });

    /*md - `MIX` set the effect mix vs the original signal.*/
    Val& mix = val(50.0f, "MIX", { "Mix", .unit = "%" });

    // internal amounts
    float scaledClipping = 0.0f;

    EffectFilteredMultiFx(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , multiFx(props.sampleRate)
    {
        initValues();
    }

    // process a single-sample buffer (matching the style of EffectVolumeClipping)
    void sample(float* buf)
    {
        float input = buf[track];

        float filterOut = filter.process(buf[track]);
        float feedbackSignal = filterOut * feedback.pct();

        filterOut = multiFx.apply(filterOut, fxAmount.pct());

        float out = input * (1 - mix.pct()) + filterOut * mix.pct();
        out += feedbackSignal;

        buf[track] = out;
    }
};
