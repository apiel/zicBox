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
    
    // LFO Randomization state
    float rndLfoMod = 1.0f;
    float rndLfoRate = 1.0f;

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
    // --- Parameters ---

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

    Val& wave = val(0.0f, "WAVE", { .label = "Saw > Square", .unit = "%" });

    Val& dist = val(60.0f, "DIST", { .label = "Dist", .unit = "%" });

    Val& lfoRate = val(20.0f, "LFO_RATE", { .label = "LFO Speed", .step = 0.1f, .unit = "%" });

    Val& lfoDepth = val(30.0f, "LFO_MOD", { .label = "LFO Depth", .unit = "%" });

    // New Parameter: Randomizes LFO behavior per cycle
    Val& drift = val(30.0f, "DRIFT", { .label = "LFO Drift", .unit = "%" });

    Val& cutoff = val(40.0f, "CUTOFF", { .label = "Cutoff", .unit = "%" });

    Val& resonance = val(80.0f, "RES", { .label = "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        float res = 0.98f * (1.0f - std::pow(1.0f - p.val.pct(), 3.0f));
        filter.setResonance(res);
    });

    Val& envMod = val(50.0f, "ENV_MOD", { .label = "Env Mod", .unit = "%" });

    Val& reverb = val(30.0f, "REVERB", { .label = "Reverb", .unit = "%" });

    Val& delay = val(60.0f, "DELAY", { .label = "Delay", .type = VALUE_CENTERED, .min = -100.0f, .unit = "%" });

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

        // 1. Oscillator Logic
        phase = fmodf(phase + (baseFreq / props.sampleRate), 1.0f);

        // 2. Randomized LFO Logic
        // Calculate base frequency and apply randomized drift
        float baseLfoFreq = (lfoRate.pct() * 80.0f) + 0.1f;
        float currentLfoFreq = baseLfoFreq * (1.0f + (rndLfoRate - 0.5f) * drift.pct() * 2.0f);
        
        float prevPhase = lfoPhase;
        lfoPhase = fmodf(lfoPhase + (currentLfoFreq / props.sampleRate), 1.0f);

        // If LFO wraps around, generate new random factors for the next cycle
        if (lfoPhase < prevPhase) {
            rndLfoMod = (float)rand() / (float)RAND_MAX;  // 0.0 to 1.0
            rndLfoRate = (float)rand() / (float)RAND_MAX; // 0.0 to 1.0
        }

        // Triangle LFO with depth randomization
        float lfoTri = (lfoPhase < 0.5f) ? (lfoPhase * 4.0f - 1.0f) : (3.0f - lfoPhase * 4.0f);
        float currentDepth = lfoDepth.pct() * (1.0f + (rndLfoMod - 0.5f) * drift.pct());
        float lfoMod = lfoTri * currentDepth;

        // 3. Waveform & Distortion
        float saw = (phase * 2.0f) - 1.0f;
        float square = (phase > 0.5f) ? 0.6f : -0.6f;
        float sig = saw * (1.0f - wave.pct()) + square * wave.pct();

        // Triple-stage waveshaping for that "Electric Guitar" scream
        sig = applyWaveshape2(sig, dist.pct());
        sig = applyWaveshape3(sig, dist.pct());
        sig = applyWaveshape4(sig, dist.pct());

        // 4. Filter Processing
        float env = (envAmpVal * envMod.pct());
        // LFO mod is added to cutoff. Drift ensures it stays unpredictable.
        float cutVal = 0.85f * (cutoff.pct() + env + lfoMod) + 0.1f;

        filter.setCutoff(fmaxf(0.01f, fminf(0.99f, cutVal)));
        filter.setSampleData(sig, 0);
        filter.setSampleData(filter.lp[0], 1);
        sig = filter.lp[1];

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
        phase = 0.0f; // keep this <--- !!
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