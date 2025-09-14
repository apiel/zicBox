#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/effects/applyBoost.h"
#include "plugins/audio/utils/effects/applyCompression.h"
#include "plugins/audio/utils/effects/applyReverb.h"
#include <array>
#include <cmath>

class ClapEngine : public DrumEngine {
protected:
    float velocity = 1.0f;

    REVERB_BUFFER
    int rIdx = 0;

    float burstTimer = 0.f;
    int burstIndex = 0;
    float env = 0.f;
    float pink = 0.f;
    float phase = 0.f;
    bool active = false;

    float bodyEnv = 0.f;
    float bodyPhase = 0.f;
    float bodyFreq = 300.f;
    float lpState = 0.f, bpState = 0.f;

    // --- Parameters (10 total) ---
    Val& decay = val(25, "DECAY", { "Burst Decay", .unit = "%" });
    Val& burstCount = val(5, "BURSTS", { "Bursts", .min = 1.f, .max = 10.f });
    Val& burstSpacing = val(30, "SPACING", { "Spacing", .unit = "%" });
    Val& noiseColor = val(70, "NOISE_COLOR", { "Noise Color", .unit = "%" });

    Val& filterFreq = val(0, "FILTER_FREQ", { "Cutoff", .unit = "%" }); // 1–4 kHz
    Val& filterReso = val(30, "FILTER_RESO", { "Resonance", .unit = "%" });

    Val& punch = val(100, "PUNCH", { "Punch", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& transient = val(0.0, "TRANSIENT", { "Transient", .unit = "%" });

    Val& boost = val(0.0f, "BOOST", { "Boost", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& reverb = val(20, "REVERB", { "Reverb", .unit = "%" });

public:
    ClapEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Clap")
    {
        initValues();
    }

    void noteOn(uint8_t, float _velocity, void* = nullptr) override
    {
        velocity = _velocity;
        burstTimer = 0.f;
        burstIndex = 0;
        env = 1.f;
        phase = 0.f;
        pink = 0.f;
        active = true;

        bodyEnv = 1.f;
        bodyPhase = 0.f;
        lpState = 0.f;
        bpState = 0.f;
    }

    void sampleOn(float* buf, float envAmp, int sc, int ts) override
    {
        if (!active)
            return;

        float t = float(sc) / ts;
        float spacing = burstSpacing.pct() * 0.03f + 0.01f;
        float decayTime = decay.pct() * 0.3f + 0.02f;

        float output = 0.0f;

        // Generate burst envelope
        if (burstIndex < int(burstCount.get())) {
            burstTimer += 1.f / props.sampleRate;
            if (burstTimer >= spacing) {
                burstTimer -= spacing;
                burstIndex++;
                env = 1.f;
            }
        }

        if (env > 0.f) {
            // Pink noise
            float white = props.lookupTable->getNoise() * 2.f - 1.f;
            pink = 0.98f * pink + 0.02f * white;
            float noise = pink * (1.f - noiseColor.pct()) + white * noiseColor.pct();

            float burst = noise * env;
            // output += burst * (1.f + 0.5f * (props.lookupTable->getNoise() - 0.5f));
            output += burst;

            env *= expf(-1.f / (props.sampleRate * decayTime));
        } else if (burstIndex >= int(burstCount.get())) {
            active = false;
        }

        output = applyBandpass(output);

        if (punch.get() < 0) {
            float amt = (0.5f - punch.pct()) * 2.f;
            output = CLAMP(output + output * amt * 8, -1.0f, 1.0f);
        } else if (t < 0.02f) {
            float amt = (punch.pct() - 0.5f) * 2.f;
            output *= 1.f + amt * 2.f;
        }

        if (t < 0.01f) {
            float highpassed = output - lpState;
            lpState += 0.01f * (output - lpState); // simple LPF
            output += highpassed * transient.pct() * 2.0f;
            if (t < 0.001f) {
                float spike = (props.lookupTable->getNoise() - 0.5f) * 10.f;
                output += spike * transient.pct();
            }
        }

        output = applyBoostOrCompression(output);
        output = applyReverb(output, reverb.pct(), buffer, rIdx);

        buf[track] = output * envAmp * velocity;
    }

    void sampleOff(float* buf) override
    {
        buf[track] = applyReverb(buf[track], reverb.pct(), buffer, rIdx);
    }

private:
    float prevInput = 0.f;
    float prevOutput = 0.f;

    float applyBoostOrCompression(float input)
    {
        if (boost.pct() == 0.5f)
            return input;
        if (boost.pct() > 0.5f) {
            float amt = (boost.pct() - 0.5f) * 2.f;
            return applyBoost(input, amt, prevInput, prevOutput);
        } else {
            float amt = (0.5f - boost.pct()) * 2.f;
            return applyCompression(input, amt);
        }
    }

    // BPF states
    float bp_x1 = 0.f, bp_x2 = 0.f;
    float bp_y1 = 0.f, bp_y2 = 0.f;
    float applyBandpass(float x)
    {
        // Biquad bandpass filter (cookbook formula)
        float f0 = 1000.f + filterFreq.pct() * 3000.f; // 1kHz to 4kHz
        float Q = 1.0f + filterReso.pct() * 3.0f; // Q: 1 to 4

        float omega = 2.f * M_PI * f0 / props.sampleRate;
        float alpha = sinf(omega) / (2.f * Q);

        float b0 = alpha;
        float b1 = 0.f;
        float b2 = -alpha;
        float a0 = 1.f + alpha;
        float a1 = -2.f * cosf(omega);
        float a2 = 1.f - alpha;

        // Direct Form I
        float y = (b0 / a0) * x + (b1 / a0) * bp_x1 + (b2 / a0) * bp_x2
            - (a1 / a0) * bp_y1 - (a2 / a0) * bp_y2;

        // Shift delay line
        bp_x2 = bp_x1;
        bp_x1 = x;
        bp_y2 = bp_y1;
        bp_y1 = y;

        float gainComp = 1.f + Q;
        return y * gainComp;
    }
};
