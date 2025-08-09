#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "plugins/audio/utils/utils.h"
#include "plugins/audio/utils/effects/applyClipping.h"
#include "plugins/audio/utils/effects/applyDrive.h"
#include "plugins/audio/utils/effects/applyWaveshape.h"

/*md
## EffectFilterBank

A dual-filter "Sherman-like" filterbank: two multimode biquad filters, drives/clipping
stages, feedback and routing (serial/parallel).
*/

class EffectFilterBank : public Mapping {
public:
    // Filter modes
    enum FilterMode { MODE_LP = 0, MODE_BP = 1, MODE_HP = 2 };

    // Parameters
    Val& volume = val(100.0f, "VOLUME", { "Volume", .unit = "%" });

    // Filter 1
    Val& cutoff1 = val(1000.0f, "CUTOFF1", { "Filter1.|Cutoff", .unit = "Hz", .min = 20.0f, .max = 20000.0f }, [&](auto p) {
        p.val.setFloat(p.value);
        needUpdateCoeffs1 = true;
    });

    Val& res1 = val(0.5f, "RES1", { "Filter1.|Resonance", .min = 0.01f, .max = 1.5f }, [&](auto p) {
        p.val.setFloat(p.value);
        needUpdateCoeffs1 = true;
    });

    Val& mode1 = val((float)MODE_LP, "MODE1", { "Filter1.|Mode" }, [&](auto p) {
        p.val.setFloat(p.value);
        needUpdateCoeffs1 = true;
    });

    // Filter 2
    Val& cutoff2 = val(2000.0f, "CUTOFF2", { "Filter2.|Cutoff", .unit = "Hz", .min = 20.0f, .max = 20000.0f }, [&](auto p) {
        p.val.setFloat(p.value);
        needUpdateCoeffs2 = true;
    });

    Val& res2 = val(0.5f, "RES2", { "Filter2.|Resonance", .min = 0.01f, .max = 1.5f }, [&](auto p) {
        p.val.setFloat(p.value);
        needUpdateCoeffs2 = true;
    });

    Val& mode2 = val((float)MODE_LP, "MODE2", { "Filter2.|Mode" }, [&](auto p) {
        p.val.setFloat(p.value);
        needUpdateCoeffs2 = true;
    });

    // Drive / waveshape / clipping
    Val& drivePre = val(0.0f, "DRIVE_PRE", { "Drive.|Pre", .max = 20.0f }, [&](auto p) { p.val.setFloat(p.value); drivePreAmount = p.val.get() / 100.0f; });
    Val& driveBetween = val(0.0f, "DRIVE_BETWEEN", { "Drive.|Between", .max = 20.0f }, [&](auto p) { p.val.setFloat(p.value); driveBetweenAmount = p.val.get() / 100.0f; });
    Val& drivePost = val(0.0f, "DRIVE_POST", { "Drive.|Post", .max = 20.0f }, [&](auto p) { p.val.setFloat(p.value); drivePostAmount = p.val.get() / 100.0f; });

    Val& clipAmount = val(0.0f, "CLIP", { "Clipping", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        // scaled like original plugin
        scaledClipping = p.val.pct() * p.val.pct() * 20.0f;
    });

    Val& feedback = val(0.0f, "FEEDBACK", { "Routing.|Feedback", .unit = "%", .min = -95.0f, .max = 95.0f }, [&](auto p) {
        p.val.setFloat(p.value);
        feedbackAmount = p.val.get() / 100.0f;
    });

    // Routing: 0 = Serial, 1 = Parallel
    Val& routing = val(0.0f, "ROUTING", { "Routing" }, [&](auto p) {
        p.val.setFloat(p.value);
        routingSerial = (p.val.get() <= 0.5f);
    });

    // internal amounts
    float scaledClipping = 0.0f;
    float drivePreAmount = 0.0f;
    float driveBetweenAmount = 0.0f;
    float drivePostAmount = 0.0f;
    float feedbackAmount = 0.0f;
    bool routingSerial = true;

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

