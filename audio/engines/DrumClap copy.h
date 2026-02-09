#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/effects/applyBoost.h"
#include "audio/effects/applyCompression.h"
#include "audio/engines/EngineBase.h"
#include "audio/effects/applySmallReverb.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"
#include "helpers/clamp.h"
#include <array>
#include <cmath>

class DrumClap : public EngineBase<DrumClap> {
public:
    EnvelopDrumAmp envelopAmp;

protected:
    const float sampleRate;
    float velocity = 1.0f;

    // Internal "Fast RAM" buffers for Reverb (approx 15KB total)
    static constexpr int R1_SIZE = 1113; // ~25ms
    static constexpr int R2_SIZE = 1357; // ~30ms
    static constexpr int R3_SIZE = 1611; // ~36ms
    float b1[R1_SIZE], b2[R2_SIZE], b3[R3_SIZE];
    SmallReverb d1, d2, d3;

    float burstTimer = 0.f;
    int burstIndex = 0;
    float env = 0.f;
    float pink = 0.f;
    float phase = 0.f;
    bool active = false;

    float time = 0.f;
    float timeRatio = 0.f;

    // Filter coefficients
    float b0_a0, b1_a0, b2_a0, a1_a0, a2_a0;
    float gainComp = 1.0f;

    void updateFilter() {
        float f0 = 1000.f + pct(filterFreq) * 3000.f;
        float Q = 1.0f + pct(filterReso) * 3.0f;
        float omega = 2.f * 3.14159265f * f0 / sampleRate;
        float s = Math::sin(omega);
        float c = Math::cos(omega);
        float alpha = s / (2.f * Q);
        float a0 = 1.f + alpha;
        float invA0 = 1.0f / a0;
        b0_a0 = alpha * invA0;
        b1_a0 = 0.f;
        b2_a0 = -alpha * invA0;
        a1_a0 = (-2.f * c) * invA0;
        a2_a0 = (1.f - alpha) * invA0;
        gainComp = 1.f + Q;
    }

public:
    Param params[12] = {
        { .label = "Duration", .unit = "ms", .value = 500.0f, .min = 50.0f, .max = 3000.0f, .step = 10.0f },
        { .label = "Amp. Env.", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float val) { static_cast<DrumClap*>(ctx)->envelopAmp.morph(val * 0.01f); } },
        { .label = "Burst Decay", .unit = "%", .value = 25.0f },
        { .label = "Bursts", .value = 5.0f, .min = 1.f, .max = 10.f },
        { .label = "Spacing", .unit = "%", .value = 30.0f },
        { .label = "Noise Color", .unit = "%", .value = 70.0f },
        { .label = "Cutoff", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float val) { static_cast<DrumClap*>(ctx)->updateFilter(); } },
        { .label = "Resonance", .unit = "%", .value = 30.0f, .onUpdate = [](void* ctx, float val) { static_cast<DrumClap*>(ctx)->updateFilter(); } },
        { .label = "Punch", .unit = "%", .value = 100.0f, .min = -100.f, .max = 100.f, .type = VALUE_CENTERED },
        { .label = "Transient", .unit = "%", .value = 0.0f },
        { .label = "Boost", .unit = "%", .value = 0.0f, .min = -100.f, .max = 100.f, .type = VALUE_CENTERED },
        { .label = "Reverb", .unit = "%", .value = 20.0f }
    };

    Param& duration = params[0];
    Param& ampEnv = params[1];
    Param& burstDecay = params[2];
    Param& burstCount = params[3];
    Param& burstSpacing = params[4];
    Param& noiseColor = params[5];
    Param& filterFreq = params[6];
    Param& filterReso = params[7];
    Param& punch = params[8];
    Param& transient = params[9];
    Param& boost = params[10];
    Param& reverb = params[11];

    DrumClap(const float sampleRate)
        : EngineBase(Drum, "Clap", params)
        , sampleRate(sampleRate)
    {
        d1.init(b1, R1_SIZE);
        d2.init(b2, R2_SIZE);
        d3.init(b3, R3_SIZE);
        updateFilter();
    }

    void noteOnImpl(uint8_t note, float _velocity) {
        velocity = _velocity;
        burstTimer = 0.f;
        burstIndex = 0;
        env = 1.f;
        active = true;
        time = 0.f;
        timeRatio = 1.f / sampleRate;
        envelopAmp.reset(static_cast<int>(sampleRate * (duration.value * 0.001f)));
    }

    float sampleImpl() {
        float envAmp = envelopAmp.next();
        time += timeRatio;

        float output = 0.0f;
        float spacing = pct(burstSpacing) * 0.03f + 0.01f;
        float decayTime = pct(burstDecay) * 0.3f + 0.02f;

        // 1. Burst Generation
        if (burstIndex < int(burstCount.value)) {
            burstTimer += timeRatio;
            if (burstTimer >= spacing) {
                burstTimer -= spacing;
                burstIndex++;
                env = 1.f;
            }
        }

        if (env > 0.f) {
            float white = Noise::sample();
            pink = 0.98f * pink + 0.02f * white;
            float noise = pink * (1.f - pct(noiseColor)) + white * pct(noiseColor);
            output += noise * env;
            env *= Math::exp(-1.f / (sampleRate * decayTime));
        }

        // 2. Processing chain
        output = applyBandpass(output);
        output = applyPunch(output);
        output = applyBoostOrCompression(output);
        output = applySmallReverb(output, pct(reverb), d1, d2, d3);

        return output * envAmp * velocity;
    }

private:
    float lpState = 0.f;
    float applyPunch(float in) {
        float out = in;
        if (punch.value < 0) {
            float amt = (0.5f - pct(punch)) * 2.f;
            out = CLAMP(out + out * amt * 8, -1.0f, 1.0f);
        } else if (time < 0.02f) {
            float amt = (pct(punch) - 0.5f) * 2.f;
            out *= 1.f + amt * 2.f;
        }
        if (time < 0.01f) {
            float highpassed = out - lpState;
            lpState += 0.01f * (out - lpState);
            out += highpassed * pct(transient) * 2.0f;
        }
        return out;
    }

    float prevInput = 0.f, prevOutput = 0.f;
    float applyBoostOrCompression(float input) {
        if (boost.value == 0.0f) return input;
        if (boost.value > 0.0f) {
            float amt = (pct(boost) - 0.5f) * 2.f;
            return applyBoost(input, amt, prevInput, prevOutput);
        } else {
            float amt = (0.5f - pct(boost)) * 2.f;
            return applyCompression(input, amt);
        }
    }

    float bp_x1 = 0.f, bp_x2 = 0.f, bp_y1 = 0.f, bp_y2 = 0.f;
    float applyBandpass(float input) {
        float y = b0_a0 * input + b1_a0 * bp_x1 + b2_a0 * bp_x2 - a1_a0 * bp_y1 - a2_a0 * bp_y2;
        bp_x2 = bp_x1; bp_x1 = input; bp_y2 = bp_y1; bp_y1 = y;
        return y * gainComp;
    }
};