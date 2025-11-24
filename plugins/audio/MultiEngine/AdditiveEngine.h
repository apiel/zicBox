/** Description:
This code defines an `AdditiveEngine`, a specialized digital sound generator (or synthesizer voice) designed to operate within a larger audio software plugin.

The engine uses **Additive Synthesis**, a method where sound is created by combining multiple synchronized simple sine waves, known as "partials," to build a complex tone. This approach is highly effective for synthesizing bell-like sounds or organ-style timbres.

The engine can generate up to eight simultaneous partials. Its core behavior is controlled by ten parameters that the user can adjust:

1.  **Tone & Timbre Controls:** Parameters like "Harmonics" determine how many partials are active, affecting the richness of the sound. "Harmonic Decay" controls how quickly the higher, brighter partials fade out. The "Odd/Even" control biases the volume toward specific partials to dramatically alter the tonal quality.
2.  **Fine Tuning:** "Spread" and "Inharm" introduce subtle detuning and pitch variations among the partials, resulting in a thicker, less sterile sound. "Body" adjusts the fundamental pitch.
3.  **Modulation:** "LFO Rate" and "LFO Depth" manage a Low-Frequency Oscillator, which introduces slow, cyclical movement (like a tremolo effect) to the output volume.
4.  **Effects:** Integrated controls manage a built-in `MultiFx` unit, allowing an external effect to be applied directly to the generated sound before it leaves the engine.

During the sound generation process, the engine calculates the waveform for each partial based on the set parameters, sums them up, applies the LFO modulation, and finally runs the combined signal through the chosen effect, producing the final audio output. The engine also handles setup, resetting all internal timers whenever a new musical note is played.

sha: 6d16ecb74b71ad6c5bfe2f836f199366b3a6bbfc3c682cc979bcff716b33647c 
*/
#pragma once

#include "helpers/math.h"
#include "plugins/audio/MultiEngine/Engine.h"
#include "audio/MultiFx.h"

class AdditiveEngine : public Engine {
protected:
    static constexpr int MAX_PARTIALS = 8;
    MultiFx multiFx;

    float velocity = 1.0f;
    float harmonicDecay = 1.0f;

    float phases[MAX_PARTIALS] = { 0.0f };

    // LFO state
    float lfoPhase = 0.0f;

    // odd/even bias
    float oddEvenBias = 0.5f;

public:
    // --- 10 parameters ---
    Val& body = val(0.0f, "BODY", { .label = "Body", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(p.val.get());
    });

    Val& harmonics = val(50.0f, "HARMONICS", { .label = "Harmonics", .unit = "%" });

    Val& spread = val(0.0f, "SPREAD", { .label = "Spread", .unit = "%" });

    Val& inharm = val(0.0f, "INHARM", { .label = "Inharm", .unit = "%" });

    Val& harmonicDecayVal = val(50.0f, "HARM_DECAY", { .label = "Decay", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        harmonicDecay = 0.2f + p.val.pct() * 2.0f; // rolloff
    });

    Val& oddEven = val(50.0f, "ODDEVEN", { .label = "Odd/Even", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        oddEvenBias = p.val.pct(); // 0 = only even, 1 = only odd, 0.5 = neutral
    });

    Val& lfoRate = val(0.0f, "LFO_RATE", { .label = "LFO Rate", .min = 0.0f, .max = 100.0f, .step = 0.1, .floatingPoint = 1, .unit = "Hz" });

    Val& lfoDepth = val(0.0f, "LFO_DEPTH", { .label = "LFO Depth", .unit = "%" });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);

    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX edit", .unit = "%" });

    // --- constructor ---
    AdditiveEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "Aditiv")
        , multiFx(props.sampleRate, props.lookupTable)
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

        int numPartials = 1 + int(harmonics.pct() * (MAX_PARTIALS - 1));
        float spreadAmt = spread.pct() * 0.1f;
        float inharmAmt = inharm.pct() * 0.5f;

        float sampleSum = 0.0f;
        for (int i = 0; i < numPartials; i++) {
            float freq = baseFreq * (i + 1 + inharmAmt * i);
            freq *= (1.0f + (i - numPartials / 2) * spreadAmt);

            float phaseInc = 2.0f * M_PI * freq / props.sampleRate;
            phases[i] += phaseInc;
            if (phases[i] > 2.0f * M_PI)
                phases[i] -= 2.0f * M_PI;

            // --- Odd/Even bias ---
            bool isOdd = ((i + 1) % 2 == 1);
            float biasAmp = isOdd ? oddEvenBias : (1.0f - oddEvenBias);

            float amp = powf(1.0f / (i + 1), harmonicDecay) * biasAmp;
            sampleSum += fastSin2(phases[i]) * amp;
        }

        float out = sampleSum / numPartials;

        // --- Apply LFO (tremolo) ---
        if (lfoRate.get() > 0.0f) {
            float lfoInc = 2.0f * M_PI * lfoRate.get() / props.sampleRate;
            lfoPhase += lfoInc;
            if (lfoPhase > 2.0f * M_PI)
                lfoPhase -= 2.0f * M_PI;
            float lfoVal = 0.5f * (1.0f + fastSin2(lfoPhase)); // [0..1]
            out *= (1.0f - lfoDepth.pct()) + lfoVal * lfoDepth.pct();
        }

        out = out * envAmpVal * velocity;
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        setBaseFreq(body.get(), note);
        for (int i = 0; i < MAX_PARTIALS; i++) {
            phases[i] = 0.0f;
        }
        lfoPhase = 0.0f;
    }
};
