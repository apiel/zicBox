#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "plugins/audio/utils/MultiFx.h"

/*md
## EffectBandIsolatorFx
*/

class EffectBandIsolatorFx : public Mapping {
protected:
    MultiFx multiFx;

    struct BQ {
        float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
        float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
        float process(float x)
        {
            float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            x2 = x1;
            x1 = x;
            y2 = y1;
            y1 = y;
            return y;
        }
    } lowpass, highpass;

    void updateCoeffs()
    {
        // Compute min/max freq from centerFreq and rangeHz
        float minFreq = std::max(20.0f, centerFreq.get() - rangeHz.get() * 0.5f);
        float maxFreq = std::min(props.sampleRate * 0.5f, centerFreq.get() + rangeHz.get() * 0.5f);

        setHighpass(highpass, minFreq, props.sampleRate);
        setLowpass(lowpass, maxFreq, props.sampleRate);
    }

    void setLowpass(BQ& bq, float freq, float fs)
    {
        float w0 = 2.0f * M_PI * freq / fs;
        float cosw0 = cosf(w0);
        float sinw0 = sinf(w0);
        float alpha = sinw0 / (2.0f * 0.707f); // Q=0.707

        float b0n = (1 - cosw0) / 2;
        float b1n = 1 - cosw0;
        float b2n = (1 - cosw0) / 2;
        float a0n = 1 + alpha;
        float a1n = -2 * cosw0;
        float a2n = 1 - alpha;

        bq.b0 = b0n / a0n;
        bq.b1 = b1n / a0n;
        bq.b2 = b2n / a0n;
        bq.a1 = a1n / a0n;
        bq.a2 = a2n / a0n;
    }

    void setHighpass(BQ& bq, float freq, float fs)
    {
        float w0 = 2.0f * M_PI * freq / fs;
        float cosw0 = cosf(w0);
        float sinw0 = sinf(w0);
        float alpha = sinw0 / (2.0f * 0.707f);

        float b0n = (1 + cosw0) / 2;
        float b1n = -(1 + cosw0);
        float b2n = (1 + cosw0) / 2;
        float a0n = 1 + alpha;
        float a1n = -2 * cosw0;
        float a2n = 1 - alpha;

        bq.b0 = b0n / a0n;
        bq.b1 = b1n / a0n;
        bq.b2 = b2n / a0n;
        bq.a1 = a1n / a0n;
        bq.a2 = a2n / a0n;
    }

public:
    /*md - `FREQ` set the center frequency of the effect.*/
    Val& centerFreq = val(1000.0f, "FREQ", { "Center Frequency", .min = 20.0f, .max = 20000.0f, .step = 10.0f, .unit = "Hz" }, [&](auto p) {
        p.val.setFloat(p.value);
        updateCoeffs();
    });

    /*md - `RANGE` set the range of the effect.*/
    Val& rangeHz = val(2000.0f, "RANGE", { "Range", .min = 50.0f, .max = 10000.0f, .step = 50.0f, .unit = "Hz" }, [&](auto p) {
        p.val.setFloat(p.value);
        updateCoeffs();
    });

    /*md - `FX_TYPE` select the effect.*/
    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) {
        multiFx.setFxType(p);
    });

    /*md - `FX_AMOUNT` set the effect amount.*/
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    /*md - `MIX` set the effect mix vs the original signal.*/
    Val& mix = val(50.0f, "MIX", { "Mix", .unit = "%" });

    // /*md - `ENV_SENS` set the envelope sensitivity.*/
    // Val& envSensitivity = val(0.0f, "ENV_SENS", { "Env. Sensitivity", .min = 0.0f, .max = 100.0f, .unit = "%" });

    EffectBandIsolatorFx(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , multiFx(props.sampleRate, props.lookupTable)
    {
        initValues();
        updateCoeffs();
    }

    float envelope = 0.0f;

    void sample(float* buf)
    {
        float input = buf[track];

        float band = highpass.process(input);
        band = lowpass.process(band);

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
