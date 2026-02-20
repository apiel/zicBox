#pragma once

#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/effects/applyDrive.h"
#include <cmath>

class SnareEngine : public DrumEngine {
protected:
    float velocity = 1.0f;
    float tonalPhase = 0.0f;
    float bodyEnvelope = 0.0f;
    float noiseEnvelope = 0.0f;
    float lowPassState = 0.0f;

    // --- Parameters (10 total) ---
    // Tonal Core
    Val& baseFrequency = val(180.0f, "FREQ", { .label = "Body Freq", .min = 100.0, .max = 400.0, .unit = "Hz" });
    Val& bodyDecay = val(30, "BODY_DECAY", { .label = "Body", .unit = "%" });
    
    // Snappy (Noise)
    Val& snappyLevel = val(50, "SNAPPY", { .label = "Snappy", .unit = "%" });
    Val& snappyDecay = val(40, "SNAPPY_DECAY", { .label = "Snap Tail", .unit = "%" });
    Val& snapTone = val(50, "SNAP_TONE", { .label = "Snap Tone", .unit = "%" }); // High Pass Filter
    
    // Character
    Val& pitchDrop = val(20, "PITCH_DROP", { .label = "Punch", .unit = "%" });
    Val& impact = val(30, "IMPACT", { .label = "Impact", .unit = "%" }); // Transient click
    
    // Saturation & Master
    Val& drive = val(15, "DRIVE", { .label = "Drive", .unit = "%" });
    Val& tightness = val(50, "TIGHTNESS", { .label = "Tightness", .unit = "%" }); // Gate-like shortening
    Val& tone = val(100, "TONE", { .label = "Tone", .unit = "%" });

public:
    SnareEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Snare")
    {
        initValues();
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        tonalPhase = 0.0f;
        bodyEnvelope = 1.0f;
        noiseEnvelope = 1.0f;
        lowPassState = 0.0f;
    }

    void sampleOn(float* buffer, float envelopeAmplitude, int sampleCounter, int totalSamples) override
    {
        // 1. Envelopes
        // Body decay (Sine part)
        float bodyTime = 0.01f + (bodyDecay.pct() * 0.2f);
        bodyEnvelope *= expf(-1.0f / (props.sampleRate * bodyTime));

        // Noise decay (Wires part)
        float noiseTime = 0.02f + (snappyDecay.pct() * 0.4f);
        noiseEnvelope *= expf(-1.0f / (props.sampleRate * noiseTime));

        // 2. Tonal Part (The "Hit")
        // Subtle pitch drop for a more realistic drum skin sound
        float pitchEnv = bodyEnvelope * pitchDrop.pct() * 100.0f;
        tonalPhase += (baseFrequency.get() + pitchEnv) / props.sampleRate;
        if (tonalPhase > 1.0f) tonalPhase -= 1.0f;
        float tonalPart = sinf(tonalPhase * 2.0f * M_PI) * bodyEnvelope;

        // 3. Noise Part (The "Wires")
        float rawNoise = (props.lookupTable->getNoise() * 2.0f - 1.0f);
        
        // Basic High Pass for noise tone
        static float noiseHpState = 0.0f;
        float hpCutoff = 0.01f + (snapTone.pct() * 0.5f);
        noiseHpState += hpCutoff * (rawNoise - noiseHpState);
        float snappyPart = (rawNoise - noiseHpState) * noiseEnvelope * snappyLevel.pct();

        // 4. Impact Transient
        float impactPart = 0.0f;
        if (sampleCounter < 150) {
            impactPart = rawNoise * impact.pct() * 2.0f;
        }

        // 5. Mix and Process
        float mixedSignal = (tonalPart * 0.6f) + snappyPart + impactPart;

        // Apply Drive
        if (drive.pct() > 0.0f) {
            mixedSignal = applyDrive(mixedSignal, drive.pct() * 3.0f);
        }

        // Master Tone (LPF)
        float masterCutoff = 0.1f + tone.pct() * 0.9f;
        lowPassState += masterCutoff * (mixedSignal - lowPassState);
        mixedSignal = lowPassState;

        // 6. Tightness (Gate/Shortener)
        // Shortens both envelopes towards the end
        float tightFactor = powf(envelopeAmplitude, 1.0f + tightness.pct() * 3.0f);

        buffer[track] = mixedSignal * tightFactor * velocity;
    }

    void sampleOff(float* buffer) override {}
};