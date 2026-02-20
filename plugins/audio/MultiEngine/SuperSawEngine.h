/** Description:
This code defines the structure and behavior for a specialized sound generator named `SuperSawEngine`. This engine is designed to create the thick, rich sound characteristic of a "supersaw" waveform, commonly used in electronic music synthesis. It is built as a core component within a larger audio plugin framework.

### How the Engine Works

The fundamental purpose is audio synthesis, carried out by layering multiple sawtooth waves. The process involves:

1.  **Layered Generation:** The engine generates up to seven individual sawtooth waveforms simultaneously.
2.  **Detuning:** It slightly detunes these layers from one another (the "detune" parameter is crucial here) to create a wide, swirling sound.
3.  **Mixing and Noise:** The layered waves are summed together, and an optional noise component can be mixed in.
4.  **Filtering:** The combined signal passes through a specialized audio filter (`MMfilter`) controlled by user settings for cutoff frequency and resonance, shaping the overall timbre.
5.  **Dual Effects:** Finally, the sound is processed by two independent effects units (`MultiFx` and `MultiFx2`) before the final output, adding depth like chorus, delay, or distortion.

### Key User Controls (Parameters)

The engine provides ten customizable parameters (knobs/sliders) that control its behavior:

*   **Body & Voices:** Adjusts the base pitch and determines how many of the seven layers are currently active.
*   **Detune:** Controls the amount of frequency offset between the layered voices, defining the width of the sound.
*   **Filter Controls:** Separate settings for **Cutoff** (where the filter acts) and **Resonance** (how sharp the filter sound is).
*   **Noise Mix:** Balances the amount of pure synthesized sound versus added static noise.
*   **Dual Effects:** Four parameters control two distinct effects slots, allowing users to select the **FX Type** (e.g., reverb or chorus) and the **FX Amount** (intensity) for each slot.

sha: 19d228c7ac4b4016a0d03a06c98fc8951a357f6a97c6716c18a725e29fd85948 
*/
#pragma once

#include "plugins/audio/MultiEngine/Engine.h"
#include "audio/MMfilter.h"
#include "audio/MultiFx.h"
#include "plugins/audio/utils/valMMfilterCutoff.h"
#include <cmath>

class SuperSawEngine : public Engine {
protected:
    static constexpr int MAX_VOICES = 7; // number of saw layers
    MultiFx multiFx;
    MultiFx multiFx2;
    MMfilter filter;

    float velocity = 1.0f;

    float phases[MAX_VOICES] = { 0.0f };

public:
    // --- 10 parameters ---
    Val& body = val(0.0f, "BODY", { .label = "Body", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(body.get());
    });

    Val& voices = val(5.0f, "VOICES", { .label = "Voices", .min = 1, .max = MAX_VOICES });

    Val& detune = val(10.0f, "DETUNE", { .label = "Detune", .unit = "%" });

    Val& noiseMix = val(0.0f, "NOISE", { .label = "Noise", .unit = "%" });

    Val& filterCutoff = val(0.0f, "CUTOFF", { .label = "LPF | HPF", .type = VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });

    Val& filterRes = val(0.0f, "RES", { .label = "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);

    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX edit", .unit = "%" });

    Val& fx2Type = val(0, "FX2_TYPE", { .label = "FX2 type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx2.setFxType);

    Val& fx2Amount = val(0, "FX2_AMOUNT", { .label = "FX2 edit", .unit = "%" });

    // --- constructor ---
    SuperSawEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "SuperSaw")
        , multiFx(props.sampleRate)
        , multiFx2(props.sampleRate)
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

        int numVoices = voices.get();
        float detuneAmt = detune.pct() * 0.05f;

        float sampleSum = 0.0f;
        for (int i = 0; i < numVoices; ++i) {
            float det = ((float)i - numVoices / 2.0f) * detuneAmt;
            float freq = baseFreq * (1.0f + det);

            float phaseInc = 2.0f * M_PI * freq / props.sampleRate;
            phases[i] += phaseInc;
            if (phases[i] > 2.0f * M_PI)
                phases[i] -= 2.0f * M_PI;

            sampleSum += (2.0f * (phases[i] / (2.0f * M_PI)) - 1.0f); // saw wave approximation
        }

        float out = sampleSum / numVoices;

        // noise
        float n = props.lookupTable->getNoise();
        out = out * (1.0f - noiseMix.pct()) + n * noiseMix.pct();

        out = filter.process(out);
        out = out * envAmpVal * velocity;
        out = multiFx.apply(out, fxAmount.pct());
        out = multiFx2.apply(out, fx2Amount.pct());

        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        setBaseFreq(body.get(), note);
        for (int i = 0; i < MAX_VOICES; ++i)
            phases[i] = 0.0f;
    }
};
