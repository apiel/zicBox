#pragma once

#define SKIP_SNDFILE

#include "audio/EnvelopDrumAmp.h"
#include "audio/KickEnvTableGenerator.h"
#include "audio/KickTransientTableGenerator.h"
#include "audio/WavetableGenerator2.h"
#include "audio/effects/applySample.h"
#include "audio/filterArray.h"
#include "audio/lookupTable.h"

#include "audio/engines/DrumToneEngine.h"
#include "audio/engines/DrumClapEngine.h"

#include <cmath>
#include <string>
#include <vector>
class Audio {
protected:
    LookupTable lookupTable;

    float velocity = 1.0f;

    Audio()
        : tone(sampleRate, &lookupTable)
        , clap(sampleRate, lookupTable)
    {
    }

    // String
    std::vector<float> delayLine;
    uint32_t stringDelayLen = 0;
    uint32_t stringWritePos = 0;
    float onePoleState = 0.0f;
    float stringTone()
    {
        if (stringDelayLen == 0)
            return 0.0f;

        uint32_t rp = stringWritePos % stringDelayLen;
        uint32_t rp1 = (rp + 1) % stringDelayLen;
        float s0 = delayLine[rp];
        float s1 = delayLine[rp1];
        float out = 0.5f * (s0 + s1);

        // one-pole lowpass
        float cutoff = std::max(0.001f, stringToneLevel * (1.05f - damping) + 0.05f);
        onePoleState += cutoff * (out - onePoleState);
        float filtered = onePoleState;

        // feedback
        float fb = stringDecay;
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
        float lfoAmp = 1.0f - stringLfoDepth + (lfo * stringLfoDepth * 2.0f);

        return applyStringFx(filtered * outputGain * stringVolume * stringVolume * lfoAmp);
    }
    // TODO fix still play even when finished...

    static constexpr uint32_t MAX_DELAY = 1 << 16; // 65536
    void stringNoteOn(uint8_t note)
    {
        if (stringVolume > 0.0f) {
            note += stringPitch - 24; // Let's remove 2 octaves
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
                delayLine[i] = n * (stringPluckNoise + 0.5f);
            }

            stringWritePos = 0;
            onePoleState = 0.0f;
        }
    }

    float stringRingPhase = 0.0f;
    float applyStringFx(float out)
    {
        out = applyRingMod(out, stringRingMod, stringRingPhase, sampleRate);
        out = tinyStringReverb(out);
        return out;
    }

    float stringLfoPhase = 0.0f;
    float nextStringLfo()
    {
        stringLfoPhase += (2.0f * M_PI * stringLfoRate) / sampleRate;
        if (stringLfoPhase > 2.0f * M_PI)
            stringLfoPhase -= 2.0f * M_PI;

        // Sine wave between 0..1
        return 0.5f * (1.0f + sinf(stringLfoPhase));
    }

    float tinyStringReverb(float input)
    {
        // Simple feedback delay reverb — about 12000 samples ≈ 250ms at 48kHz
        static float delay[12000] = { 0 };
        static int pos = 0;

        // Read delayed sample
        float out = delay[pos];

        // Feedback: feed the input plus some of the old signal
        // delay[pos] = input + out * stringReverbFeedback;
        delay[pos] = input + out * (0.5f + stringReverb * 0.4f);

        // Increment and wrap buffer index
        pos = (pos + 1) % 12000;

        // Mix dry/wet with adjustable stringReverb parameter (0.0f – 1.0f)
        float dry = input * (1.0f - stringReverb);
        float wet = out * stringReverb;

        return dry + wet;
    }

public:
    // Tone
    DrumToneEngine tone;
    float toneVolume = 1.0f; // 0.00 to 1.00

    // Clap
    DrumClapEngine clap;
    float clapVolume = 0.2f;

    // String

    float stringVolume = 0.5f;
    float damping = 0.5f; // 0.0 to 1.0
    float stringDecay = 0.99f; // 0.80 to 0.99
    float stringToneLevel = 0.5f; // 0.0 to 1.0
    int8_t stringPitch = 0; // -36 to 36
    float stringPluckNoise = 0.5f; // 0.0 to 1.0
    float stringRingMod = 0.0f; // 0.0 to 1.0
    float stringReverb = 0.5f; // 0.0 to 1.0
    float stringReverbFeedback = 0.8f; // 0.0 to 1.0

    float stringLfoRate = 5.0f; // Hz (LFO speed)
    float stringLfoDepth = 0.2f; // 0.0–1.0 amplitude modulation depth

    // const static int sampleRate = 48000;
    const static int sampleRate = 44100;
    const static uint8_t channels = 2;

    static Audio* instance;
    static Audio& get()
    {
        if (!instance) {
            instance = new Audio();
        }
        return *instance;
    }

    float sample()
    {
        float sumVolume = toneVolume + clapVolume + stringVolume;
        if (sumVolume == 0.0f) {
            return 0.0f;
        }

        float out = 0.0f;
        if (toneVolume > 0.0f) {
            out += tone.sample() * toneVolume;
        }

        if (clapVolume > 0.0f) {
            out += clap.sample() * clapVolume;
        }

        if (stringVolume > 0.0f) {
            out += stringTone();
        }

        // add modulation that could turn into FM --> might use page switch on same button
        //       ----> maybe instead add a second layer that could act as FM modulation
        // add fx and f2 using page switch on same button
        //    ----> add master volume, and then could be 3 multi effect

        out = (out * velocity) / sumVolume;
        out = CLAMP(out, -1.0f, 1.0f);
        return out;
    }

    void noteOn(uint8_t note, float _velocity)
    {
        velocity = _velocity;

        tone.noteOn(note);
        clap.noteOn(note);

        stringNoteOn(note);
    }

    void noteOff(uint8_t note) { }
};

Audio* Audio::instance = NULL;
