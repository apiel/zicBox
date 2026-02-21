/** Description:
This header file defines a specialized digital audio synthesizer named `AlienFreakEngine`. Its primary purpose is to generate unusual, experimental, and glitch-oriented sounds within a larger audio software framework.

**How the Engine Works:**

The core of the sound generation relies on three main sound sources, known as oscillators. Unlike traditional synthesizers that might use harmonious tuning, this engine intentionally employs "inharmonic ratios." This means the three oscillators are tuned to mathematically complex frequencies that clash slightly, creating a dissonant and metallic or alien sound texture.

**Key Sound Shaping Mechanisms:**

1.  **Modulation (LFO):** A built-in, slow-moving internal cycle (Low-Frequency Oscillator or LFO) is used to continuously shift the pitch of the sound sources. This adds subtle or extreme movement and wobble to the sound over time.
2.  **Glitch Generation:** The engine features a unique "glitch" setting. Based on the user’s control, the frequency of the oscillators is randomly and abruptly reset or altered during playback. This mechanism generates the unpredictable, broken, or artifact-heavy textures the engine is known for.
3.  **Detune and Noise:** Further complexity is added through controls for fine-tuning the pitch separation between the three sources (detune) and mixing in layers of digital noise.

**User Control:**

The code exposes ten crucial parameters that allow a user to sculpt the sound, including setting the musical pitch, adjusting the intensity of the inharmonic ratios, controlling the speed and depth of the LFO, and dialing in the amount of glitch.

Finally, the generated raw sound passes through an integrated Multi-Effects unit (`MultiFx`) before the final audio output is calculated, allowing the user to add final layers of processing like delay or reverb.

sha: f0e346bb3f0894d53e4ccbee160079fc50f8a2f1bf42219f3dad0dc281568234 
*/
#pragma once

#include "plugins/audio/MultiEngine/Engine.h"
#include "plugins/audio/utils/valMultiFx.h"
#include "helpers/math.h"
#include <cmath>
#include <cstdlib>

// FIXME

class AlienFreakEngine : public Engine {
protected:
    static constexpr int NUM_OSC = 3;

    MultiFx multiFx;

    float baseFreq = 100.0f;
    float velocity = 1.0f;

    float oscPhases[NUM_OSC] = {0.0f};
    float oscRatios[NUM_OSC] = {1.0f, 2.3f, 3.7f}; // inharmonic ratios
    float glitchState[NUM_OSC] = {0.0f};

    // LFO for sample-and-hold modulation
    float lfoPhase = 0.0f;
    float lfoRateHz = 1.0f;
    float lfoDepth = 0.0f;

    float noiseLow = 0.0f;
    float noiseHigh = 0.0f;
    float detune = 0.0f;
    float glitchAmount = 0.0f;

    float sampleOsc(int idx, float freq) {
        float phaseInc = 2.0f * M_PI * freq / props.sampleRate;
        oscPhases[idx] += phaseInc;
        if (oscPhases[idx] > 2.0f * M_PI) oscPhases[idx] -= 2.0f * M_PI;

        return fastSin2(oscPhases[idx]);
    }

public:
    // --- 10 parameters ---
    Val& basePitch = val(0.0f, "BASE_PITCH", {"Base Pitch", VALUE_CENTERED, .min=-24, .max=24}, [&](auto p){
        p.val.setFloat(p.value);
        baseFreq = 220.0f * powf(2.0f, p.val.get() / 12.0f);
    });

    Val& inharm = val(50.0f, "INHARM", {"Inharmonics", .unit="%"}, [&](auto p){
        float pct = p.val.pct();
        oscRatios[1] = 2.0f + pct * 2.0f; // 2..4.0
        oscRatios[2] = 3.0f + pct * 2.0f; // 3..5.0
    });

    Val& glitch = val(50.0f, "GLITCH", {"Glitch", .unit="%"}, [&](auto p){
        glitchAmount = p.val.pct();
    });

    Val& noiseLowParam = val(25.0f, "NOISE_LOW", {"Noise Low", .unit="%"}, [&](auto p){
        noiseLow = p.val.pct();
    });

    Val& noiseHighParam = val(25.0f, "NOISE_HIGH", {"Noise High", .unit="%"}, [&](auto p){
        noiseHigh = p.val.pct();
    });

    Val& lfoRateParam = val(1.0f, "LFO_RATE", {"LFO Rate", .min=0.1f, .max=20.0f, .unit="Hz"}, [&](auto p){
        lfoRateHz = p.value;
    });

    Val& lfoDepthParam = val(50.0f, "LFO_DEPTH", {"LFO Depth", .unit="%"}, [&](auto p){
        lfoDepth = p.val.pct();
    });

    Val& detuneParam = val(10.0f, "DETUNE", {"Detune", .unit="%"}, [&](auto p){
        detune = p.val.pct() * 0.05f;
    });

    Val& fxType = val(0, "FX_TYPE", {"FX Type", VALUE_STRING, .max = MultiFx::FX_COUNT - 1 }, valMultiFx(multiFx));

    Val& fxAmount = val(50.0f, "FX_AMOUNT", {"FX Amount", .unit="%"});

    // --- constructor ---
    AlienFreakEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "AlienFreak")
        , multiFx(props.sampleRate)
    {
        initValues();
    }

    void sample(float* buf, float envAmpVal) override {
        if(envAmpVal == 0.0f){
            float out = buf[track];
            out = multiFx.apply(out, fxAmount.pct());
            buf[track] = out;
            return;
        }

        // --- LFO update ---
        float lfoInc = 2.0f * M_PI * lfoRateHz / props.sampleRate;
        lfoPhase += lfoInc;
        if(lfoPhase > 2.0f * M_PI) lfoPhase -= 2.0f * M_PI;
        float lfoVal = fastSin2(lfoPhase) * lfoDepth; // ±LFO depth

        float sampleSum = 0.0f;
        for(int i=0;i<NUM_OSC;i++){
            float freq = baseFreq * oscRatios[i];
            // add detune
            freq *= 1.0f + ((float)i - NUM_OSC/2.0f)*detune;
            // apply LFO to pitch
            freq *= 1.0f + lfoVal*0.02f;
            // apply glitch randomly
            if(((float)rand()/RAND_MAX) < glitchAmount*0.01f){
                freq *= 0.5f + (float)rand()/RAND_MAX;
            }
            sampleSum += sampleOsc(i,freq);
        }

        float out = sampleSum / NUM_OSC;

        // // noise layers
        // float nLow = props.lookupTable->getNoise() * noiseLow;
        // float nHigh = props.lookupTable->getNoise() * noiseHigh;
        // out = out * (1.0f - noiseLow - noiseHigh) + nLow + nHigh;

        out *= envAmpVal * velocity;
        out = multiFx.apply(out, fxAmount.pct());

        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        baseFreq = 220.0f * powf(2.0f,(note-60)/12.0f);
        for(int i=0;i<NUM_OSC;i++){
            oscPhases[i] = 0.0f;
        }
        lfoPhase = 0.0f;
    }
};
