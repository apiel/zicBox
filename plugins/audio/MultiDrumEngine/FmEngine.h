/** Description:
This code defines a specialized software module called `FmDrumEngine`, which functions as a virtual drum sound generator within an audio application or plugin.

**Core Concept: FM Synthesis**

The engine utilizes Frequency Modulation (FM) synthesis. This technique involves using one basic sound wave (the "modulator") to rapidly change the pitch of a second sound wave (the "carrier"). This interaction creates the complex, rich harmonics needed to mimic physical drums or produce unique electronic percussion sounds.

**How the Engine Works**

When a drum hit is triggered, the engine calculates the fundamental tone (the "Body"). It then relies on two built-in oscillators (the carrier and the modulator) to generate the core FM sound. A separate time-based controller (the pitch envelope) adds the initial high-to-low pitch movement typical of a drum’s attack phase (the "Snap").

The resulting sound is then mixed with controllable white noise, allowing the generation of snare or hi-hat textures. Finally, the signal passes through two essential tone-shaping stages: a variable filter (which can brighten or dull the sound) and a Multi-Effects unit (adding post-processing like delay or distortion) before the final audio sample is created.

**User Controls**

The engine provides ten customizable settings:

1.  **Body, Tone, Snap:** Control the drum’s fundamental frequency and the character of its initial attack.
2.  **Color (Mod Index):** Defines the depth of the FM modulation, controlling the complexity and timbre.
3.  **Noise Mix:** Blends the FM tone with white noise.
4.  **Pitch Env:** Adjusts how quickly the initial pitch modulation fades.
5.  **Filter & Resonance:** Fine-tune the brightness and intensity of the sound.
6.  **FX Type & Amount:** Select and adjust a range of built-in effects applied to the output.

This structure allows the user to sculpt highly specific and dynamic percussion sounds from a simple synthesized core.

sha: 6afdc2d33d26d1a3fcd6417ab9d584f0bb60e64ca72db61f9f0400dcdbbc9c07 
*/
#pragma once

#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/EnvelopDrumAmp.h"
#include "audio/MMfilter.h"
#include "audio/MultiFx.h"
#include "audio/WavetableGenerator2.h"
#include "plugins/audio/utils/valMMfilterCutoff.h"

class FmDrumEngine : public DrumEngine {
protected:
    WavetableGenerator carrier;
    WavetableGenerator mod;
    EnvelopDrumAmp envPitch;
    MultiFx multiFx;
    MMfilter filter;

    float sampleIndexCar = 0.0f;
    float sampleIndexMod = 0.0f;
    float baseFreq = 100.0f;
    float velocity = 1.0f;

    float noiseAmount = 0.0f;

    // params
    float bodyHz = 100.0f;
    float toneRatio = 1.0f;
    float snapRatio = 2.0f;
    float modIndex = 0.0f;

public:
    // --- 10 parameters ---
    Val& body = val(0.0f, "BODY", { .label = "Body", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        bodyHz = 220.0f * powf(2.0f, p.val.get() / 12.0f);
    });

    Val& tone = val(100.0f, "TONE", { .label = "Tone", .min = 0, .max = 400 }, [&](auto p) {
        p.val.setFloat(p.value);
        toneRatio = 0.25f + p.val.pct() * 4.0f; // ~0.25 to 4
    });

    Val& snap = val(100.0f, "SNAP", { .label = "Snap", .min = 0, .max = 400 }, [&](auto p) {
        p.val.setFloat(p.value);
        snapRatio = 0.5f + p.val.pct() * 8.0f; // ~0.5 to 8
    });

    Val& color = val(0.0f, "COLOR", { .label = "Color", .step = 0.1, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        modIndex = p.val.pct() * 10.0f; // FM depth
    });

    Val& pitchEnv = val(50.0f, "PITCH_ENV", { .label = "PitchEnv", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        envPitch.morph(p.val.pct());
    });

    Val& noiseMix = val(50.0f, "NOISE", { .label = "Noise", .unit = "%" });

    Val& cutoff = val(0.0, "CUTOFF", { .label = "LPF | HPF", .type = VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, valMMfilterCutoff(filter));
    Val& resonance = val(0.0, "RESONANCE", { .label = "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX type", .type = VALUE_STRING, .max = MFx::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX edit", .unit = "%" });

    // --- constructor ---
    FmDrumEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "FM")
        , carrier(p.lookupTable, p.sampleRate)
        , mod(p.lookupTable, p.sampleRate)
        , multiFx(props.sampleRate)
    {
        carrier.setType(WavetableGenerator::Type::Sine);
        mod.setType(WavetableGenerator::Type::Sine);
        initValues();
    }

    void sampleOn(float* buf, float envAmpVal, int sampleCounter, int totalSamples) override
    {
        float t = (float)sampleCounter / totalSamples;

        float pitchEnvVal = envPitch.next(t);

        // base + pitch envelope
        float freq = bodyHz * toneRatio * powf(2.0f, pitchEnvVal * 2.0f);
        float modFreq = bodyHz * snapRatio;

        float modSignal = mod.sample(&sampleIndexMod, modFreq);

        // float car = carrier.sample(&sampleIndexCar, freq + modSignal * modIndex);
        float car = carrier.sample(&sampleIndexCar, freq + modSignal * modIndex * freq);

        // float noise = props.lookupTable->getNoise() * (1.0f - t);
        // float mixed = (car * (1.0f - noiseMix.pct())) + (noise * noiseMix.pct());
        float noise = props.lookupTable->getNoise() * (1.0f - t);
        // noise = filter.process(noise); // apply noise tone
        float out = (car * (1.0f - noiseMix.pct())) + (noise * noiseMix.pct());

        out = filter.process(out);
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out * envAmpVal * velocity;
    }

    void sampleOff(float* buf) override
    {
        float out = buf[track];
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        sampleIndexCar = 0.0f;
        sampleIndexMod = 0.0f;
        baseFreq = bodyHz * powf(2.0f, (note - 60) / 12.0f);
    }
};
