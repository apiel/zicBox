/** Description:
This complex C++ header defines a dedicated audio synthesis module called `FmEngine`. This engine is specialized in Frequency Modulation (FM) synthesis, a technique renowned for creating distinct, often metallic, percussive, or digital tones.

**Basic Idea of Operation**

The engine generates sound using two primary components: a "carrier" waveform and a "modulator" waveform. The crucial step in FM synthesis is that the modulator subtly and continuously changes the frequency (pitch) of the carrier. This interaction, rather than simply mixing the two waves, creates complex new harmonics that define the timbre.

**Internal Sound Processing Pipeline**

1.  **FM Generation:** The engine uses internal wave generators (oscillators) to create the core sound. User controls like "Tone," "Snap," and "Color" determine the relationship between the carrier and modulator, controlling how complex or harsh the sound is.
2.  **Pitch Shaping:** A dedicated pitch envelope ("PitchEnv") allows the user to program rapid pitch changes that occur immediately after a note is triggered, common in drum synthesis.
3.  **LFO Tremolo:** A low-frequency oscillator ("LFO Rate") is used to introduce a slow, subtle volume modulation (tremolo), adding movement to the output.
4.  **Filtering:** An advanced filter ("Cutoff" and "Resonance") processes the generated sound, allowing the user to brighten or dampen the final output.
5.  **Multi-Effects:** Finally, the sound passes through a configurable multi-effects unit ("FX Type" and "FX Amount") for final polish, such as adding spatial effects.

In short, the `FmEngine` is a self-contained unit that receives a note input, calculates the raw FM waveform based on 10 user-controlled parameters, and then applies shaping, filtering, and effects before outputting the final audio signal.

sha: ba36583857f1d91e396ac15ef0a57631dfb5c0500098943f9a7d8349b19ee3e3 
*/
#pragma once

#include "plugins/audio/MultiEngine/Engine.h"
#include "audio/EnvelopDrumAmp.h"
#include "audio/MMfilter.h"
#include "audio/MultiFx.h"
#include "audio/WavetableGenerator2.h"
#include "plugins/audio/utils/valMMfilterCutoff.h"
#include "helpers/math.h"

class FmEngine : public Engine {
protected:
    WavetableGenerator carrier;
    WavetableGenerator mod;

    EnvelopDrumAmp envPitch;
    MultiFx multiFx;
    MMfilter filter;

    float sampleIndexCar = 0.0f;
    float sampleIndexMod = 0.0f;
    float sampleIndexLfo = 0.0f;

    float velocity = 1.0f;

    // params
    float toneRatio = 1.0f;
    float snapRatio = 2.0f;
    float modIndex = 0.0f;

public:
    // --- 10 parameters ---
    Val& body = val(0.0f, "BODY", { .label = "Body", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(body.get());
    });

    Val& tone = val(100.0f, "TONE", { .label = "Tone", .min = 0, .max = 400 }, [&](auto p) {
        p.val.setFloat(p.value);
        toneRatio = 0.25f + p.val.pct() * 4.0f; // ~0.25 to 4
    });

    Val& snap = val(100.0f, "SNAP", { .label = "Snap", .min = 0, .max = 400 }, [&](auto p) {
        p.val.setFloat(p.value);
        snapRatio = 0.5f + p.val.pct() * 8.0f; // ~0.5 to 8
    });

    Val& color = val(0.0f, "COLOR", { .label = "Color", .step = 0.1, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        modIndex = p.val.pct() * 10.0f; // FM depth
    });

    Val& pitchEnv = val(50.0f, "PITCH_ENV", { .label = "PitchEnv", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        envPitch.morph(p.val.pct());
    });

    // LFO rate only, no mix → tremolo depth is implicit in envelope
    Val& lfoRate = val(50.0f, "LFO_RATE", { .label = "LFO Rate", .min = 0.0f, .max = 50.0f, .step = 0.1, .floatingPoint = 1, .unit = "Hz" });

    Val& cutoff = val(0.0, "CUTOFF", { .label = "LPF | HPF", .type = VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });
    Val& resonance = val(0.0, "RESONANCE", { .label = "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX type", .type =VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX edit", .unit = "%" });

    // --- constructor ---
    FmEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "FM")
        , carrier(p.lookupTable, p.sampleRate)
        , mod(p.lookupTable, p.sampleRate)
        , multiFx(props.sampleRate, props.lookupTable)
    {
        carrier.setType(WavetableGenerator::Type::Sine);
        mod.setType(WavetableGenerator::Type::Sine);
        initValues();
    }

    void sample(float* buf, float envAmpVal) override
    {
        if (envAmpVal == 0.0f) {
            float out = buf[track];
            out = multiFx.apply(out, fxAmount.pct());
            buf[track] = out * velocity;
            return;
        }

        float pitchEnvVal = envAmpVal; // envelope morph applied

        // base + pitch envelope
        float freq = baseFreq * toneRatio * powf(2.0f, pitchEnvVal * 2.0f);
        float modFreq = baseFreq * snapRatio;

        float modSignal = mod.sample(&sampleIndexMod, modFreq);
        float car = carrier.sample(&sampleIndexCar, freq + modSignal * modIndex * freq);

        // --- LFO tremolo ---
        float lfoHz = lfoRate.get();
        float lfoVal = 0.0f;
        if (lfoHz > 0.0f) {
            float phaseInc = (2.0f * M_PI * lfoHz) / props.sampleRate;
            sampleIndexLfo += phaseInc;
            if (sampleIndexLfo >= 2.0f * M_PI) sampleIndexLfo -= 2.0f * M_PI;
            // lfoVal = (fastSin2(sampleIndexLfo) + 1.0f) * 0.5f; // map [-1,1] → [0,1]
            lfoVal = (sinf(sampleIndexLfo) + 1.0f) * 0.5f; // map [-1,1] → [0,1]
        }

        float out = car * (1.0f - 0.5f * lfoVal); // depth fixed at 50% tremolo

        out = filter.process(out);
        out = out * envAmpVal * velocity;
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        sampleIndexCar = 0.0f;
        sampleIndexMod = 0.0f;
        sampleIndexLfo = 0.0f;
        setBaseFreq(body.get(), note);
    }
};
