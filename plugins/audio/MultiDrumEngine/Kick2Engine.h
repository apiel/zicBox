#pragma once

#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include <cmath>

class Kick2Engine : public DrumEngine {
protected:
    float velocity = 1.0f;
    float oscillatorPhase = 0.0f;

    // Internal state for transient tracking
    float pitchEnvelopeState = 0.0f;
    float clickEnvelopeState = 0.0f;
    float lowPassState = 0.0f;

    // --- Parameters (10 total) ---
    Val& baseFrequency = val(45.0f, "FREQ", { .label = "Sub Freq", .min = 30.0, .max = 80.0, .unit = "Hz" });
    Val& pitchOffset = val(0.0f, "PITCH", { .label = "Pitch", .type = VALUE_CENTERED, .min = -12, .max = 12 });

    // Pitch Envelope (The "Knock")
    Val& sweepDepth = val(70, "SWEEP_DEPTH", { .label = "Sweep", .unit = "%" });
    Val& sweepSpeed = val(30, "SWEEP_SPEED", { .label = "Punch", .unit = "%" });

    // Character & Shaping
    Val& symmetry = val(0, "SYMMETRY", { .label = "Symmetry", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& clickLevel = val(20, "CLICK", { .label = "Click", .unit = "%" });
    Val& airNoise = val(5, "NOISE", { .label = "Air", .unit = "%" });

    // Saturation & Fatness
    Val& driveAmount = val(30, "DRIVE", { .label = "Drive", .unit = "%" });
    Val& compressionAmount = val(20, "COMP", { .label = "Glue", .unit = "%" });
    Val& toneCutoff = val(50, "TONE", { .label = "Tone", .unit = "%" });

public:
    Kick2Engine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Kick2")
    {
        initValues();
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        oscillatorPhase = 0.0f;
        pitchEnvelopeState = 1.0f;
        clickEnvelopeState = 1.0f;
        lowPassState = 0.0f;
    }

    void sampleOn(float* buffer, float envelopeAmplitude, int sampleCounter, int totalSamples) override
    {
        // 1. Pitch Envelope (Fast exponential decay for the initial "knock")
        // We use a very short time constant for that punchy start
        float sweepDecayTime = 0.005f + (1.0f - sweepSpeed.pct()) * 0.05f;
        pitchEnvelopeState *= expf(-1.0f / (props.sampleRate * sweepDecayTime));

        // 2. Frequency Calculation
        float rootFreq = baseFrequency.get() * powf(2.0f, (pitchOffset.get()) / 12.0f);
        // Frequency starts high (based on Sweep Depth) and drops to the root frequency
        float currentFrequency = rootFreq + (sweepDepth.pct() * 600.0f * pitchEnvelopeState);

        // 3. Oscillator with Symmetry (Waveshaping)
        oscillatorPhase += currentFrequency / props.sampleRate;
        if (oscillatorPhase > 1.0f) oscillatorPhase -= 1.0f;

        float rawSine = sinf(2.0f * M_PI * oscillatorPhase);

        // Apply Symmetry: This pushes the sine wave towards a square or tilts the peaks
        // It creates that 'hollow' or 'heavy' techno timbre
        float shapeAmount = (symmetry.pct() * 2.0f - 1.0f) * 0.9f;
        float shapedSine = (rawSine + shapeAmount * (rawSine * rawSine * rawSine)) / (1.0f + shapeAmount);

        // 4. Transient (Click) and Air (Noise)
        float noiseSample = (props.lookupTable->getNoise() * 2.0f - 1.0f);
        float clickPart = noiseSample * clickEnvelopeState * clickLevel.pct();

        // Rapid decay for the click
        clickEnvelopeState *= expf(-1.0f / (props.sampleRate * 0.002f));

        // 5. Signal Summing
        float finalOutput = shapedSine + clickPart;

        // Add subtle high-end "air" noise linked to the main amplitude envelope
        finalOutput += noiseSample * airNoise.pct() * 0.05f * envelopeAmplitude;

        // 6. Non-linear Processing
        // Drive (Saturation)
        if (driveAmount.get() > 0.0f) {
            finalOutput = applyDrive(finalOutput, driveAmount.pct() * 2.5f, props.lookupTable);
        }

        // Tone Control (Low Pass Filter to smooth out the drive harmonics)
        float filterCoefficient = 0.05f + toneCutoff.pct() * 0.7f;
        lowPassState += filterCoefficient * (finalOutput - lowPassState);
        finalOutput = lowPassState;

        // Glue (Compression) to bring the tail and the punch together
        if (compressionAmount.get() > 0.0f) {
            finalOutput = applyCompression(finalOutput, compressionAmount.pct());
        }

        buffer[track] = finalOutput * envelopeAmplitude * velocity;
    }
};