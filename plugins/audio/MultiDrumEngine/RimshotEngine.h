#pragma once

#include "audio/MultiFx.h"
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/effects/applyDrive.h"
#include <cmath>

class RimshotEngine : public DrumEngine {
protected:
    MultiFx multiFx;

    float velocity = 1.0f;
    float phaseBody = 0.0f;
    float phaseMod = 0.0f;
    float envBody = 0.0f;
    float lowPassState = 0.0f;

    // --- Parameters (10 total) ---
    Val& frequency = val(450.0f, "FREQ", { .label = "Pitch", .min = 200.0, .max = 900.0, .unit = "Hz" });
    Val& decay = val(15, "DECAY", { .label = "Decay", .unit = "%" });
    
    // Tonal Shaping
    Val& timber = val(30, "TIMBER", { .label = "Body Morph", .unit = "%" }); // Wood to Metal
    Val& ring = val(20, "RING", { .label = "Ring Mod", .unit = "%" }); // FM intensity
    
    // The "Snap"
    Val& click = val(50, "CLICK", { .label = "Click", .unit = "%" });
    Val& resonance = val(40, "RESONANCE", { .label = "Resonance", .unit = "%" });
    
    // Processing
    Val& drive = val(10, "DRIVE", { .label = "Drive", .unit = "%" });
    Val& tone = val(80, "TONE", { .label = "Tone", .unit = "%" });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX edit", .unit = "%" });

public:
    RimshotEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Rimshot")
        , multiFx(props.sampleRate, props.lookupTable)
    {
        initValues();
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        phaseBody = 0.0f;
        phaseMod = 0.0f;
        envBody = 1.0f;
        lowPassState = 0.0f;
    }

    void sampleOn(float* buffer, float envelopeAmplitude, int sampleCounter, int totalSamples) override
    {
        // 1. Envelopes
        // Rimshots must be very fast.
        float decayTime = 0.005f + (decay.pct() * 0.15f);
        envBody *= expf(-1.0f / (props.sampleRate * decayTime));

        // 2. Dual Oscillator Logic (Phase Modulation)
        float baseFreq = frequency.get();
        
        // Modulator: Provides the metallic "ping"
        float modFreq = baseFreq * (1.0f + timber.pct() * 4.0f);
        phaseMod += modFreq / props.sampleRate;
        if (phaseMod > 1.0f) phaseMod -= 1.0f;
        float modulator = sinf(phaseMod * 2.0f * M_PI) * ring.pct() * 2.0f;

        // Carrier: The main body
        phaseBody += (baseFreq / props.sampleRate);
        if (phaseBody > 1.0f) phaseBody -= 1.0f;
        
        // Apply FM + a bit of "Timber" squareness
        float carrier = sinf((phaseBody + modulator) * 2.0f * M_PI);
        float woodSquare = carrier > 0.0f ? 1.0f : -1.0f;
        float mixedBody = (carrier * (1.0f - timber.pct())) + (woodSquare * timber.pct() * 0.5f);

        // 3. The Initial Click (Transient)
        float noiseClick = 0.0f;
        if (sampleCounter < 120) {
            noiseClick = (props.lookupTable->getNoise() * 2.0f - 1.0f) * click.pct();
        }

        // 4. Mix and Filter
        float out = (mixedBody * envBody) + noiseClick;

        // Resonant "Ping"
        static float resState = 0.0f;
        float resFreq = 0.2f + (resonance.pct() * 0.5f);
        resState += resFreq * (out - resState);
        out += resState * resonance.pct();

        // 5. Final Shaping
        if (drive.pct() > 0.0f) {
            out = applyDrive(out, drive.pct() * 3.0f);
        }

        float cutoff = 0.1f + tone.pct() * 0.9f;
        lowPassState += cutoff * (out - lowPassState);
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