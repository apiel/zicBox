#ifndef _WAVEFORM_H_
#define _WAVEFORM_H_

// https://potterdraw.sourceforge.io/Help/Modulation/Waveform.htm
// https://potterdraw.sourceforge.io/Help/Modulation/Slew.htm

// could make a LUT and do like envelop to get value between 2 points

// Param: edge sharpness for sine

// Param: stairs number?

// option1
// Param: point1 x
// Param: point1 y
// Param: point2 x
// Param: point2 y
// Param: width

// option4
// Param: angle from sawtooth to triangle
// Param: top width
// Param: bottom width

// option5
// Param: angle from sawtooth to triangle
// Param: stairs number: infinite triangle, 10, 9, to 1 became a square
// Param: pulse width: rectangular wave

// Type: sine, triangle, square,....
// Shape: sawtooth to triangle to soft edge triangle, square to circular...
// Pulse
// Stairs ?

#include "helpers/range.h"
#include "plugins/audio/lookupTable.h"
#include "plugins/audio/utils/WaveformInterface.h"
#include "plugins/audio/utils/utils.h"

#include <cstdint>

class Waveform : public WaveformInterface {
public:
    enum Type {
        Sine,
        Triangle,
        Square,
        Pulse,
        Fm,
        FmSquare,
    };

protected:
    LookupTable* sharedLut;
    uint64_t sampleRate;
    float shape = 0.5f;
    float macro = 0.5f;

    float lut[LOOKUP_TABLE_SIZE];

    Type selectedType = Type::Sine;

    // Use shape to introduce harmonic components (second and third harmonics).
    // Use macro to adjust the balance between the second and third harmonics for creative wave shaping.
    void loadSineType()
    {
        float maxAmplitude = 1.0f; // To track maximum amplitude for normalization

        // First pass: Compute the waveform and track the maximum amplitude
        for (uint16_t i = 0; i < sampleCount; i++) {
            float phase = i / (float)sampleCount; // Normalized phase [0, 1)
            float baseSine = sharedLut->sine[i]; // Original sine wave value

            // Add harmonic distortion
            float harmonicSine = baseSine
                + shape * macro * std::sin(2.0f * M_PI * phase * 2.0f) // Second harmonic
                + shape * (1.0f - macro) * std::sin(2.0f * M_PI * phase * 3.0f); // Third harmonic

            lut[i] = harmonicSine; // Store in LUT for now
            maxAmplitude = std::max(maxAmplitude, std::abs(harmonicSine));
        }

        // Second pass: Normalize to fit within [-1.0, 1.0]
        for (uint16_t i = 0; i < sampleCount; i++) {
            lut[i] /= maxAmplitude; // Scale down to fit within [-1.0, 1.0]
        }
    }

    void loadTriangleType()
    {
        float pulse = range(macro, 0.05f, 1.0f);
        for (uint16_t i = 0; i < sampleCount; i++) {
            float phase = i / (float)sampleCount; // Normalized phase [0, 1)

            // Adjust phase to fit within the pulse width
            if (phase < pulse) {
                phase /= pulse; // Scale phase within the active pulse region

                // Compute the waveform value based on the shape parameter
                float y = phase < shape
                    ? (1.0f / shape) * phase // Rising edge
                    : (1.0f - (1.0f / (1.0f - shape)) * (phase - shape)); // Falling edge

                // Scale and store in the lookup table
                lut[i] = -(2.0f * y - 1.0f);
            } else {
                // Outside the pulse range, set to 0
                lut[i] = 0.0f;
            }
        }
    }

