#pragma once

#include "audio/MultiFx.h"
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/effects/applyDrive.h"
#include <cmath>

class FreakHatEngine : public DrumEngine {
protected:
    MultiFx multiFx;

    float velocity = 1.0f;
    float oscillatorPhases[6] = { 0.0f };
    float decayState = 0.0f;
    float lowPassState = 0.0f;
    float sampleHoldState = 0.0f;
    int sampleHoldCounter = 0;

    // --- Parameters (10 total) ---
    Val& openClose = val(10, "MORPH", { .label = "Open/Close", .unit = "%" });

    // Tonal Core
    Val& metalFreq = val(400.0f, "METAL", { .label = "Metal Freq", .min = 100.0, .max = 2000.0, .unit = "Hz" });
    Val& ringMod = val(50, "RING_MOD", { .label = "Inharmonic", .unit = "%" });

    // The Craziness
    Val& grit = val(0, "GRIT", { .label = "Digital Grit", .unit = "%" }); 
    Val& feedback = val(0, "FEEDBACK", { .label = "Resonance", .unit = "%" }); 
    
    // NEW: FM Madness (Replacing Vibrato)
    // This uses the internal oscillators to cross-modulate each other
    Val& fmMadness = val(0, "FM_MADNESS", { .label = "FM Scream", .unit = "%" });

    // Shaping
    Val& highPass = val(70, "HPF", { .label = "Air", .unit = "%" });
    Val& drive = val(10, "DRIVE", { .label = "Heat", .unit = "%" });

    // Standard Drum FX
    Val& fxType = val(0, "FX_TYPE", { .label = "FX type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX edit", .unit = "%" });

public:
    FreakHatEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "FreakHat")
        , multiFx(props.sampleRate, props.lookupTable)
    {
        initValues();
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        decayState = 1.0f;
    }

    void sampleOn(float* buffer, float envelopeAmplitude, int sampleCounter, int totalSamples) override
    {
        // 1. Morphing Envelope
        float morphFactor = openClose.pct();
        float decayTime = 0.005f + (morphFactor * 0.5f);
        decayState *= expf(-1.0f / (props.sampleRate * decayTime));

        // 2. Metallic Oscillator Bank with FM
        float ratios[6] = { 1.0f, 1.523f, 1.965f, 2.381f, 3.121f, 4.451f };
        float mix = 0.0f;
        float base = metalFreq.get();
        float inharmonicity = ringMod.pct() * 500.0f;
        
        float fmIntensity = (1.0f - fmMadness.pct()) * 2.0f;
        float lastSig = 0.0f; // Used for cross-modulation

        for (int i = 0; i < 6; ++i) {
            float freq = (base * ratios[i]) + (i * inharmonicity);
            
            // The FM Magic: Each oscillator's phase is pushed by the previous oscillator
            float fmOffset = (lastSig * fmIntensity);
            oscillatorPhases[i] += (freq / props.sampleRate) + (fmOffset * 0.01f);
            
            if (oscillatorPhases[i] > 1.0f) oscillatorPhases[i] -= 1.0f;

            float sig = oscillatorPhases[i] > 0.5f ? 1.0f : -1.0f;
            lastSig = sig; // Feed this into the next oscillator in the loop

            if (i % 2 == 0) mix += sig;
            else mix *= sig;
        }

        // 3. Digital Grit (Downsampling)
        float finalSource = mix;
        if (grit.pct() > 0.0f) {
            int holdSamples = (int)(grit.pct() * 40.0f);
            if (++sampleHoldCounter >= holdSamples) {
                sampleHoldState = finalSource;
                sampleHoldCounter = 0;
            }
            finalSource = sampleHoldState;
        }

        // 4. Filters & Feedback
        static float hpState = 0.0f;
        float hpCutoff = 0.1f + highPass.pct() * 0.85f;
        hpState += hpCutoff * (finalSource - hpState);
        float filtered = (finalSource - hpState);

        if (feedback.pct() > 0.0f) {
            filtered += lowPassState * feedback.pct() * 0.95f;
        }

        // 5. Final Processing
        float out = filtered * decayState;

        if (drive.pct() > 0.0f) {
            out = applyDrive(out, drive.pct() * 4.0f);
        }

        lowPassState += 0.7f * (out - lowPassState);
        out = lowPassState;
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