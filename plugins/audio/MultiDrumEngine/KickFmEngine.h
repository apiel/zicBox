#pragma once

#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/effects/applyDrive.h"
#include <cmath>

class KickFmEngine : public DrumEngine {
protected:
    float velocity = 1.0f;
    float carrierPhase = 0.0f;
    float modulatorPhase = 0.0f;
    float modulatorFeedbackState = 0.0f; // Stores the previous modulator sample
    bool isActive = false;

    float modulationEnvelope = 0.0f;
    float lowPassState = 0.0f;

    // --- Parameters (10 total) ---
    Val& baseFrequency = val(50.0f, "FREQ", { .label = "Sub Freq", .min = 30.0, .max = 100.0, .unit = "Hz" });
    Val& pitchOffset = val(0.0f, "PITCH", { .label = "Pitch", .type = VALUE_CENTERED, .min = -12, .max = 12 });

    // FM Core
    Val& fmAmount = val(40, "FM_AMOUNT", { .label = "FM Depth", .unit = "%" });
    Val& fmRatio = val(1.0f, "FM_RATIO", { .label = "FM Ratio", .min = 0.5f, .max = 8.0f, .step = 0.1f });
    Val& fmDecay = val(20, "FM_DECAY", { .label = "FM Speed", .unit = "%" });
    
    // NEW: FM Feedback (Turns the modulator into a complex harmonic generator)
    Val& fmFeedback = val(0, "FM_FEEDBACK", { .label = "FM Grit", .unit = "%" });
    
    // NEW: Env Curve (Controls the "snappiness" of the FM zap)
    Val& fmCurve = val(50, "FM_CURVE", { .label = "FM Curve", .unit = "%" });

    // Shaping & Master
    Val& punch = val(50, "PUNCH", { .label = "Punch", .unit = "%" }); 
    Val& drive = val(25, "DRIVE", { .label = "Drive", .unit = "%" });
    Val& tone = val(60, "TONE", { .label = "Tone", .unit = "%" });

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
        modulatorFeedbackState = 0.0f;
        modulationEnvelope = 1.0f;
        lowPassState = 0.0f;
        isActive = true;
    }

    void sampleOn(float* buffer, float envelopeAmplitude, int sampleCounter, int totalSamples) override
    {
        if (!isActive) return;

        // 1. FM Envelope logic with variable curve
        // High curve values make the envelope drop much faster (more "snappy")
        float curvePower = 1.0f + (fmCurve.pct() * 4.0f);
        float fmDecayTime = 0.002f + (fmDecay.pct() * 0.15f);
        
        // Base linear-ish decay
        modulationEnvelope *= expf(-1.0f / (props.sampleRate * fmDecayTime));
        
        // Apply the curve to the envelope result
        float shapedFmEnvelope = powf(modulationEnvelope, curvePower);

        // 2. Frequencies
        float rootFreq = baseFrequency.get() * powf(2.0f, (pitchOffset.get()) / 12.0f);
        float pitchSpike = (punch.pct() * 400.0f * shapedFmEnvelope);
        float carrierFreq = rootFreq + pitchSpike;
        float modulatorFreq = carrierFreq * fmRatio.get();

        // 3. Modulator with Feedback
        // Feedback loop: the last modulator sample is added to its own phase
        float feedbackAmt = fmFeedback.pct() * 0.25f; 
        float modulatorLookup = modulatorPhase + (modulatorFeedbackState * feedbackAmt);
        
        float modulatorSignal = sinf(2.0f * M_PI * modulatorLookup);
        modulatorFeedbackState = modulatorSignal; // Save for next sample
        
        modulatorPhase += modulatorFreq / props.sampleRate;
        if (modulatorPhase > 1.0f) modulatorPhase -= 1.0f;

        // 4. Carrier Oscillator (Phase Modulation)
        float fmIntensity = fmAmount.pct() * 1.5f * shapedFmEnvelope;
        carrierPhase += (carrierFreq / props.sampleRate) + (modulatorSignal * fmIntensity * 0.1f);
        if (carrierPhase > 1.0f) carrierPhase -= 1.0f;
        
        float finalSignal = sinf(2.0f * M_PI * carrierPhase);

        // 5. Processing
        // Drive for warmth
        if (drive.pct() > 0.0f) {
            finalSignal = applyDrive(finalSignal, drive.pct() * 3.0f, props.lookupTable);
        }

        // Tone LPF
        float filterCut = 0.05f + tone.pct() * 0.8f;
        lowPassState += filterCut * (finalSignal - lowPassState);
        finalSignal = lowPassState;

        buffer[track] = finalSignal * envelopeAmplitude * velocity;
    }

    void sampleOff(float* buffer) override
    {
        isActive = false;
    }
};