    void loadPulseType()
    {
        float pulse = range(macro, 0.1f, 0.9f);
        for (uint16_t i = 0; i < sampleCount; i++) {
            float phase = i / (float)sampleCount; // Normalized phase [0, 1)
            float dutyCycle = pulse; // Pulse width directly set by `pulse` parameter

            float value;
            if (phase < dutyCycle) {
                // High part of the pulse
                float t = phase / dutyCycle; // Normalized phase within the high region
                if (shape < 0.5f) {
                    // Interpolate between sharp and rounded
                    float mix = shape * 2.0f; // Normalize shape to [0, 1]
                    float sharp = 1.0f; // Pure sharp edge
                    float rounded = std::tanh(5.0f * (1.0f - t)); // Smooth transition down
                    value = sharp * (1.0f - mix) + rounded * mix;
                } else {
                    // Interpolate between rounded and circular
                    float mix = (shape - 0.5f) * 2.0f; // Normalize shape to [0, 1]
                    float rounded = std::tanh(5.0f * (1.0f - t)); // Smooth transition down
                    float circular = std::sin(t * M_PI_2); // Circular transition down
                    value = rounded * (1.0f - mix) + circular * mix;
                }
            } else {
                // Low part of the pulse
                float t = (phase - dutyCycle) / (1.0f - dutyCycle); // Normalized phase within the low region
                if (shape < 0.5f) {
                    // Interpolate between sharp and rounded
                    float mix = shape * 2.0f; // Normalize shape to [0, 1]
                    float sharp = -1.0f; // Pure sharp edge
                    float rounded = -std::tanh(5.0f * t); // Smooth transition up
                    value = sharp * (1.0f - mix) + rounded * mix;
                } else {
                    // Interpolate between rounded and circular
                    float mix = (shape - 0.5f) * 2.0f; // Normalize shape to [0, 1]
                    float rounded = -std::tanh(5.0f * t); // Smooth transition up
                    float circular = -std::sin(t * M_PI_2); // Circular transition up
                    value = rounded * (1.0f - mix) + circular * mix;
                }
            }

            // Store the computed value in the lookup table
            lut[i] = value;
        }
    }

    void loadSquareType()
    {
        for (uint16_t i = 0; i < sampleCount; i++) {
            float phase = i / (float)sampleCount; // Normalized phase [0, 1)

            // Base square wave
            float square1 = phase < 0.5f ? 1.0f : -1.0f;

            // Second square wave at double the frequency
            float square2 = ((int)(phase * 2) % 2) == 0 ? 1.0f : -1.0f;

            // Split macro into two regions
            if (macro <= 0.5f) {
                // First behavior: Phase shift modulation
                float modulatedPhase = std::fmod(phase + macro * 2.0f, 1.0f); // Scale macro for 0-0.5 range
                float modulation = shape * std::cos(modulatedPhase * M_PI * 2);
                float modulated = square2 * modulation;

                // Blend the base and modulated square waves using the shape parameter
                lut[i] = (1.0f - shape) * square1 + shape * (square1 - modulated) * 0.5f;
            } else {
                // Second behavior: Frequency modulation
                float fmAmount = (macro - 0.5f) * 4.0f; // Scale FM strength for 0.5-1.0 range
                float modulatedPhase = std::fmod(phase + fmAmount * std::sin(phase * M_PI * 2), 1.0f);
                float modulation = shape * std::cos(modulatedPhase * M_PI * 2);
                float modulated = square2 * modulation;

                // Blend the base and frequency-modulated waveforms using the shape parameter
                lut[i] = (1.0f - shape) * square1 + shape * (square1 - modulated) * 0.5f;
            }
        }
    }

    void loadFmSquareType()
    {
        // FM parameters
        float carrierFreq = 1.0f; // Base frequency for the carrier
        float modulatorFreq = carrierFreq * (1.0f + macro); // Modulator frequency adjusted by macro

        // FM modulation depth: higher shape means more modulation
        float modulationIndex = shape * 10.0f; // Controls modulation depth

        for (uint16_t i = 0; i < sampleCount; i++) {
            float phase = i / (float)sampleCount; // Normalized phase [0, 1)

            // Carrier sine wave (basic tone)
            float carrier = std::sin(phase * 2.0f * M_PI * carrierFreq);

            // Modulator sine wave (modulating signal)
            float modulator = std::sin(phase * 2.0f * M_PI * modulatorFreq);

            // Apply FM modulation: the modulator modulates the carrier based on modulation depth
            float modulatedWave = carrier + modulationIndex * modulator;

            // Normalize to [0, 1]
            modulatedWave = 0.5f * (modulatedWave + 1.0f); // Shift from [-1, 1] to [0, 1]

            // Store the result in the lookup table
            lut[i] = range(modulatedWave, -1.0f, 1.0f); // Ensure value is within bounds
        }
    }

