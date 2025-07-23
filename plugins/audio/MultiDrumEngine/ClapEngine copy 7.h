#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/effects/applyBoost.h"
#include "plugins/audio/utils/effects/applyCompression.h"
#include "plugins/audio/utils/effects/applyReverb.h"
#include "plugins/audio/utils/effects/applyWaveshape.h"
#include <array>
#include <cmath>

class ClapEngine : public DrumEngine {
public:
    ClapEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Clap")
    {
        initValues();
    }

    // --- Parameters (10 total) ---
    Val& decay = val(25, "DECAY", { "Burst Decay", .unit = "%" });
    Val& burstCount = val(5, "BURSTS", { "Bursts", .min = 1.f, .max = 10.f });
    Val& burstSpacing = val(50, "SPACING", { "Spacing", .unit = "%" });
    Val& noiseColor = val(15, "NOISE_COLOR", { "Noise Color", .unit = "%" });

    Val& bodyTone = val(30, "BODY_TONE", { "Body Tone", .unit = "%" });
    Val& bodyDecay = val(30, "BODY_DECAY", { "Body Decay", .unit = "%" });
    Val& bodyFilter = val(50, "BODY_FILTER", { "Body Filter", .unit = "%" });

    Val& stereo = val(50, "STEREO", { "Stereo", .unit = "%" });
    Val& boost = val(0.0f, "BOOST", { "Boost", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& reverb = val(30, "REVERB", { "Reverb", .unit = "%" });

    static constexpr int REVERB_SIZE = 48000;
    float reverbBuf[REVERB_SIZE] = {};
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

    void noteOn(uint8_t, float, void* = nullptr) override
    {
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
            output += burst * (1.f + 0.5f * (props.lookupTable->getNoise() - 0.5f) * stereo.pct());

            env *= expf(-1.f / (props.sampleRate * decayTime));
        } else if (burstIndex >= int(burstCount.get())) {
            active = false;
        }

        // Add tonal body
        if (bodyEnv > 0.f) {
            float decayTime = bodyDecay.pct() * 0.3f + 0.01f;
            bodyFreq = 100.f + bodyTone.pct() * 100.f;

            float phaseStep = 2.f * M_PI * bodyFreq / props.sampleRate;
            // float tone = sinf(bodyPhase);
            float sine = sinf(bodyPhase);
            float tone = (sine > 0.f ?  0.6f : - 0.6f);

            bodyPhase += phaseStep;

            float cutoff = 80.f + bodyFilter.pct() * bodyEnv * 400.f;
            float rc = 1.0f / (2.f * M_PI * cutoff);
            float dt = 1.f / props.sampleRate;
            float alpha = dt / (rc + dt); // smoother cutoff calc

            // 1-pole LP & fake BP
            float hp = tone - lpState;
            lpState += alpha * hp;
            bpState += alpha * (lpState - bpState);

            float body = lpState * bodyEnv;

            output += body * 0.5f; // Mix in softly

            bodyEnv *= expf(-1.f / (props.sampleRate * decayTime));
        }

        output = applyBoostOrCompression(output);
        output = applyReverb(output, reverb.pct(), reverbBuf, rIdx, REVERB_SIZE);

        buf[track] = output * envAmp;
    }

    void sampleOff(float* buf) override
    {
        buf[track] = applyReverb(buf[track], reverb.pct(), reverbBuf, rIdx, REVERB_SIZE);
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
};
