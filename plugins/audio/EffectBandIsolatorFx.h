/** Description:
This C++ header file defines a specialized audio processing unit called `EffectBandIsolatorFx`. Its primary purpose is to apply a customizable audio effect (like distortion, chorus, or delay) exclusively to a specific range of frequencies within a sound signal, rather than affecting the entire signal uniformly.

**How It Works:**

The system operates in three main stages using internal components:
1.  **Isolation:** A frequency filter component isolates a specific "band" of sound defined by the user (e.g., only the high-mid frequencies).
2.  **Application:** This isolated band is then passed to a separate, customizable multi-effect unit. This unit applies the chosen effect type only to the frequencies currently in the band.
3.  **Blending:** The newly processed frequency band is carefully mixed back with the original, untouched input signal, resulting in a sound where the effect is precisely targeted.

**Key User Controls:**

*   **Center Frequency (`FREQ`):** Sets the central point (in Hertz) where the filter is focused, determining which part of the sound spectrum will be affected (e.g., focusing on bass or treble).
*   **Range (`RANGE`):** Defines the width of the frequency band being isolated. A wider range includes more frequencies in the effect.
*   **FX Type (`FX_TYPE`):** Allows selection of the specific audio effect to be applied to the isolated band (e.g., a time-based effect or a modulation effect).
*   **FX Amount (`FX_AMOUNT`):** Controls the intensity or depth of the chosen effect.
*   **Mix (`MIX`):** Determines the final balance between the original audio signal and the processed, effect-laden signal.

This setup gives precise control, allowing users to apply strong effects to one frequency area (like adding heavy distortion only to the bass) without muddying or compromising the rest of the audio.

sha: 998e7d5b1317197de6bc64aedb81e341fcb34f4fc5fe83219bc86ee365795ed2 
*/
#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "audio/MultiFx.h"
#include "audio/BandEq.h"

/*md
## EffectBandIsolatorFx
*/

class EffectBandIsolatorFx : public Mapping {
protected:
    MultiFx multiFx;
    BandEq bandEq;

public:
    /*md - `FREQ` set the center frequency of the effect.*/
    Val& centerFreq = val(1000.0f, "FREQ", { "Center Frequency", .min = 20.0f, .max = 20000.0f, .step = 10.0f, .unit = "Hz" }, [&](auto p) {
        p.val.setFloat(p.value);
        bandEq.setCenterFreq(p.value);
    });

    /*md - `RANGE` set the range of the effect.*/
    Val& rangeHz = val(2000.0f, "RANGE", { "Range", .min = 50.0f, .max = 10000.0f, .step = 50.0f, .unit = "Hz" }, [&](auto p) {
        p.val.setFloat(p.value);
        bandEq.setRangeHz(p.value);
    });

    /*md - `FX_TYPE` select the effect.*/
    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);

    /*md - `FX_AMOUNT` set the effect amount.*/
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    /*md - `MIX` set the effect mix vs the original signal.*/
    Val& mix = val(50.0f, "MIX", { "Mix", .unit = "%" });

    // /*md - `ENV_SENS` set the envelope sensitivity.*/
    // Val& envSensitivity = val(0.0f, "ENV_SENS", { "Env. Sensitivity", .min = 0.0f, .max = 100.0f, .unit = "%" });

    EffectBandIsolatorFx(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , multiFx(props.sampleRate)
        , bandEq(props.sampleRate)
    {
        initValues();
    }

    float envelope = 0.0f;

    void sample(float* buf)
    {
        float input = buf[track];

        float band = bandEq.process(input);

        float modulatedFxAmount = fxAmount.pct();
        // if (envSensitivity.pct() > 0.0f) {
        //     float envIn = fabsf(band);
        //     float depth = envSensitivity.pct(); // modulation depth
        //     float speed = 0.002f + 0.098f * depth; // faster at high sensitivity
        //     float coeff = expf(-speed * props.sampleRate);

        //     if (envIn > envelope)
        //         envelope += (envIn - envelope) * (1.0f - coeff);
        //     else
        //         envelope += (envIn - envelope) * (1.0f - coeff * 4.0f); // slower release

        //     modulatedFxAmount = (1.0f - envelope * depth) * fxAmount.pct() + (envelope * depth) * 1.0f;
        //     modulatedFxAmount = std::clamp(modulatedFxAmount, 0.0f, 1.0f);
        // }
        band = multiFx.apply(band, modulatedFxAmount);

        buf[track] = input * (1.0f - mix.pct()) + band * mix.pct();
    }
};
