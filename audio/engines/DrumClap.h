#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/effects/applyBoost.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyReverb.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"
#include "helpers/clamp.h"

#include <array>
#include <cmath>

class DrumClap : public EngineBase<DrumClap> {

protected:
    const float sampleRate;

    float* reverbBuffer = nullptr;
    int reverbIndex = 0;

    float velocity = 1.0f;

    float burstTimer = 0.f;
    int burstIndex = 0;
    float env = 0.f;
    float pink = 0.f;
    bool active = false;
    float ampEnv = 0.0f;
    float ampStep = 0.0f;

    float lpState = 0.f, bpState = 0.f;

    float time = 0.f;
    float timeRatio = 0.f;

    // Pre-calculated filter coefficients
    float b0_a0, b1_a0, b2_a0, a1_a0, a2_a0;
    float gainComp = 1.0f;

    // Rain state
    float rainTimer = 0.f;
    float rainDropEnv = 0.f;
    float rainDropInterval = 0.f;
    float rainPink = 0.f;

    void updateFilter()
    {
        float f0 = 1000.f + pct(filterFreq) * 3000.f;
        float Q = 1.0f + pct(filterReso) * 3.0f;

        float omega = 2.f * 3.14159265f * f0 / sampleRate;
        // Use fast hardware-accelerated sin/cos if available
        float s = Math::sin(omega);
        float c = Math::cos(omega);
        float alpha = s / (2.f * Q);

        float a0 = 1.f + alpha;
        float invA0 = 1.0f / a0; // Calculate reciprocal once

        b0_a0 = alpha * invA0;
        b1_a0 = 0.f;
        b2_a0 = -alpha * invA0;
        a1_a0 = (-2.f * c) * invA0;
        a2_a0 = (1.f - alpha) * invA0;

        gainComp = 1.f + Q;
    }

public:
    Param params[12] = {
        { .label = "Duration", .unit = "ms", .value = 500.0f, .min = 10.0f, .max = 3000.0f, .step = 10.0f },
        { .label = "Burst Decay", .unit = "%", .value = 25.0f },
        { .label = "Bursts", .value = 5.0f, .min = 1.f, .max = 10.f },
        { .label = "Spacing", .unit = "%", .value = 30.0f },
        { .label = "Noise Color", .unit = "%", .value = 70.0f },
        { .label = "Cutoff", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float val) { static_cast<DrumClap*>(ctx)->updateFilter(); } }, // 1–4 kHz
        { .label = "Resonance", .unit = "%", .value = 30.0f, .onUpdate = [](void* ctx, float val) { static_cast<DrumClap*>(ctx)->updateFilter(); } },
        { .label = "Punch", .unit = "%", .value = 100.0f, .min = -100.f, .max = 100.f, .type = VALUE_CENTERED },
        { .label = "Transient", .unit = "%", .value = 0.0f },
        { .label = "Boost", .unit = "%", .value = 0.0f, .min = -100.f, .max = 100.f, .type = VALUE_CENTERED },
        { .label = "Reverb", .unit = "%", .value = 20.0f, .min = -100.f },
        { .label = "Rain", .unit = "%", .value = 0.0f },
    };

    Param& duration = params[0];
    Param& burstDecay = params[1];
    Param& burstCount = params[2];
    Param& burstSpacing = params[3];
    Param& noiseColor = params[4];
    Param& filterFreq = params[5];
    Param& filterReso = params[6];
    Param& punch = params[7];
    Param& transient = params[8];
    Param& boost = params[9];
    Param& reverb = params[10];
    Param& rain = params[11];

    DrumClap(const float sampleRate, float* rvBuffer)
        : EngineBase(Drum, "Clap", params)
        , sampleRate(sampleRate)
        , reverbBuffer(rvBuffer)
    {
        init();
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        burstTimer = 0.f;
        burstIndex = 0;
        env = 1.f;
        pink = 0.f;
        active = true;

        lpState = 0.f;
        bpState = 0.f;

        timeRatio = 1.0f / sampleRate;
        time = 0.f;

        ampEnv = 1.0f;
        float durSamples = std::max(1.0f, sampleRate * (duration.value * 0.001f));
        ampStep = 1.0f / durSamples;

        // Reset rain state
        rainTimer = 0.f;
        rainDropEnv = 0.f;
        rainDropInterval = sampleRate * (0.04f + (1.f - pct(rain)) * 0.12f);
        rainPink = 0.f;
    }

    int totalSamples = 0;
    float sampleImpl()
    {
        if (ampEnv <= 0.0f) return applyRvb(0.0f);
        float currentAmp = ampEnv;
        ampEnv -= ampStep;

        time += timeRatio;
        float spacing = pct(burstSpacing) * 0.03f + 0.01f;
        float decayTime = pct(burstDecay) * 0.3f + 0.02f;

        float output = 0.0f;

        // Generate burst envelope
        if (burstIndex < int(burstCount.value)) {
            burstTimer += 1.f / sampleRate;
            if (burstTimer >= spacing) {
                burstTimer -= spacing;
                burstIndex++;
                env = 1.f;
            }
        }

        if (env > 0.f) {
            // Pink noise
            float white = Noise::sample();
            pink = 0.98f * pink + 0.02f * white;
            float noise = pink * (1.f - pct(noiseColor)) + white * pct(noiseColor);

            float burst = noise * env;
            output += burst;

            env *= Math::exp(-1.f / (sampleRate * decayTime));
        } else if (burstIndex >= int(burstCount.value)) {
            active = false;
        }

        output = applyBandpass(output);

        if (punch.value < 0) {
            float amt = (0.5f - pct(punch)) * 2.f;
            output = CLAMP(output + output * amt * 8, -1.0f, 1.0f);
        } else if (time < 0.02f) {
            float amt = (pct(punch) - 0.5f) * 2.f;
            output *= 1.f + amt * 2.f;
        }

        if (time < 0.01f) {
            float highpassed = output - lpState;
            lpState += 0.01f * (output - lpState); // simple LPF
            output += highpassed * pct(transient) * 2.0f;
            if (time < 0.001f) {
                float spike = (Noise::get() - 0.5f) * 10.f;
                output += spike * pct(transient);
            }
        }

        // Rain effect: sparse high-passed noise droplets that trail after the clap
        if (rain.value > 0.0f) {
            float rainAmt = pct(rain);

            rainTimer += 1.f;
            if (rainTimer >= rainDropInterval) {
                rainTimer = 0.f;
                // Randomise next drop interval — denser as rain amount increases
                float minInterval = sampleRate * (0.005f + (1.f - rainAmt) * 0.03f);
                float maxInterval = sampleRate * (0.04f + (1.f - rainAmt) * 0.12f);
                rainDropInterval = minInterval + (Noise::get()) * (maxInterval - minInterval);
                // Each drop starts with a short sharp envelope
                rainDropEnv = 0.3f + rainAmt * 0.7f;
            }

            if (rainDropEnv > 0.0001f) {
                // Bright, high-passed white noise burst for the drip texture
                float w = Noise::sample();
                rainPink = 0.85f * rainPink + 0.15f * w; // mild LPF to soften slightly
                float drip = (w - rainPink) * 1.5f; // high-pass by subtracting smoothed
                float dropOut = drip * rainDropEnv;

                // Decay the drop quickly — shorter decay for denser rain
                float dropDecay = 0.002f + (1.f - rainAmt) * 0.008f;
                rainDropEnv *= Math::exp(-1.f / (sampleRate * dropDecay));

                output += dropOut * rainAmt * 0.35f;
            }
        }

        output = applyBoostOrCompression(output);
        output = applyRvb(output);

        return output * currentAmp * velocity;
    }

