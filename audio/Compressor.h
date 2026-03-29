#pragma once

#include <algorithm>
#include <cmath>

class Compressor {
public:
    float threshold = -6.0f; // dB
    float ratio = 3.0f; // ratio:1
    float attack = 0.005f; // Seconds
    float release = 0.100f; // Seconds
    float makeupGain = 1.2f; // Linear multiplier

    float gainReduction = 1.0f; // Current linear scaling (1.0 = no reduction)

private:
    float envelope = 0.0f;
    float sampleRate = 44100.0f;

public:
    Compressor(float sr = 44100.0f)
        : sampleRate(sr)
    {
    }

    void setSampleRate(float sr)
    {
        sampleRate = sr;
    }

    float process(float input)
    {
        float absInput = std::abs(input);

        float theta = (absInput > envelope) ? attack : release;
        float alpha = std::exp(-1.0f / (sampleRate * theta));
        envelope = (1.0f - alpha) * absInput + alpha * envelope;

        float envDb = 20.0f * std::log10(std::max(envelope, 1.0e-6f));

        float gainDb = 0.0f;
        if (envDb > threshold) {
            gainDb = (threshold - envDb) * (1.0f - 1.0f / ratio);
        }

        gainReduction = std::pow(10.0f, gainDb / 20.0f);

        return input * gainReduction * makeupGain;
    }

    float getGainReductionDb() const
    {
        return 20.0f * std::log10(gainReduction);
    }
};
