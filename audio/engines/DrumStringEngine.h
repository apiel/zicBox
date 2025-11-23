/** Description:
This header file defines the blueprint for a specialized digital sound synthesizer called `DrumStringEngine`. Its primary purpose is to generate realistic or modeled percussive and string-like sounds, similar to a plucked guitar, banjo, or a ringing drum.

### Core Functionality

The engine operates on the principle of *physical modeling*, simulating the vibration of a string or membrane. When a note is triggered, the engine calculates the required length for a virtual delay line (like setting the length of a physical string to determine its pitch). It then introduces an initial burst of noise—the "pluck"—into this line.

In a continuous loop, the engine generates sound by circulating the signal through the delay line. As the signal loops, it is filtered, dampened, and reduced in volume, mimicking how a real string vibrates and fades over time.

### Controllable Sound Parameters

The user controls the character of the sound through several key settings:

*   **Pitch:** Determines the fundamental musical note.
*   **Damping & Tone Level:** Controls how quickly high frequencies are lost, affecting the perceived "brightness" or "muffled" quality of the sound.
*   **Decay:** Sets the rate at which the sound volume fades out.
*   **Pluck Noise:** Adjusts the amount of initial noisy attack when the string is struck.
*   **LFO Rate & Depth:** Controls a slow, cyclical change (like a vibrato or tremolo), adding movement to the sound.
*   **Ring Modulation:** Applies a complex, often metallic or bell-like audio effect.
*   **Reverb:** Adds a sense of space and echo.

### Technical Structure

The `DrumStringEngine` is designed to be highly efficient, especially in embedded systems. It includes optional optimizations to use pre-calculated mathematical tables (`lookupTable`) and "fast math" functions for quicker processing. It also provides built-in mechanisms to easily save (`serialize`) and load (`hydrate`) all its current settings, allowing the user to preserve custom sounds.

sha: cbe32942833cd0c885b6d8e37aeed15f4c121fd415a35d9f5d53a5a19657eca4 
*/
#pragma once

#include "audio/engines/Engine.h"
#ifdef USE_LUT_AND_FAST_MATH
#include "audio/effects/applyRingModFast.h"
#else
#include "audio/effects/applyRingMod.h"
#endif
#include "audio/effects/tinyReverb.h"
#include "audio/lookupTable.h"
#include "helpers/clamp.h"
#include "helpers/math.h"

#include <cmath>
#include <cstdint>
#include <vector>
class DrumStringEngine : public Engine {
protected:
    int sampleRate;
    LookupTable& lookupTable;
    static constexpr uint32_t MAX_DELAY = 1 << 16; // 65536

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

    void hydrate(const std::vector<KeyValue>& values) override
    {
        for (auto& kv : values) {
            if (kv.key == "damping") damping = std::get<float>(kv.value);
            else if (kv.key == "decay") decay = std::get<float>(kv.value);
            else if (kv.key == "toneLevel") toneLevel = std::get<float>(kv.value);
            else if (kv.key == "pitch") pitch = std::get<float>(kv.value);
            else if (kv.key == "pluckNoise") pluckNoise = std::get<float>(kv.value);
            else if (kv.key == "ringMod") ringMod = std::get<float>(kv.value);
            else if (kv.key == "reverb") reverb = std::get<float>(kv.value);
            else if (kv.key == "lfoRate") lfoRate = std::get<float>(kv.value);
            else if (kv.key == "lfoDepth") lfoDepth = std::get<float>(kv.value);
        }
    }
    std::vector<KeyValue> serialize() const override { return {
        { "damping", damping },
        { "decay", decay },
        { "toneLevel", toneLevel },
        { "pitch", (float)pitch },
        { "pluckNoise", pluckNoise },
        { "ringMod", ringMod },
        { "reverb", reverb },
        { "lfoRate", lfoRate },
        { "lfoDepth", lfoDepth },
    }; }

    DrumStringEngine(int sampleRate, LookupTable& lookupTable, float* tinyReverbBuffer)
        : Engine(Engine::Type::Drum, "String", "String")
        , sampleRate(sampleRate)
        , lookupTable(lookupTable)
        , tinyReverbBuffer(tinyReverbBuffer)
    {
    }

    void setDamping(float value) { damping = CLAMP(value, 0.0f, 1.0f); }
    void setDecay(float value) { decay = CLAMP(value, 0.80f, 0.99f); }
    void setToneLevel(float value) { toneLevel = CLAMP(value, 0.0f, 1.0f); }
    void setPitch(int8_t value) { pitch = CLAMP(value, -36, 36); }
    void setPluckNoise(float value) { pluckNoise = CLAMP(value, 0.0f, 1.0f); }
    void setRingMod(float value) { ringMod = CLAMP(value, 0.0f, 1.0f); }
    void setReverb(float value) { reverb = CLAMP(value, 0.0f, 1.0f); }
    void setLfoRate(float value) { lfoRate = CLAMP(value, 0.01f, 20.0f); }
    void setLfoDepth(float value) { lfoDepth = CLAMP(value, 0.0f, 1.0f); }

    void noteOn(uint8_t note) override
    {
        note += pitch - 24; // Let's remove 2 octaves
        float freq = 440.0f * powf(2.0f, (note - 69) / 12.0f);
        freq = CLAMP(freq, 20.0f, sampleRate * 0.45f);

        stringDelayLen = std::min<uint32_t>(MAX_DELAY, std::max<uint32_t>(2, (uint32_t)std::round(sampleRate / freq)));
        delayLine.assign(stringDelayLen + 4, 0.0f);

        for (uint32_t i = 0; i < stringDelayLen; ++i) {
            float n = lookupTable.getNoise();
            delayLine[i] = n * (pluckNoise + 0.5f);
        }

        stringWritePos = 0;
        onePoleState = 0.0f;
        // totalSamples = (stringDelayLen / (float)sampleRate) * (std::log(0.001f) / std::log(decay)) * sampleRate;
    }

protected:
    float* tinyReverbBuffer;

    // int totalSamples = 0;
    float stringRingPhase = 0.0f;
    int reverbPos = 0;
    std::vector<float> delayLine;
    uint32_t stringDelayLen = 0;
    uint32_t stringWritePos = 0;
    float onePoleState = 0.0f;
    float stringLfoPhase = 0.0f;

public:
    float sample() override
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

        // TODO define a 2PI however in this case i think we could even bypass using LUT getSin2()
        stringLfoPhase += (PI_X2 * lfoRate) / sampleRate;
        if (stringLfoPhase > PI_X2)
            stringLfoPhase -= PI_X2;

#ifdef USE_LUT_AND_FAST_MATH
        float lfo = 0.5f * (1.0f + fastSin2(stringLfoPhase));
#else
        float lfo = 0.5f * (1.0f + sinf(stringLfoPhase));
#endif

        float lfoAmp = 1.0f - lfoDepth + (lfo * lfoDepth * 2.0f);

        out = filtered * outputGain * lfoAmp;

#ifdef USE_LUT_AND_FAST_MATH
        out = applyRingModFast(out, ringMod, stringRingPhase, sampleRate);
#else
        out = applyRingMod(out, ringMod, stringRingPhase, sampleRate);
#endif

        out = tinyReverb(out, reverb, reverbPos, tinyReverbBuffer);
        return out;
    }
};