    void loadFmType()
    {
        // FM parameters
        float carrierFreq = 1.0f; // Base frequency for the carrier
        float modulatorFreq = carrierFreq * (1.0f + macro); // Modulator frequency adjusted by macro

        // FM modulation depth: higher shape means more modulation
        float modulationIndex = shape * 10.0f; // Controls modulation depth

        float maxAmplitude = 0.0f; // Variable to track the max amplitude during FM computation

        // First pass to compute the maximum amplitude
        for (uint16_t i = 0; i < sampleCount; i++) {
            float phase = i / (float)sampleCount; // Normalized phase [0, 1)

            // Carrier sine wave (basic tone)
            float carrier = std::sin(phase * 2.0f * M_PI * carrierFreq);

            // Modulator sine wave (modulating signal)
            float modulator = std::sin(phase * 2.0f * M_PI * modulatorFreq);

            // Apply FM modulation: the modulator modulates the carrier based on modulation depth
            float modulatedWave = carrier + modulationIndex * modulator;

            // Track the maximum absolute amplitude value for normalization
            maxAmplitude = std::max(maxAmplitude, std::abs(modulatedWave));
        }

        // Second pass to apply the modulation and normalization
        for (uint16_t i = 0; i < sampleCount; i++) {
            float phase = i / (float)sampleCount; // Normalized phase [0, 1)

            // Carrier sine wave (basic tone)
            float carrier = std::sin(phase * 2.0f * M_PI * carrierFreq);

            // Modulator sine wave (modulating signal)
            float modulator = std::sin(phase * 2.0f * M_PI * modulatorFreq);

            // Apply FM modulation: the modulator modulates the carrier based on modulation depth
            float modulatedWave = carrier + modulationIndex * modulator;

            // Normalize the waveform to fit within the [0, 1] range
            modulatedWave /= maxAmplitude; // Normalize by the maximum amplitude from the first pass

            // Soft clip if necessary to avoid extreme clipping (optional)
            lut[i] = range(modulatedWave, -1.0f, 1.0f);
        }
    }

public:
    Waveform(LookupTable* sharedLut, uint64_t sampleRate)
        : WaveformInterface(LOOKUP_TABLE_SIZE)
        , sharedLut(sharedLut)
        , sampleRate(sampleRate)
    {
        setWaveformType(Type::Sine);
    }

    float sample(float* index, float freq) override
    {
        float modulatedFreq = 110.0f * freq;
        float phaseIncrement = modulatedFreq / sampleRate;

        *index += phaseIncrement;
        while (*index >= 1.0f) {
            *index -= 1.0f;
        }

        // return lut[(uint16_t)(*index * sampleCount)];
        return linearInterpolation(*index, sampleCount, lut);
    }

    float* samples() override
    {
        return lut;
    }

    void setWaveformType(Type waveformType, bool reset = true)
    {
        selectedType = waveformType;
        switch (waveformType) {
        case Type::Sine: {
            if (reset) {
                shape = 0.0f;
                macro = 0.0f;
            }
            loadSineType();
            break;
        }
        case Type::Triangle: {
            if (reset) {
                shape = 0.5f; // triangle
                macro = 1.0f; // no pulse
            }
            loadTriangleType();
            break;
        }
        case Type::Square: {
            if (reset) {
                shape = 0.0f; // square
                macro = 0.0f;
            }
            loadSquareType();
            break;
        }
        case Type::Pulse: {
            if (reset) {
                shape = 0.0f; // square
                macro = 0.5f; // centered
            }
            loadPulseType();
            break;
        }
        case Type::Fm: {
            if (reset) {
                shape = 0.25f;
                macro = 1.0f;
            }
            loadFmType();
            break;
        }
        case Type::FmSquare: {
            if (reset) {
                shape = 0.50f;
                macro = 0.75f;
            }
            loadFmSquareType();
            break;
        }
        }
    }

    void setShape(float value)
    {
        shape = range(value, 0.0f, 1.0f);
        setWaveformType(selectedType, false);
    }

    void setMacro(float value)
    {
        macro = range(value, 0.0f, 1.0f);
        setWaveformType(selectedType, false);
    }
};

#endif
