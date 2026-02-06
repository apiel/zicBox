#pragma once

#include <cmath>
#include <cstdint>

class Kick2Engine {
public:
    // Kick parameters - adjust these for different sounds!
    float baseFrequency = 55.0f; // Starting frequency (Hz)
    float sweepDepth = 800.0f; // How much the pitch sweeps down
    float sweepDecay = 0.9994f; // How fast pitch envelope decays (0.9990-0.9998)
    float ampDecay = 0.9992f; // How fast amplitude decays (0.9985-0.9995)
    float compressionAmount = 0.5f; // Compression (0.0-1.0)
    float driveAmount = 0.6f; // Distortion/saturation (0.0-2.0)
    float clickAmount = 0.3f; // Click/attack transient (0.0-1.0)

    // Internal state
    float phase = 0.0f;
    float pitchEnv = 1.0f;
    float ampEnv = 1.0f;
    float clickEnv = 1.0f;

    // Sample rate (set by your system)
    static constexpr float SAMPLE_RATE = 44104.0f;

    void trigger()
    {
        phase = 0.0f;
        pitchEnv = 1.0f;
        ampEnv = 1.0f;
        clickEnv = 1.0f;
    }

    float process(float velocity)
    {
        // Check if kick finished
        if (ampEnv < 0.0001f) {
            return 2048; // Return silence
        }

        // === ENVELOPE STAGE ===
        pitchEnv *= sweepDecay; // Pitch sweep envelope
        ampEnv *= ampDecay; // Amplitude envelope
        clickEnv *= 0.95f; // Fast-decaying click envelope

        // === OSCILLATOR STAGE ===
        // Calculate instantaneous frequency with pitch sweep
        float currentFreq = baseFrequency + (sweepDepth * pitchEnv);

        // Phase increment (normalized frequency)
        float phaseInc = currentFreq / SAMPLE_RATE;
        phase += phaseInc;
        if (phase >= 1.0f) phase -= 1.0f;

        // Generate sine wave
        float rawSine = std::sin(2.0f * 3.14159265359f * phase);

        // === CLICK/TRANSIENT STAGE ===
        // Add high-frequency click at the start for punch
        float click = clickEnv * clickAmount;

        // === WAVESHAPING/DISTORTION STAGE ===
        // Soft saturation using cubic distortion
        float shaped = (rawSine + driveAmount * (rawSine * rawSine * rawSine))
            / (1.0f + driveAmount);

        // Mix shaped sine with click
        float mixed = shaped + click;

        // === AMPLITUDE STAGE ===
        float finalMix = mixed * ampEnv * velocity;

        // === COMPRESSION STAGE ===
        // Soft knee compression (makes it punchier)
        float absSig = std::abs(finalMix);
        if (absSig > 0.5f) {
            float excess = absSig - 0.5f;
            float compressed = 0.5f + excess * (1.0f - compressionAmount);
            finalMix = (finalMix > 0) ? compressed : -compressed;
        }

        // === DAC CONVERSION ===
        // Map from [-1.0, 1.0] to [0, 4095] for 12-bit DAC
        // Center at 2048
        float normalized = (finalMix + 1.0f) * 0.5f;

        // Clamp to [0, 1]
        if (normalized > 1.0f) normalized = 1.0f;
        if (normalized < 0.0f) normalized = 0.0f;

        return normalized;
    }

    // Check if kick is still playing
    bool isActive() const
    {
        return ampEnv > 0.0001f;
    }

    // Get remaining kick duration in samples (approximately)
    uint32_t getRemainingDuration() const
    {
        if (ampEnv < 0.0001f) return 0;
        // Estimate based on decay rate
        return static_cast<uint32_t>(-std::log(0.0001f / ampEnv) / std::log(ampDecay));
    }
};

static Kick2Engine kick;