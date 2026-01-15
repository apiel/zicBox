#pragma once

#include "audio/MultiFx.h"
#include "helpers/math.h"
#include "plugins/audio/MultiEngine/Engine.h"

class PhaseDistEngine : public Engine {
protected:
    MultiFx multiFx;
    MultiFx multiFx2;

    float velocity = 1.0f;
    float phase = 0.0f;

    // Internal helper for Phase Distortion math
    float pdOscillator(float phase, float distortion)
    {
        float warpedPhase = 0.0f;

        // The "Kink" logic: split the phase into two speeds
        if (phase < distortion) {
            warpedPhase = (phase * 0.5f) / distortion;
        } else {
            warpedPhase = 0.5f + ((phase - distortion) * 0.5f) / (1.0f - distortion);
        }

        return sinf(warpedPhase * 2.0f * M_PI);
    }

public:
    // --- 10 Parameters ---
    Val& body = val(0.0f, "BODY", { .label = "Body", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(p.val.get());
    });

    Val& distortion = val(0.0f, "DIST", { .label = "PD Amount", .unit = "%" });

    Val& detune = val(0.0f, "DETUNE", { .label = "Sub Detune", .unit = "%" });

    Val& vibratoRate = val(0.0f, "VIB_RATE", { .label = "Vibrato", .min = 0.0, .max = 10.0, .unit = "Hz" });

    // Envelope modulation of the Distortion (Simulates filter env)
    Val& envMod = val(50.0f, "ENV_MOD", { .label = "Env > PD", .unit = "%" });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX1 Type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX1 Edit", .unit = "%" });

    Val& fx2Type = val(0, "FX2_TYPE", { .label = "FX2 Type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx2.setFxType);
    Val& fx2Amount = val(0, "FX2_AMOUNT", { .label = "FX2 Edit", .unit = "%" });

    PhaseDistEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "PhasDist")
        , multiFx(props.sampleRate, props.lookupTable)
        , multiFx2(props.sampleRate, props.lookupTable)
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

        // Calculate PD amount based on Knob + Envelope
        float currentDist = distortion.pct() + (envMod.pct() * envAmpVal);
        if (currentDist > 0.99f) currentDist = 0.99f; // Stability clamp

        // Advance Main Phase
        phase += baseFreq / props.sampleRate;
        if (phase > 1.0f) phase -= 1.0f;

        // Generate primary PD sound
        float out = pdOscillator(phase, 1.0f - currentDist);

        // Add a secondary slightly detuned "Sub" oscillator for thickness
        if (detune.pct() > 0.0f) {
            static float subPhase = 0.0f;
            subPhase += (baseFreq * (1.0f - detune.pct() * 0.02f)) / props.sampleRate;
            if (subPhase > 1.0f) subPhase -= 1.0f;
            out = (out + pdOscillator(subPhase, 1.0f - currentDist)) * 0.6f;
        }

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
        phase = 0.0f;
    }
};