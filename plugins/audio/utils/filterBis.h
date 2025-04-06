#pragma once

#include <cstdint>
#include <math.h>


// Another implmentation of a resonant filter
class EffectFilterBisData {
public:
    EffectFilterBisData(uint64_t _sampleRate)
        : sampleRate(_sampleRate)
    {
        updateCoefficients();
    }

    void setCutoff(float _cutoff)
    {
        cutoff = _cutoff;
        updateCoefficients();
    }

    void setResonance(float _resonance)
    {
        resonance = _resonance;
        updateCoefficients();
    }

    void setSampleData(float inputValue)
    {
        // State-variable filter implementation
        float hp = (inputValue - (z1 + feedback * z2)) * highPassCoeff;
        float bp = hp * bandPassCoeff + z1;
        float lp = bp * lowPassCoeff + z2;

        // Update state variables
        z1 = bp;
        z2 = lp;

        // Output the filtered signal
        this->hp = hp;
        this->bp = bp;
        this->lp = lp;
    }

    float processLp(float inputValue)
    {
        setSampleData(inputValue);
        return lp;
    }

    float processHp(float inputValue)
    {
        setSampleData(inputValue);
        return hp;
    }

    float processBp(float inputValue)
    {
        setSampleData(inputValue);
        return bp;
    }

private:
void updateCoefficients() {
    float minFrequency = 100.0f;    // Minimum frequency in Hz
    float maxFrequency = 8000.0f; // Maximum frequency in Hz
    float frequency = minFrequency * std::pow(maxFrequency / minFrequency, cutoff);

    // Calculate filter coefficients
    float omega = 2.0f * M_PI * frequency / sampleRate;
    float alpha = sin(omega) / (2.0f * resonance);

    // Apply a small damping factor to prevent instability
    float damping = 1.0f / (1.0f + alpha);

    lowPassCoeff = damping * (1.0f - cos(omega));
    bandPassCoeff = damping * sin(omega);
    highPassCoeff = damping * (1.0f + cos(omega)) / 2.0f;

    float reso = resonance * 0.98f + 0.01f;
    feedback = reso + reso / (1.0f - alpha);
}

    float cutoff;
    float resonance;
    float sampleRate;

    float lowPassCoeff;
    float bandPassCoeff;
    float highPassCoeff;
    float feedback;

    float z1 = 0.0f; // State variable for band-pass
    float z2 = 0.0f; // State variable for low-pass

public:
    float lp = 0.0f; // Low-pass output
    float hp = 0.0f; // High-pass output
    float bp = 0.0f; // Band-pass output
};