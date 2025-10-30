#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/lookupTable.h"
#include "helpers/clamp.h"
#include "audio/effects/tinyReverb.h"

#include <cstdint>

class DrumMetalicEngine {
protected:
    int sampleRate;
    float* tinyReverbBuffer;

    float resonatorState = 0.0f;
    float applyResonator(float input)
    {
        // Advance "state" (acts like a time accumulator)
        resonatorState += 1.0f / sampleRate;

        // Compute output = input * e^(-decay * t) * sin(2π f t)
        float output = input * expf(-0.02f * resonatorState)
            * sinf(2.0f * M_PI * noteFreq * resonatorState * resonator);

        // Optional loudness compensation so higher freq = less drop in volume
        float compensation = sqrtf(220.0f / std::max(noteFreq, 1.0f));
        output *= compensation;

        float amount = resonator / 1.5f;
        return (output * amount) + (input * (1.0f - amount));
    }

    // Sine wave oscillator
    float sineWave(float frequency, float phase)
    {
        return sinf(2.0f * M_PI * frequency * phase);
        // return fastSin(2.0f * M_PI * frequency * phase);
    }

public:
    int duration = 1000; // 50 to 3000
    int8_t pitch = 0; // -36 to 36
    float resonator = 0.0f; // 0.00 to 1.50
    float timbre = 0.0f; // 0.00 to 1.00
    float fmAmplitude = 0.0f; // 0.00 to 1.00
    float fmFreq = 1.0f; // 1 to 500
    float envMod = 0.0f; // 0.00 to 1.00
    float reverb = 0.5f; // 0.0 to 1.0

    DrumMetalicEngine(int sampleRate, float* tinyReverbBuffer)
        : sampleRate(sampleRate)
        , tinyReverbBuffer(tinyReverbBuffer)
    {
        phaseIncrement = 1.0f / sampleRate;
    }

    void setDuration(int value) { duration = CLAMP(value, 50, 3000); }
    void setPitch(int value) { pitch = CLAMP(value, -36, 36); }
    void setResonator(float value) { resonator = CLAMP(value, 0.0f, 1.5f); }
    void setTimbre(float value) { timbre = CLAMP(value, 0.0f, 1.0f); }
    void setFmAmplitude(float value) { fmAmplitude = CLAMP(value, 0.0f, 1.0f); }
    void setFmFreq(float value) { fmFreq = CLAMP(value, 1.0f, 500.0f); }
    void setEnvMod(float value) { envMod = CLAMP(value, 0.0f, 1.0f); }
    void setReverb(float value) { reverb = CLAMP(value, 0.0f, 1.0f); }

    const uint8_t baseNote = 60;
    void noteOn(uint8_t note)
    {
        float freq = pow(2, ((note - baseNote + pitch) / 12.0));
        noteFreq = freq * 440.0f;

        // logDebug("freq: %f noteFreq: %f", freq, noteFreq);

        totalSamples = static_cast<int>(sampleRate * (duration / 1000.0f));
        sampleCounter = 0;

        phase = 0.0f;
        resonatorState = 0.0f;

        envValue = 1.0f;
        float envTarget = 0.001f; // end value at end of duration
        envDecayCoeff = powf(envTarget, 1.0f / totalSamples);
    }

protected:
    int totalSamples = 0;
    int sampleCounter = 0;
    float phase = 0.0f;
    float phaseIncrement = 0.0f;
    float noteFreq = 440.0f;
    int reverbPos = 0;
    float envValue = 1.0f;
    float envDecayCoeff = 0.0f;

public:
    float sample()
    {
        if (sampleCounter < totalSamples) {
            float t = float(sampleCounter) / totalSamples;
            sampleCounter++;

            float envAmp = envValue;
            envValue *= envDecayCoeff;

            float modulatedFreq = noteFreq;
            if (envMod > 0.0f) {
                // float envFactor = exp(-t);
                float envFactor = envAmp;
                modulatedFreq = noteFreq + noteFreq * envMod * envFactor;
            }

            float modulator = fmAmplitude > 0.0f ? sineWave(fmFreq, phase) * fmAmplitude * 10.0f : 0.0f;
            float tone = sineWave(modulatedFreq + modulator, phase);
            phase += phaseIncrement;

            if (resonator > 0.0f) {
                tone = applyResonator(tone);
            }

            if (timbre > 0.0f) {
                // Adjust timbre by filtering harmonics dynamically
                tone *= (1.0f - timbre) + timbre * sinf(2.0f * M_PI * noteFreq * 0.5f * t);
            }

            tone *= envAmp;
            tone = tinyReverb(tone, reverb, reverbPos, tinyReverbBuffer);
            return tone;
        }
        return tinyReverb(0.0f, reverb, reverbPos, tinyReverbBuffer);
    }
};