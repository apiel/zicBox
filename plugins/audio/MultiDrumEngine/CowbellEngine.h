#pragma once

#include "plugins/audio/utils/valMultiFx.h"
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/effects/applyDrive.h"
#include <cmath>

class CowbellEngine : public DrumEngine {
protected:
    MultiFx multiFx;

    float velocity = 1.0f;
    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float envelope = 0.0f;
    float bandPassState1 = 0.0f;
    float bandPassState2 = 0.0f;

    // --- Parameters (10 total) ---
    Val& frequency = val(540.0f, "FREQ", { .label = "Pitch", .min = 300.0, .max = 800.0, .unit = "Hz" });
    Val& decay = val(30, "DECAY", { .label = "Decay", .unit = "%" });
    
    // The "Ratio" creates the metallic clash
    // Classic 808 ratio is roughly 1.0 : 1.48
    Val& dissonance = val(48, "DISSONANCE", { .label = "Dissonance", .unit = "%" });
    
    // Material Morph (Square to Pulse width)
    Val& material = val(50, "MATERIAL", { .label = "Material", .unit = "%" });
    
    // Filtering
    Val& filterCutoff = val(60, "CUTOFF", { .label = "Filter", .unit = "%" });
    Val& resonance = val(40, "RING", { .label = "Resonance", .unit = "%" });
    
    // Character
    Val& drive = val(15, "DRIVE", { .label = "Heat", .unit = "%" });
    Val& impact = val(20, "IMPACT", { .label = "Stick Hit", .unit = "%" });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX type", .type = VALUE_STRING, .max = MultiFx::FX_COUNT - 1 }, valMultiFx(multiFx));
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX edit", .unit = "%" });

public:
    CowbellEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Cowbell")
        , multiFx(props.sampleRate)
    {
        initValues();
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        envelope = 1.0f;
        // We don't necessarily reset phases to keep the metal "shimmer" natural
    }

    void sampleOn(float* buffer, float envelopeAmplitude, int sampleCounter, int totalSamples) override
    {
        // 1. Envelope
        float decayTime = 0.02f + (decay.pct() * 0.5f);
        envelope *= expf(-1.0f / (props.sampleRate * decayTime));

        // 2. Dual Oscillators
        float f1 = frequency.get();
        float f2 = f1 * (1.0f + dissonance.pct() * 1.0f); // Ratio scaling
        
        phase1 += f1 / props.sampleRate;
        phase2 += f2 / props.sampleRate;
        if (phase1 > 1.0f) phase1 -= 1.0f;
        if (phase2 > 1.0f) phase2 -= 1.0f;

        // Material shapes the "Pulse Width"
        float pw = 0.1f + (material.pct() * 0.4f);
        float sig1 = phase1 > pw ? 1.0f : -1.0f;
        float sig2 = phase2 > pw ? 1.0f : -1.0f;
        
        float mix = (sig1 + sig2) * 0.5f;

        // 3. Band Pass Filter (Essential for Cowbell "Clank")
        // We use a simple 2-pole resonant filter structure
        float cutoff = 500.0f + (filterCutoff.pct() * 4000.0f);
        float q = 0.1f + (resonance.pct() * 0.9f);
        
        float f = 2.0f * sinf(M_PI * cutoff / props.sampleRate);
        bandPassState1 = bandPassState1 + f * (mix - bandPassState2 - q * bandPassState1);
        bandPassState2 = bandPassState2 + f * bandPassState1;
        
        float filtered = bandPassState1;

        // 4. Impact Transient (Stick hitting the metal)
        if (sampleCounter < 200) {
            filtered += (props.lookupTable->getNoise() * 2.0f - 1.0f) * impact.pct() * 2.0f;
        }

        // 5. Final Gain & Processing
        float out = filtered * envelope;

        if (drive.pct() > 0.0f) {
            out = applyDrive(out, drive.pct() * 4.0f);
        }

        out = multiFx.apply(out, fxAmount.pct());

        buffer[track] = out * envelopeAmplitude * velocity;
    }

    void sampleOff(float* buf) override
    {
        float out = buf[track];
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }
};