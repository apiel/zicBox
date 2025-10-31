#pragma once

#include "helpers/clamp.h"
#include "audio/effects/applyRingModFast.h"
// #include "audio/effects/applyRingMod.h"
#include "audio/effects/tinyReverb.h"

#include <cstdint>
#include <vector>
#include <cmath>

class DrumStringEngine {
protected:
    int sampleRate;
    static constexpr uint32_t MAX_DELAY = 1 << 16; // 65536

    float* tinyReverbBuffer;

    float stringRingPhase = 0.0f;
    int reverbPos = 0;
    float applyStringFx(float out)
    {
        out = applyRingModFast(out, ringMod, stringRingPhase, sampleRate);
        // out = applyRingMod(out, ringMod, stringRingPhase, sampleRate);
        out = tinyReverb(out, reverb, reverbPos, tinyReverbBuffer);
        return out;
    }

    float stringLfoPhase = 0.0f;
    float nextStringLfo()
    {
        stringLfoPhase += (2.0f * M_PI * lfoRate) / sampleRate;
        if (stringLfoPhase > 2.0f * M_PI)
            stringLfoPhase -= 2.0f * M_PI;

        // Sine wave between 0..1
        // return 0.5f * (1.0f + sinf(stringLfoPhase));
        return 0.5f * (1.0f + fastSin2(stringLfoPhase));
    }
public:
    float damping = 0.5f; // 0.0 to 1.0
    float decay = 0.99f; // 0.80 to 0.99
    float toneLevel = 0.5f; // 0.0 to 1.0
    int8_t pitch = 0; // -36 to 36
    float pluckNoise = 0.5f; // 0.0 to 1.0
    float ringMod = 0.0f; // 0.0 to 1.0
    float reverb = 0.5f; // 0.0 to 1.0
    float lfoRate = 5.0f; // Hz (LFO speed)
    float lfoDepth = 0.2f; // 0.0–1.0 amplitude modulation depth

    DrumStringEngine(int sampleRate, float* tinyReverbBuffer)
        : sampleRate(sampleRate)
        , tinyReverbBuffer(tinyReverbBuffer)
    {
    }

    void setDamping(float value) { damping = CLAMP(value, 0.0f, 1.0f); }
    void setDecay(float value) { decay = CLAMP(value, 0.0f, 1.0f); }
    void setToneLevel(float value) { toneLevel = CLAMP(value, 0.0f, 1.0f); }
    void setPitch(int8_t value) { pitch = CLAMP(value, -36, 36); }
    void setPluckNoise(float value) { pluckNoise = CLAMP(value, 0.0f, 1.0f); }
    void setRingMod(float value) { ringMod = CLAMP(value, 0.0f, 1.0f); }
    void setReverb(float value) { reverb = CLAMP(value, 0.0f, 1.0f); }
    void setLfoRate(float value) { lfoRate = CLAMP(value, 0.01f, 20.0f); }
    void setLfoDepth(float value) { lfoDepth = CLAMP(value, 0.0f, 1.0f); }

    void noteOn(uint8_t note)
    {
        note += pitch - 24; // Let's remove 2 octaves
        float freq = 440.0f * powf(2.0f, (note - 69) / 12.0f);
        if (freq < 20.0f)
            freq = 20.0f;
        if (freq > sampleRate * 0.45f)
            freq = sampleRate * 0.45f;

        stringDelayLen = std::min<uint32_t>(MAX_DELAY, std::max<uint32_t>(2, (uint32_t)std::round(sampleRate / freq)));
        delayLine.assign(stringDelayLen + 4, 0.0f);

        // White noise
        for (uint32_t i = 0; i < stringDelayLen; ++i) {
            float n = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            delayLine[i] = n * (pluckNoise + 0.5f);
        }

        stringWritePos = 0;
        onePoleState = 0.0f;
    }

protected:
    std::vector<float> delayLine;
    uint32_t stringDelayLen = 0;
    uint32_t stringWritePos = 0;
    float onePoleState = 0.0f;

public:
    float sample()
    {
        if (stringDelayLen == 0)
            return 0.0f;

        uint32_t rp = stringWritePos % stringDelayLen;
        uint32_t rp1 = (rp + 1) % stringDelayLen;
        float s0 = delayLine[rp];
        float s1 = delayLine[rp1];
        float out = 0.5f * (s0 + s1);

        // one-pole lowpass
        float cutoff = std::max(0.001f, toneLevel * (1.05f - damping) + 0.05f);
        onePoleState += cutoff * (out - onePoleState);
        float filtered = onePoleState;

        // feedback
        float fb = decay;
        delayLine[stringWritePos % stringDelayLen] = filtered * fb;

        stringWritePos = (stringWritePos + 1) % stringDelayLen;

        float outputGain = 2.0f * (1.0f - damping + 0.05f);

        // // Auto stop
        // static float avgEnergy = 0.0f;
        // avgEnergy = 0.999f * avgEnergy + 0.001f * (filtered * filtered); // moving RMS
        // if (avgEnergy < 1e-6f) { // silence threshold
        //     stringDelayLen = 0;
        //     delayLine.clear();
        // }

        float lfo = nextStringLfo();
        float lfoAmp = 1.0f - lfoDepth + (lfo * lfoDepth * 2.0f);

        return applyStringFx(filtered * outputGain * lfoAmp);
    }
};