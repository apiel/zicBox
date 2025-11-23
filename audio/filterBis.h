/** Description:
This C++ structure defines an advanced digital audio processing component, specifically an implementation of a **resonant filter**. This kind of filter is crucial in applications like music synthesizers or audio effects, where the quality or timbre of sound needs to be dynamically altered.

### What It Does

The main purpose of the `EffectFilterBisData` class is to take a continuous stream of audio samples and mathematically modify them based on user-defined settings, producing a filtered output.

### Key Controls and Setup

1.  **Sample Rate:** When the filter is initialized, it requires the audio system's `sample rate` (how quickly the audio is being processed digitally).
2.  **Cutoff:** This determines the specific frequency point where the filter begins to affect the sound (e.g., muffling everything above 1000 Hz).
3.  **Resonance:** This controls how sharp or exaggerated the sounds are right around the cutoff frequency, often leading to a "ringing" or distinct sound.

### How It Works

The filter operates by continuously recalculating its internal behavior whenever the cutoff or resonance changes. This process involves complex trigonometric math (like sine and cosine functions) to generate precise **coefficients**.

When an audio sample enters the filter, it uses these coefficients along with internal memory points (called "state variables," representing the history of the previous samples) to calculate the new filtered sound. This method ensures that the filtering is smooth and resonant, acting much like real-world analog audio circuits.

A key feature of this design is that for every input sample, it simultaneously generates three separate filtered outputs:

1.  **Low-Pass (LP):** Allows bass frequencies to pass, muffling high sounds.
2.  **High-Pass (HP):** Allows treble frequencies to pass, muffling low sounds.
3.  **Band-Pass (BP):** Only allows a narrow band of frequencies to pass through, muting both very high and very low sounds.

sha: b0187183f245f74369df769d39dbc3d18d1fa7e2fdb8f0fcacba328ed5d9859f 
*/
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