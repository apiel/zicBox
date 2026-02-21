/** Description:
This code defines an advanced sound generator component specifically tailored to create complex audio, labeled the "SpaceShip Engine." It is designed to run within a larger audio processing framework, functioning as the core sound source for an audio plugin.

**How the Sound is Created:**

The engine generates its output using three separate foundational sound generators (oscillators). The unique character of the sound comes from combining these generators at adjustable levels, along with a "Detune" control that slightly shifts their frequencies apart to create a thicker, more textured hum.

**Key Features and Controls:**

1.  **Glide and Pitch Control:** When a new note or pitch is requested, the engine doesn't jump instantly to the new frequency. Instead, it smoothly "glides" over a short period, simulating the slow spool-up or shut-down of mechanical systems.
2.  **Modulation (LFO):** A Low-Frequency Oscillator (LFO) is used to introduce slow, subtle movement. It gently varies the pitch of the sound, adding a living quality or a slight wobble. Users can control both the speed and the depth of this modulation.
3.  **Tone Shaping (Saturation):** After the initial sound is generated, a "Saturation" or "Soft Clip" function intentionally adds a controlled amount of distortion. This shapes the tone, allowing the sound to range from a clean hum to a harsh, overloaded mechanical roar.
4.  **Effects Processing:** The final sound is passed through a built-in effects unit (`MultiFx`). This allows the user to apply common post-processing effects, such as delay or reverb, to the engine sound before it reaches the final output.

The `SpaceShipEngine` exposes numerous parameters (like Pitch, Detune, LFO Rate, and FX Amount) that allow the user to sculpt and customize the sound dynamically.

sha: 68ea2d5c5f6029027622d9e4d3c77006ccd7a55f856cd09167a3afb1a70aa121 
*/
#pragma once

#include "plugins/audio/MultiEngine/Engine.h"
#include "plugins/audio/utils/valMultiFx.h"
#include "helpers/math.h"

#include <cmath>

class SpaceShipEngine : public Engine {
protected:
    static constexpr int NUM_OSC = 3;

    MultiFx multiFx;

    float baseFreq = 50.0f; // low engine hum
    float velocity = 1.0f;

    float oscPhases[NUM_OSC] = { 0.0f };
    float oscRatios[NUM_OSC] = { 1.0f, 1.5f, 2.2f };
    float glideTarget = 50.0f;
    float glideCurrent = 50.0f;
    float glideSpeed = 0.01f;

    float lfoPhase = 0.0f;
    float lfoRateHz = 0.5f;
    float lfoDepth = 0.0f;

    float sampleOsc(int idx, float freq)
    {
        float phaseInc = 2.0f * M_PI * freq / props.sampleRate;
        oscPhases[idx] += phaseInc;
        if (oscPhases[idx] > 2.0f * M_PI)
            oscPhases[idx] -= 2.0f * M_PI;
        return fastSin2(oscPhases[idx]);
    }

    inline float softClip(float x, float amount)
    {
        // amount: 0 = no clipping, 1 = full hard-ish clip
        float k = amount * 5.0f + 0.1f; // scale factor for shaping
        return x / (1.0f + k * fabsf(x));
    }

public:
    uint8_t ptichNote = 60;
    // --- 10 parameters ---
    Val& basePitch = val(0.0f, "BASE_PITCH", { .label = "Pitch", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        // update target frequency
        glideTarget = 50.0f * powf(2.0f, (ptichNote - 60 + p.val.get()) / 12.0f);
    });

    Val& shape = val(50.0f, "SHAPE", { .label = "Saturation", .unit = "%" });

    Val& detune = val(10.0f, "DETUNE", { .label = "Detune", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        oscRatios[0] = 1.0f - p.val.pct() * 0.5f;
        oscRatios[1] = 1.5f;
        oscRatios[2] = 2.2f + p.val.pct() * 0.5f;
    });

    Val& oscMix1 = val(40.0f, "OSC1", { .label = "Osc1 Level", .unit = "%" });
    Val& oscMix2 = val(30.0f, "OSC2", { .label = "Osc2 Level", .unit = "%" });
    Val& oscMix3 = val(30.0f, "OSC3", { .label = "Osc3 Level", .unit = "%" });

    Val& lfoRate = val(0.5f, "LFO_RATE", { .label = "LFO Rate", .min = 0.0f, .max = 10.0f, .step = 0.1f, .unit = "Hz" });
    Val& lfoDepthVal = val(20.0f, "LFO_DEPTH", { .label = "LFO Depth", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        lfoDepth = p.val.pct() * 0.05f;
    });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX Type", .type = VALUE_STRING, .max = MultiFx::FX_COUNT - 1 }, valMultiFx(multiFx));
    Val& fxAmount = val(50.0f, "FX_AMOUNT", { .label = "FX Amount", .unit = "%" });

    // --- constructor ---
    SpaceShipEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "Space")
        , multiFx(props.sampleRate)
    {
        initValues();
    }

    void sample(float* buf, float envAmpVal) override
    {
        if (envAmpVal == 0.0f) {
            float out = buf[track];
            out = multiFx.apply(out, fxAmount.pct());
            buf[track] = out;
            return;
        }

        // glide
        glideCurrent += (glideTarget - glideCurrent) * glideSpeed;

        // LFO
        float lfoVal = 0.0f;
        if (lfoRate.get() > 0.0f) {
            float lfoInc = 2.0f * M_PI * lfoRate.get() / props.sampleRate;
            lfoPhase += lfoInc;
            if (lfoPhase > 2.0f * M_PI)
                lfoPhase -= 2.0f * M_PI;
            lfoVal = fastSin2(lfoPhase) * lfoDepth;
        }

        float sampleSum = 0.0f;
        float mixLevels[NUM_OSC] = { oscMix1.pct(), oscMix2.pct(), oscMix3.pct() };

        for (int i = 0; i < NUM_OSC; i++) {
            float freq = glideCurrent * oscRatios[i] * (1.0f + lfoVal);
            sampleSum += sampleOsc(i, freq) * mixLevels[i];
        }

        // float out = sampleSum;
        // out *= envAmpVal * velocity;
        // out = multiFx.apply(out, fxAmount.pct());
        // buf[track] = out;
        float out = sampleSum;
        out = softClip(out, shape.pct());
        out *= envAmpVal * velocity;
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        ptichNote = note;
        velocity = _velocity;
        // start glide at previous pitch or target
        glideCurrent = glideTarget;
        // glideTarget = 50.0f * powf(2.0f, (note - 60 + basePitch.get()) / 12.0f);
        glideTarget = 50.0f * powf(2.0f, (note - 24 + basePitch.get()) / 12.0f);

        for (int i = 0; i < NUM_OSC; i++)
            oscPhases[i] = 0.0f;

        lfoPhase = 0.0f;
    }
};
