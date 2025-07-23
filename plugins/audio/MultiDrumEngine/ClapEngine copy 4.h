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

    Val& stereo = val(50, "STEREO", { "Stereo", .unit = "%" });
    Val& boost = val(0.0f, "BOOST", { "Boost", .unit = "%" });
    Val& compression = val(0.0f, "COMPRESSION", { "Compression", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0, .step = 1.0, .unit = "%" });
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
            float tone = sinf(bodyPhase) * bodyEnv;
            bodyPhase += phaseStep;

            output += tone * 0.3f; // Mix in softly

            bodyEnv *= expf(-1.f / (props.sampleRate * decayTime));
        }

        output = applyBoost(output, boost.pct(), prevInput, prevOutput);
        output = doCompression(output);
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

    float doCompression(float input)
    {
        if (compression.pct() == 0.5f) {
            return input;
        }
        if (compression.pct() > 0.5f) {
            float amount = compression.pct() * 2 - 1.0f;
            return applyCompression(input, amount);
        }
        float amount = 1 - compression.pct() * 2;
        return applyWaveshape(input, amount);
    }
};
