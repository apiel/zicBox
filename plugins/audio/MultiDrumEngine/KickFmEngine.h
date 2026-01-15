#pragma once

#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/effects/applyDrive.h"
#include <cmath>

class KickFmEngine : public DrumEngine {
protected:
    float velocity = 1.0f;
    float carrierPhase = 0.0f;
    float modulatorPhase = 0.0f;
    bool isActive = false;

    // Internal state trackers
    float modulationEnvelope = 0.0f;
    float lowPassState = 0.0f;

    // --- Parameters (10 total) ---
    // Tuning
    Val& baseFrequency = val(50.0f, "FREQ", { .label = "Sub Freq", .min = 30.0, .max = 100.0, .unit = "Hz" });
    Val& pitchOffset = val(0.0f, "PITCH", { .label = "Pitch", .type = VALUE_CENTERED, .min = -12, .max = 12 });

    // FM Core
    Val& fmAmount = val(40, "FM_AMOUNT", { .label = "FM Depth", .unit = "%" });
    Val& fmRatio = val(1.0f, "FM_RATIO", { .label = "FM Ratio", .min = 0.5f, .max = 8.0f, .step = 0.1f });
    Val& fmDecay = val(20, "FM_DECAY", { .label = "FM Speed", .unit = "%" });

    // Shaping
    Val& punch = val(50, "PUNCH", { .label = "Punch", .unit = "%" }); // Initial pitch spike
    Val& drive = val(25, "DRIVE", { .label = "Drive", .unit = "%" });
    Val& tone = val(60, "TONE", { .label = "Tone", .unit = "%" });
    
    // Sub/Body
    Val& subWeight = val(50, "WEIGHT", { .label = "Weight", .unit = "%" }); // Sinuosity vs Squareness
    Val& click = val(15, "CLICK", { .label = "Click", .unit = "%" });

public:
    KickFmEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "KickFM")
    {
        initValues();
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        
        carrierPhase = 0.0f;
        modulatorPhase = 0.0f;
        modulationEnvelope = 1.0f;
        lowPassState = 0.0f;
        isActive = true;
    }

    void sampleOn(float* buffer, float envelopeAmplitude, int sampleCounter, int totalSamples) override
    {
        if (!isActive) return;

        // 1. Envelopes
        // Modulation envelope controls how much the FM affects the sound over time
        float fmDecayTime = 0.002f + (fmDecay.pct() * 0.1f);
        modulationEnvelope *= expf(-1.0f / (props.sampleRate * fmDecayTime));

        // 2. Frequency Logic
        float rootFreq = baseFrequency.get() * powf(2.0f, (pitchOffset.get()) / 12.0f);
        
        // Add a "Punch" spike to the frequency (classical pitch sweep)
        float pitchSpike = (punch.pct() * 500.0f * modulationEnvelope);
        float carrierFreq = rootFreq + pitchSpike;
        float modulatorFreq = carrierFreq * fmRatio.get();

        // 3. Modulator Oscillator
        modulatorPhase += modulatorFreq / props.sampleRate;
        if (modulatorPhase > 1.0f) modulatorPhase -= 1.0f;
        float modulatorSignal = sinf(2.0f * M_PI * modulatorPhase);

        // 4. Carrier Oscillator (FM Applied Here)
        // The modulator "pushes" the phase of the carrier
        float modulationIntensity = fmAmount.pct() * 2.0f * modulationEnvelope;
        carrierPhase += (carrierFreq / props.sampleRate) + (modulatorSignal * modulationIntensity * 0.1f);
        if (carrierPhase > 1.0f) carrierPhase -= 1.0f;
        
        float carrierSignal = sinf(2.0f * M_PI * carrierPhase);

        // 5. Weight Shaping (Non-linear blending)
        // Similar to the symmetry in the last engine, this makes the body "heavier"
        if (subWeight.pct() > 0.0f) {
            float weight = subWeight.pct() * 0.5f;
            carrierSignal = (1.0f + weight) * carrierSignal / (1.0f + weight * fabsf(carrierSignal));
        }

        // 6. Click Transient (High frequency pulse)
        float noise = (props.lookupTable->getNoise() * 2.0f - 1.0f);
        float clickImpulse = (sampleCounter < 200) ? noise * click.pct() * 2.0f : 0.0f;

        // 7. Processing & Output
        float mixedSignal = carrierSignal + clickImpulse;

        // Apply Drive
        if (drive.pct() > 0.0f) {
            mixedSignal = applyDrive(mixedSignal, drive.pct() * 3.0f, props.lookupTable);
        }

        // Tone (Low Pass)
        float filterCut = 0.05f + tone.pct() * 0.8f;
        lowPassState += filterCut * (mixedSignal - lowPassState);
        mixedSignal = lowPassState;

        buffer[track] = mixedSignal * envelopeAmplitude * velocity;
    }

    void sampleOff(float* buffer) override
    {
        isActive = false;
    }
};