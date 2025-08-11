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
    // EQ controls
    Val& lowGain = val(0.0f, "LOW_GAIN", { "EQ.|Low Gain", .min = -12.0f, .max = 12.0f, .step = 0.5f, .unit = "dB" }, [&](auto p) {
        p.val.setFloat(p.value);
        updateCoeffs();
    });
    Val& midGain = val(0.0f, "MID_GAIN", { "EQ.|Mid Gain", .min = -12.0f, .max = 12.0f, .step = 0.5f, .unit = "dB" }, [&](auto p) {
        p.val.setFloat(p.value);
        updateCoeffs();
    });
    Val& highGain = val(0.0f, "HIGH_GAIN", { "EQ.|High Gain", .min = -12.0f, .max = 12.0f, .step = 0.5f, .unit = "dB" }, [&](auto p) {
        p.val.setFloat(p.value);
        updateCoeffs();
    });

    // Drive / clipping / mix
    Val& drive = val(0.0f, "DRIVE", { "Drive", .unit = "%" });
    Val& clipAmount = val(0.0f, "CLIP", { "Clipping", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        scaledClipping = p.val.pct() * p.val.pct() * 20.0f;
    });
    Val& mix = val(50.0f, "MIX", { "Mix", .unit = "%" });

    // Internal EQ filter structure
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
    } lowShelf, midPeak, highShelf;

    float sr = 48000.0f;
    float scaledClipping = 0.0f;

    EffectFilterBank(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        if (props.sampleRate > 0.0f)
            sr = props.sampleRate;
        initValues();
        updateCoeffs();
    }

    void updateCoeffs()
    {
        setLowShelf(lowShelf, 120.0f, lowGain.get(), sr);
        setPeakingEQ(midPeak, 1000.0f, midGain.get(), 1.0f, sr);
        setHighShelf(highShelf, 6000.0f, highGain.get(), sr);
    }

    // --- Simple EQ coefficient calculators ---
    void setLowShelf(BQ& bq, float freq, float gainDB, float fs)
    {
        float A = powf(10.0f, gainDB / 40.0f);
        float w0 = 2.0f * M_PI * freq / fs;
        float alpha = sinf(w0) / 2.0f * sqrtf((A + 1 / A) * (1 / 0.707f - 1) + 2);
        float cosw0 = cosf(w0);

        float b0n = A * ((A + 1) - (A - 1) * cosw0 + 2 * sqrtf(A) * alpha);
        float b1n = 2 * A * ((A - 1) - (A + 1) * cosw0);
        float b2n = A * ((A + 1) - (A - 1) * cosw0 - 2 * sqrtf(A) * alpha);
        float a0n = (A + 1) + (A - 1) * cosw0 + 2 * sqrtf(A) * alpha;
        float a1n = -2 * ((A - 1) + (A + 1) * cosw0);
        float a2n = (A + 1) + (A - 1) * cosw0 - 2 * sqrtf(A) * alpha;

        bq.b0 = b0n / a0n;
        bq.b1 = b1n / a0n;
        bq.b2 = b2n / a0n;
        bq.a1 = a1n / a0n;
        bq.a2 = a2n / a0n;
    }

    void setHighShelf(BQ& bq, float freq, float gainDB, float fs)
    {
        float A = powf(10.0f, gainDB / 40.0f);
        float w0 = 2.0f * M_PI * freq / fs;
        float alpha = sinf(w0) / 2.0f * sqrtf((A + 1 / A) * (1 / 0.707f - 1) + 2);
        float cosw0 = cosf(w0);

        float b0n = A * ((A + 1) + (A - 1) * cosw0 + 2 * sqrtf(A) * alpha);
        float b1n = -2 * A * ((A - 1) + (A + 1) * cosw0);
        float b2n = A * ((A + 1) + (A - 1) * cosw0 - 2 * sqrtf(A) * alpha);
        float a0n = (A + 1) - (A - 1) * cosw0 + 2 * sqrtf(A) * alpha;
        float a1n = 2 * ((A - 1) - (A + 1) * cosw0);
        float a2n = (A + 1) - (A - 1) * cosw0 - 2 * sqrtf(A) * alpha;

        bq.b0 = b0n / a0n;
        bq.b1 = b1n / a0n;
        bq.b2 = b2n / a0n;
        bq.a1 = a1n / a0n;
        bq.a2 = a2n / a0n;
    }

    void setPeakingEQ(BQ& bq, float freq, float gainDB, float Q, float fs)
    {
        float A = powf(10.0f, gainDB / 40.0f);
        float w0 = 2.0f * M_PI * freq / fs;
        float alpha = sinf(w0) / (2.0f * Q);
        float cosw0 = cosf(w0);

        float b0n = 1 + alpha * A;
        float b1n = -2 * cosw0;
        float b2n = 1 - alpha * A;
        float a0n = 1 + alpha / A;
        float a1n = -2 * cosw0;
        float a2n = 1 - alpha / A;

        bq.b0 = b0n / a0n;
        bq.b1 = b1n / a0n;
        bq.b2 = b2n / a0n;
        bq.a1 = a1n / a0n;
        bq.a2 = a2n / a0n;
    }

    void sample(float* buf)
    {
        float input = buf[track];

        // EQ pass
        float eqd = lowShelf.process(input);
        eqd = midPeak.process(eqd);
        eqd = highShelf.process(eqd);

        // Drive/clipping
        if (drive.pct() > 0.0f)
            eqd = applyDrive(eqd, drive.pct(), props.lookupTable);

        eqd = applyClipping(eqd, scaledClipping);

        // Mix
        buf[track] = input * (1.0f - mix.pct()) + eqd * mix.pct();
    }
};
