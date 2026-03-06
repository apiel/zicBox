#pragma once

#include "audio/AsrEnvelop.h"
#include "audio/EnvelopDrumAmp.h"
#include "audio/effects/applyReverb.h"
#include "audio/effects/applyWaveshape.h"
#include "audio/filterArray.h"
#include "helpers/math.h"
#include "helpers/midiNote.h"
#include "plugins/audio/MultiEngine.h"
#include "plugins/audio/audioPlugin.h"
#include "plugins/audio/mapping.h"

class MentalEngine : public MultiEngine {
protected:
    EffectFilterArray<2> filter;

    float velocity = 1.0f;
    float phase = 0.0f;
    float lfoPhase = 0.0f;

    float* fxBuffer = nullptr;
    int bufferIndex = 0;
    float* fxBuffer2 = nullptr;
    int bufferIndex2 = 0;

    float baseFreq = 220.0f;
    uint8_t baseFreqNote = 60;
    void setBaseFreq(int8_t pitchSemitones, uint8_t note = 0)
    {
        if (note == 0)
            note = baseFreqNote;

        baseFreqNote = note;
        baseFreq = getMidiNoteFrequency(baseFreqNote + pitchSemitones);
    }

public:
    // --- 10 Parameters ---

    float attackStep = 0.0f;
    float releaseStep = 0.0f;
    AsrEnvelop envelopAmp = AsrEnvelop(&attackStep, &releaseStep);

    Val& release = val(300.0f, "RELEASE", { .label = "Release", .min = 10.0, .max = 3000.0, .step = 10.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        releaseStep = 1.0f / (p.val.get() * 0.001f * props.sampleRate);
    });

    Val& body = val(0.0f, "BODY", { .label = "Body", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(p.val.get() - 12);
    });

    // 1. Waveform: Dirty Saw to Square
    Val& wave = val(0.0f, "WAVE", { .label = "Saw > Square", .unit = "%" });

    Val& dist = val(60.0f, "DIST", { .label = "Dist", .unit = "%" });

    // 3. LFO Rate: Speed of the bubbling filter modulation
    Val& lfoRate = val(20.0f, "LFO_RATE", { .label = "LFO Speed", .step = 0.1f, .unit = "%" });

    // 4. LFO Depth: How much the LFO "stretches" the filter
    Val& lfoDepth = val(30.0f, "LFO_MOD", { .label = "LFO Depth", .unit = "%" });

    // 5. Global Cutoff
    Val& cutoff = val(40.0f, "CUTOFF", { .label = "Cutoff", .unit = "%" });

    // 6. Resonance: The 303-style scream
    Val& resonance = val(80.0f, "RES", { .label = "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        // Exponential resonance mapping for better "squeal"
        float res = 0.98f * (1.0f - std::pow(1.0f - p.val.pct(), 3.0f));
        filter.setResonance(res);
    });

    // 7. Env Mod: Filter Envelope depth
    Val& envMod = val(50.0f, "ENV_MOD", { .label = "Env Mod", .unit = "%" });

    // 8. Accent: Velocity-to-Cutoff sensitivity
    Val& accent = val(40.0f, "ACCENT", { .label = "Accent", .unit = "%" });

    // 9. Reverb
    Val& reverb = val(30.0f, "REVERB", { .label = "Reverb", .unit = "%" });

    // 10. Delay
    Val& delay = val(60.0f, "DELAY", { .label = "Delay", .type = VALUE_CENTERED, .min = -100.0f, .unit = "%" });

    // Let's put attack at the end because it's not important to be able to modify it
    Val& attack = val(50.0f, "ATTACK", { .label = "Attack", .min = 10.0, .max = 3000.0, .step = 10.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        attackStep = 1.0f / (p.val.get() * 0.001f * props.sampleRate);
    });

    MentalEngine(AudioPlugin::Props& p, AudioPlugin::Config& c, float* fxBuffer, float* fxBuffer2)
        : MultiEngine(p, c, "Mental")
        , fxBuffer(fxBuffer)
        , fxBuffer2(fxBuffer2)
    {
        initValues();
    }

    void sample(float* buf) override
    {
        float envAmpVal = envelopAmp.next();
        if (envAmpVal == 0.0f) {
            float out = applyReverb(buf[track], reverb.pct(), fxBuffer2, bufferIndex2);
            buf[track] = fxDelay(out);
            return;
        }

        // 1. Oscillator & LFO
        phase = fmodf(phase + (baseFreq / props.sampleRate), 1.0f);

        // Faster LFO for "Mental" bubbling
        float lfoFreq = (lfoRate.pct() * 20.0f) + 0.1f;
        lfoPhase = fmodf(lfoPhase + (lfoFreq / props.sampleRate), 1.0f);
        float lfoSig = sinf(lfoPhase * 2.0f * M_PI); // Sine is smoother for "liquid" acid

        float saw = (phase * 2.0f) - 1.0f;
        float square = (phase > 0.5f) ? 0.5f : -0.5f;
        float sig = (saw * (1.0f - wave.pct()) + square * wave.pct());

        // // 2. The "Electric" Drive Stage (Pre-Filter Saturation)
        // // We boost the signal significantly based on DRIVE and ACCENT
        // float totalGain = 1.0f + (drive.pct() * 12.0f) + (velocity * accent.pct() * 5.0f);
        // sig *= totalGain;

        // Soft saturation stage (Soft-clipper/Overdrive)
        // f(x) = x / (1 + abs(x)) -> Classic analog-style saturation
        sig = sig / (1.0f + std::abs(sig));

        // 3. Filter Processing
        // Accent now adds a "punch" to the cutoff and envMod
        float accentBoost = (velocity * accent.pct());
        float mod = (envAmpVal * (envMod.pct() + accentBoost * 0.2f));
        float cutVal = 0.85f * (cutoff.pct() + mod + (lfoSig * lfoDepth.pct() * 0.5f)) + 0.05f;

        filter.setCutoff(fmaxf(0.01f, fminf(0.98f, cutVal)));
        filter.setSampleData(sig, 0);
        filter.setSampleData(filter.lp[0], 1);
        sig = filter.lp[1];

        // 4. Post-Filter "Guitar" Scream (Hard Stage)
        // This clips the resonance "peaks" to create the electric guitar grit
        sig = applyWaveshape2(sig, dist.pct());
        sig = applyWaveshape3(sig, dist.pct());
        sig = applyWaveshape4(sig, dist.pct());

        sig *= envAmpVal * velocity;

        // 5. Global FX
        sig = applyReverb(sig, reverb.pct(), fxBuffer2, bufferIndex2);
        sig = fxDelay(sig);

        buf[track] = sig;
    }

    uint8_t playingNote = 0;
    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        playingNote = note;
        envelopAmp.attack();
        velocity = _velocity;
        setBaseFreq(body.get() - 12, note);
        phase = 0.0f; // keep this <---
    }

    void noteOff(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        if (note == playingNote) {
            envelopAmp.release();
        }
    }

    float fxDelay(float input)
    {
        if (std::abs(delay.get()) < 0.1f) return input;
        if (delay.get() > 0.0f) return applyDelay(input, delay.get() * 0.01f, fxBuffer, bufferIndex);
        return applyDelay2(input, delay.get() * -0.01f, fxBuffer, bufferIndex);
    }
};