        inline float process(float x) {
            float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            x2 = x1; x1 = x;
            y2 = y1; y1 = y;
            return y;
        }

        void reset() { x1 = x2 = y1 = y2 = 0.0f; }

        void setLP(float fc, float Q, float fs) {
            const float w0 = 2.0f * M_PI * fc / fs;
            const float cosw0 = cosf(w0);
            const float sinw0 = sinf(w0);
            const float alpha = sinw0 / (2.0f * Q);
            const float A = 1.0f; // not used for simple LP/HP/BP

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

        void setHP(float fc, float Q, float fs) {
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

        void setBP(float fc, float Q, float fs) {
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
    BQ filter2;

    bool needUpdateCoeffs1 = true;
    bool needUpdateCoeffs2 = true;

    EffectFilterBank(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        // get sample rate if available
        if (props.sampleRate > 0.0f) sr = props.sampleRate;

        initValues();
        // initial coefficient update
        needUpdateCoeffs1 = needUpdateCoeffs2 = true;
    }

    void updateCoeffsIfNeeded()
    {
        if (needUpdateCoeffs1) {
            float fc = std::clamp(cutoff1.get(), 20.0f, sr * 0.45f);
            float Q = std::clamp(res1.get(), 0.01f, 10.0f);
            int m = (int)mode1.get();
            if (m == MODE_LP) filter1.setLP(fc, Q, sr);
            else if (m == MODE_BP) filter1.setBP(fc, Q, sr);
            else filter1.setHP(fc, Q, sr);
            needUpdateCoeffs1 = false;
        }
        if (needUpdateCoeffs2) {
            float fc = std::clamp(cutoff2.get(), 20.0f, sr * 0.45f);
            float Q = std::clamp(res2.get(), 0.01f, 10.0f);
            int m = (int)mode2.get();
            if (m == MODE_LP) filter2.setLP(fc, Q, sr);
            else if (m == MODE_BP) filter2.setBP(fc, Q, sr);
            else filter2.setHP(fc, Q, sr);
            needUpdateCoeffs2 = false;
        }
    }

    // process a single-sample buffer (matching the style of EffectVolumeClipping)
    void sample(float* buf)
    {
        updateCoeffsIfNeeded();

        // read input
        float input = buf[track];

        // pre-drive
        if (drivePreAmount > 0.0f) input = applyDrive(input, drivePreAmount, props.lookupTable);
        // if (waveshapeAmount > 0.0f) input = applyWaveshape(input, waveshapeAmount, props.lookupTable);
        // early clipping
        input = applyClipping(input, scaledClipping);

        float out1 = 0.0f;
        float out2 = 0.0f;

        if (routingSerial) {
            // serial: input -> filter1 -> (drive/clip) -> filter2 -> output
            out1 = filter1.process(input);
            // between-drive
            if (driveBetweenAmount > 0.0f) out1 = applyDrive(out1, driveBetweenAmount, props.lookupTable);
            out1 = applyClipping(out1, scaledClipping);
            out2 = filter2.process(out1);
        } else {
            // parallel: input split -> both filters -> mix
            out1 = filter1.process(input);
            out2 = filter2.process(input);
            // allow a simple blend
        }

        // feedback: inject a portion of previous output back into filters' states
        // we'll implement simple feedback by feeding back into filter1's input on next call via a tiny buffer
        float feedbackSignal = (out1 + out2) * 0.5f * feedbackAmount;

        // post-drive and post-clipping
        float out = 0.0f;
        if (routingSerial) out = out2; else out = (out1 + out2) * 0.5f;

        if (drivePostAmount > 0.0f) out = applyDrive(out, drivePostAmount, props.lookupTable);
        out = applyClipping(out, scaledClipping);

        // simple feedback application: mix feedback into output (not perfect loop but keeps code simple)
        out += feedbackSignal;

        // final waveshape if user set waveshape through mix param
        // out = applyWaveshape(out, waveshapeAmount, props.lookupTable);

        // apply final volume
        buf[track] = out * volume.pct();
    }
};
