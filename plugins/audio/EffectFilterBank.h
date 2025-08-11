#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "plugins/audio/utils/effects/applyClipping.h"
#include "plugins/audio/utils/effects/applyDrive.h"
#include "plugins/audio/utils/effects/applyWaveshape.h"
#include "plugins/audio/utils/effects/applySoftClipping.h"
#include "plugins/audio/utils/utils.h"

/*md
## EffectFilterBank

A dual-filter "Sherman-like" filterbank: two multimode biquad filters, drives/clipping
stages, feedback and routing (serial/parallel).
*/

class EffectFilterBank : public Mapping {
public:
    // Filter modes
    enum FilterMode { MODE_LP = 0,
        MODE_BP,
        MODE_HP,
        MODE_COUNT };

    // Filter 1
    Val& cutoff1 = val(1000.0f, "CUTOFF1", { "Filter1.|Cutoff", .min = 20.0f, .max = 20000.0f, .step = 20.0f, .unit = "Hz" }, [&](auto p) {
        p.val.setFloat(p.value);
        needUpdateCoeffs1 = true;
    });

    Val& res1 = val(0.5f, "RES1", { "Filter1.|Resonance", .min = 0.01f, .max = 1.5f, .step = 0.01f, .floatingPoint = 2, .unit = "Q" }, [&](auto p) {
        p.val.setFloat(p.value);
        needUpdateCoeffs1 = true;
    });

    Val& mode1 = val((float)MODE_LP, "MODE1", { "Filter1.|Mode", .max = MODE_COUNT - 1, .skipJumpIncrements = true }, [&](auto p) {
        p.val.setFloat(p.value);
        needUpdateCoeffs1 = true;
    });

    Val& drive = val(0.0f, "DRIVE", { "Drive", .unit = "%" });
    Val& clipAmount = val(0.0f, "CLIP", { "Clipping", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        // scaled like original plugin
        scaledClipping = p.val.pct() * p.val.pct() * 20.0f;
    });

    Val& feedback = val(0.0f, "FEEDBACK", { "Routing.|Feedback", .unit = "%" });
    Val& mix = val(50.0f, "MIX", { "Mix", .unit = "%" });

    // internal amounts
    float scaledClipping = 0.0f;

    // sample rate
    float sr = 48000.0f;

    // Biquad implementation (per-filter)
    struct BQ {
        // coefficients
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f; // a0 normalized to 1
        // state
        float x1 = 0.0f, x2 = 0.0f;
        float y1 = 0.0f, y2 = 0.0f;

        inline float process(float x)
        {
            float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            x2 = x1;
            x1 = x;
            y2 = y1;
            y1 = y;
            return y;
        }

        void reset() { x1 = x2 = y1 = y2 = 0.0f; }

        void setLP(float fc, float Q, float fs)
        {
            const float w0 = 2.0f * M_PI * fc / fs;
            const float cosw0 = cosf(w0);
            const float sinw0 = sinf(w0);
            const float alpha = sinw0 / (2.0f * Q);

            float b0n = (1.0f - cosw0) / 2.0f;
            float b1n = 1.0f - cosw0;
            float b2n = (1.0f - cosw0) / 2.0f;
            float a0n = 1.0f + alpha;
            float a1n = -2.0f * cosw0;
            float a2n = 1.0f - alpha;

            b0 = b0n / a0n;
            b1 = b1n / a0n;
            b2 = b2n / a0n;
            a1 = a1n / a0n;
            a2 = a2n / a0n;
        }

        void setHP(float fc, float Q, float fs)
        {
            const float w0 = 2.0f * M_PI * fc / fs;
            const float cosw0 = cosf(w0);
            const float sinw0 = sinf(w0);
            const float alpha = sinw0 / (2.0f * Q);

            float b0n = (1.0f + cosw0) / 2.0f;
            float b1n = -(1.0f + cosw0);
            float b2n = (1.0f + cosw0) / 2.0f;
            float a0n = 1.0f + alpha;
            float a1n = -2.0f * cosw0;
            float a2n = 1.0f - alpha;

            b0 = b0n / a0n;
            b1 = b1n / a0n;
            b2 = b2n / a0n;
            a1 = a1n / a0n;
            a2 = a2n / a0n;
        }

        void setBP(float fc, float Q, float fs)
        {
            const float w0 = 2.0f * M_PI * fc / fs;
            const float cosw0 = cosf(w0);
            const float sinw0 = sinf(w0);
            const float alpha = sinw0 / (2.0f * Q);

            float b0n = alpha;
            float b1n = 0.0f;
            float b2n = -alpha;
            float a0n = 1.0f + alpha;
            float a1n = -2.0f * cosw0;
            float a2n = 1.0f - alpha;

            b0 = b0n / a0n;
            b1 = b1n / a0n;
            b2 = b2n / a0n;
            a1 = a1n / a0n;
            a2 = a2n / a0n;
        }
    };

    // two filters
    BQ filter1;

    bool needUpdateCoeffs1 = true;

    EffectFilterBank(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        // get sample rate if available
        if (props.sampleRate > 0.0f)
            sr = props.sampleRate;

        initValues();
        // initial coefficient update
        needUpdateCoeffs1 = true;
    }

    void updateCoeffsIfNeeded()
    {
        if (needUpdateCoeffs1) {
            float fc = std::clamp(cutoff1.get(), 20.0f, sr * 0.45f);
            float Q = std::clamp(res1.get(), 0.01f, 10.0f);
            int mode = (int)mode1.get();
            if (mode == MODE_LP)
                filter1.setLP(fc, Q, sr);
            else if (mode == MODE_BP)
                filter1.setBP(fc, Q, sr);
            else
                filter1.setHP(fc, Q, sr);
            needUpdateCoeffs1 = false;
        }
    }

    // process a single-sample buffer (matching the style of EffectVolumeClipping)
    void sample(float* buf)
    {
        updateCoeffsIfNeeded();

        float input = buf[track];

        float filterOut = filter1.process(input);
        float feedbackSignal = filterOut * feedback.pct();

        if (drive.pct() > 0.0f) filterOut = applyDrive(filterOut, drive.pct(), props.lookupTable);
        filterOut = applyClipping(filterOut, scaledClipping);

        float out = input * (1 - mix.pct()) + filterOut * mix.pct();
        out += feedbackSignal;

        buf[track] = out;
    }

    // void sample(float* buf)
    // {
    //     updateCoeffsIfNeeded();

    //     float input = buf[track];

    //     float cleanFiltered = filter1.process(input);
    //     float feedbackSignal = cleanFiltered * feedback.pct();

    //     float driven = cleanFiltered;
    //     if (drive.pct() > 0.0f) {
    //         float preGain = 1.0f + drive.pct() * 1.5f; // gentle gain staging
    //         driven = applyDrive(driven * preGain, drive.pct(), props.lookupTable);
    //     }
    //     driven = applySoftClipping(driven, props.lookupTable);
    //     float driveLevel = 1.0f / (1.0f + drive.pct() * 0.4f);
    //     driven *= driveLevel;

    //     float out = input * (1.0f - mix.pct()) + driven * mix.pct();

    //     out += feedbackSignal;

    //     buf[track] = out;
    // }
};
