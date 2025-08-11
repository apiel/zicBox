#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "plugins/audio/utils/effects/applyClipping.h"
#include "plugins/audio/utils/effects/applyDrive.h"
#include "plugins/audio/utils/effects/applySoftClipping.h"
#include "plugins/audio/utils/effects/applyWaveshape.h"
#include "plugins/audio/utils/utils.h"

/*md
## EffectFilterBank

A dual-filter "Sherman-like" filterbank: two multimode biquad filters, drives/clipping
stages, feedback and routing (serial/parallel).
*/

class EffectFilterBank : public Mapping {
public:
    Val& minFreq = val(200.0f, "MIN_FREQ", { "Min Freq", .min = 20.0f, .max = 20000.0f, .step = 20.0f, .unit = "Hz" }, [&](auto p) {
        p.val.setFloat(p.value);
        updateCoeffs();
    });
    Val& maxFreq = val(2000.0f, "MAX_FREQ", { "Max Freq", .min = 20.0f, .max = 20000.0f, .step = 20.0f, .unit = "Hz" }, [&](auto p) {
        p.val.setFloat(p.value);
        updateCoeffs();
    });

    Val& drive = val(0.0f, "DRIVE", { "Drive", .unit = "%" });
    Val& clipAmount = val(0.0f, "CLIP", { "Clipping", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        scaledClipping = p.val.pct() * p.val.pct() * 20.0f;
    });
    Val& mix = val(50.0f, "MIX", { "Mix", .unit = "%" });

    struct BQ {
        float b0=1, b1=0, b2=0, a1=0, a2=0;
        float x1=0, x2=0, y1=0, y2=0;
        float process(float x) {
            float y = b0*x + b1*x1 + b2*x2 - a1*y1 - a2*y2;
            x2 = x1; x1 = x;
            y2 = y1; y1 = y;
            return y;
        }
    } lowpass, highpass;

    float sr = 48000.0f;
    float scaledClipping = 0.0f;

    EffectFilterBank(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        if (props.sampleRate > 0.0f) sr = props.sampleRate;
        initValues();
        updateCoeffs();
    }

    void updateCoeffs() {
        setHighpass(highpass, minFreq.get(), sr);
        setLowpass(lowpass, maxFreq.get(), sr);
    }

    void setLowpass(BQ& bq, float freq, float fs) {
        float w0 = 2.0f * M_PI * freq / fs;
        float cosw0 = cosf(w0);
        float sinw0 = sinf(w0);
        float alpha = sinw0 / (2.0f * 0.707f); // Q=0.707

        float b0n = (1 - cosw0) / 2;
        float b1n = 1 - cosw0;
        float b2n = (1 - cosw0) / 2;
        float a0n = 1 + alpha;
        float a1n = -2*cosw0;
        float a2n = 1 - alpha;

        bq.b0 = b0n / a0n; bq.b1 = b1n / a0n; bq.b2 = b2n / a0n;
        bq.a1 = a1n / a0n; bq.a2 = a2n / a0n;
    }

    void setHighpass(BQ& bq, float freq, float fs) {
        float w0 = 2.0f * M_PI * freq / fs;
        float cosw0 = cosf(w0);
        float sinw0 = sinf(w0);
        float alpha = sinw0 / (2.0f * 0.707f);

        float b0n = (1 + cosw0) / 2;
        float b1n = -(1 + cosw0);
        float b2n = (1 + cosw0) / 2;
        float a0n = 1 + alpha;
        float a1n = -2*cosw0;
        float a2n = 1 - alpha;

        bq.b0 = b0n / a0n; bq.b1 = b1n / a0n; bq.b2 = b2n / a0n;
        bq.a1 = a1n / a0n; bq.a2 = a2n / a0n;
    }

    void sample(float* buf) {
        float input = buf[track];

        // Extract band
        float band = highpass.process(input);
        band = lowpass.process(band);

        // Process the band
        if (drive.pct() > 0.0f)
            band = applyDrive(band, drive.pct(), props.lookupTable);

        band = applyClipping(band, scaledClipping);

        // Recombine
        float out = input * (1.0f - mix.pct()) + band * mix.pct();

        buf[track] = out;
    }
};
