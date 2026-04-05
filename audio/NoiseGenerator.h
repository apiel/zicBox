#pragma once

// To get a simple noise generator have a look at audio/utils/noise.h
//
// This is a more complex noise generator, which allow you to have different noise types
// which need to save internal state in order to produce them.

#include <cstdint>
#include <cstdlib>

#include "audio/utils/noise.h"

class NoiseGenerator {
protected:
    float sampleRateDiv;

    float lastNoise = 0.0f;
    float pinkStore[7] = { 0.0f };
    float brownState = 0.0f;
    float shPhase = 0.0f;
    float shValue = 0.0f;

    static float lerp(float a, float b, float t) { return a + t * (b - a); }

public:
    NoiseGenerator(float sampleRate)
        : sampleRateDiv(1.0f / sampleRate)
    {
    }

    float getBrownNoise(float white) { return (brownState = (brownState + (0.02f * white)) / 1.02f) * 3.5f; }

    float getPinkNoise()
    {
        // Pink noise really need random white noise
        float white = (float)rand() / (float)RAND_MAX * 2.0f - 1.0f;

        pinkStore[0] = 0.99886f * pinkStore[0] + white * 0.0555179f;
        pinkStore[1] = 0.99332f * pinkStore[1] + white * 0.0750759f;
        pinkStore[2] = 0.96900f * pinkStore[2] + white * 0.1538520f;
        pinkStore[3] = 0.86650f * pinkStore[3] + white * 0.3104856f;
        pinkStore[4] = 0.55000f * pinkStore[4] + white * 0.5329522f;
        pinkStore[5] = -0.7616f * pinkStore[5] - white * 0.0168980f;
        float pink = (pinkStore[0] + pinkStore[1] + pinkStore[2] + pinkStore[3] + pinkStore[4] + pinkStore[5] + pinkStore[6] + white * 0.5362f) * 0.11f;
        pinkStore[6] = white * 0.115926f;
        return pink;
    }

    float getBlueNoise(float white)
    {
        float blue = (white - lastNoise) * 0.5f;
        lastNoise = white;
        return blue;
    }

    float getSHNoise(float rateHz)
    {
        shPhase += rateHz * sampleRateDiv;
        if (shPhase >= 1.0f) {
            shPhase -= 1.0f;
            shValue = Noise::get();
        }
        return shValue;
    }

    float getWhite() { return Noise::get(); }

    float get(float color)
    {
        float white = Noise::get();

        // color 0.0 to 0.5: Brown -> Pink -> White -> Blue
        if (color <= 0.5f) {
            float t = color * 2.0f; // normalize to 0.0 - 1.0
            if (t < 0.33f) return lerp(getBrownNoise(white), getPinkNoise(), t / 0.33f);
            if (t < 0.66f) return lerp(getPinkNoise(), white, (t - 0.33f) / 0.33f);
            return lerp(white, getBlueNoise(white), (t - 0.66f) / 0.34f);
        }

        // color 0.5 to 1.0: Blue morphing into S&H (300Hz to 800Hz)
        float tSH = (color - 0.5f) * 2.0f;
        float shRate = lerp(300.0f, 800.0f, tSH);

        if (color < 0.6f)
            return lerp(getBlueNoise(white), getSHNoise(shRate), tSH * 4.0f);
        return getSHNoise(shRate);
    }
};