private:
    float applyRvb(float output)
    {
        if (reverb.value == 0.0f) return output;

        if (reverb.value < 0.0f) {
            return applyMiniReverb(output, -reverb.value * 0.01f, reverbBuffer, reverbIndex);
        }

        return applyReverb(output, reverb.value * 0.01f, reverbBuffer, reverbIndex);
    }

    float prevInput = 0.f;
    float prevOutput = 0.f;

    float applyBoostOrCompression(float input)
    {
        if (boost.value == 0.0f)
            return input;
        if (boost.value > 0.0f) {
            float amt = (pct(boost) - 0.5f) * 2.f;
            return applyBoost(input, amt, prevInput, prevOutput);
        } else {
            float amt = (0.5f - pct(boost)) * 2.f;
            return applyCompression(input, amt);
        }
    }

    // BPF states
    float bp_x1 = 0.f, bp_x2 = 0.f;
    float bp_y1 = 0.f, bp_y2 = 0.f;
    float applyBandpass(float input)
    {
        float y = b0_a0 * input + b1_a0 * bp_x1 + b2_a0 * bp_x2
            - a1_a0 * bp_y1 - a2_a0 * bp_y2;

        bp_x2 = bp_x1;
        bp_x1 = input;
        bp_y2 = bp_y1;
        bp_y1 = y;
        return y * gainComp;
    }
};