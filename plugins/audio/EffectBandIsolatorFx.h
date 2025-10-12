#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/BandEq.h"

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
        , multiFx(props.sampleRate, props.lookupTable)
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
