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
    
    // Vibrato state
    float vibratoPhase = 0.0f;

    // --- Parameters (10 total) ---
    Val& openClose = val(10, "MORPH", { .label = "Open/Close", .unit = "%" });

    // Tonal Core
    Val& metalFreq = val(400.0f, "METAL", { .label = "Metal Freq", .min = 100.0, .max = 2000.0, .unit = "Hz" });
    Val& ringMod = val(50, "RING_MOD", { .label = "Inharmonic", .unit = "%" });

    // The Craziness
    Val& grit = val(0, "GRIT", { .label = "Digital Grit", .unit = "%" }); 
    Val& feedback = val(0, "FEEDBACK", { .label = "Resonance", .unit = "%" }); 
    
    // NEW: Vibrato (The 10th Parameter!)
    Val& vibrato = val(0, "VIBRATO", { .label = "Swirl", .unit = "%" });

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
        // We don't reset phases to keep the "shimmer" moving between hits
    }

    void sampleOn(float* buffer, float envelopeAmplitude, int sampleCounter, int totalSamples) override
    {
        // 1. Morphing Envelope
        float morphFactor = openClose.pct();
        float decayTime = 0.005f + (morphFactor * 0.5f);
        decayState *= expf(-1.0f / (props.sampleRate * decayTime));

        // 2. Vibrato Logic
        // As you turn up 'Swirl', the speed goes from 2Hz to 20Hz and the depth increases
        float vibSpeed = 2.0f + (vibrato.pct() * 18.0f);
        vibratoPhase += vibSpeed / props.sampleRate;
        if (vibratoPhase > 1.0f) vibratoPhase -= 1.0f;
        
        // Sine vibrato [-1.0, 1.0]
        float vibMod = sinf(vibratoPhase * 2.0f * M_PI) * vibrato.pct() * 0.2f;

        // 3. Metallic Oscillator Bank
        float ratios[6] = { 1.0f, 1.523f, 1.965f, 2.381f, 3.121f, 4.451f };
        float mix = 0.0f;
        float base = metalFreq.get() * (1.0f + vibMod); // Apply vibrato to the base frequency
        float inharmonicity = ringMod.pct() * 500.0f;

        for (int i = 0; i < 6; ++i) {
            float freq = (base * ratios[i]) + (i * inharmonicity);
            oscillatorPhases[i] += freq / props.sampleRate;
            if (oscillatorPhases[i] > 1.0f) oscillatorPhases[i] -= 1.0f;

            float sig = oscillatorPhases[i] > 0.5f ? 1.0f : -1.0f;

            if (i % 2 == 0) mix += sig;
            else mix *= sig;
        }

        // 4. Digital Grit
        float finalSource = mix;
        if (grit.pct() > 0.0f) {
            int holdSamples = (int)(grit.pct() * 40.0f);
            if (++sampleHoldCounter >= holdSamples) {
                sampleHoldState = finalSource;
                sampleHoldCounter = 0;
            }
            finalSource = sampleHoldState;
        }

        // 5. Filters
        static float hpState = 0.0f;
        float hpCutoff = 0.1f + highPass.pct() * 0.85f;
        hpState += hpCutoff * (finalSource - hpState);
        float filtered = (finalSource - hpState);

        if (feedback.pct() > 0.0f) {
            filtered += lowPassState * feedback.pct() * 0.95f;
        }

        // 6. Final Processing
        float out = filtered * decayState;

        if (drive.pct() > 0.0f) {
            out = applyDrive(out, drive.pct() * 4.0f, props.lookupTable);
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