/** Description:
This C++ header defines the blueprint for an advanced audio component called the `MetalicDrumEngine`. This engine is a specialized sound generator designed to synthesize complex, metallic, and resonant drum sounds, such as cymbals, bells, or deep metallic kicks, rather than using recorded audio samples.

### How the Engine Works

The core functionality relies on mathematical synthesis methods to create its signature tone:

1.  **Complex Tone Generation (FM):** The engine uses **Frequency Modulation (FM)** to generate a complex foundational tone. This technique involves using one oscillator (a digital wave creator) to rapidly change the frequency of another, which is crucial for achieving harmonically rich, non-standard sounds typical of metallic objects.
2.  **Resonance and Decay:** The sound is then processed by a **Resonator** component. This digital filter mimics the way a physical object (like a drum shell or metal plate) vibrates and rings, giving the tone its "body" and sustain.
3.  **Envelope Control:** The shape of the sound, from its sharp attack to its smooth decay, is precisely controlled by adjustable mathematical functions, allowing for anything from a short click to a long, ringing tone.

### User Controls and Effects

The engine provides numerous adjustable parameters to shape the sound:

*   **Tonal Control:** Parameters like **Base Frequency**, **FM Frequency/Amplitude**, and **Body Resonance** allow users to dial in the fundamental pitch and how long the sound rings out.
*   **Timbre:** Controls the complexity and harmonic content of the tone.
*   **Integrated Effects:** To finalize the output, the engine includes built-in effects processing for **Drive** (distortion/saturation) or **Compression** for punch, and **Reverb** to add spatial depth.

In essence, the `MetalicDrumEngine` is a complete, self-contained instrument focused on recreating and manipulating synthetic metallic percussion through sophisticated mathematical modeling.

sha: 0259455386f6b5bd45a25cb365b0f0a15f51271c0f27e44047fd13f30609ccb2 
*/
#pragma once

#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyReverb.h"

class MetalicDrumEngine : public DrumEngine {
protected:
    float velocity = 1.0f;

    // Sine wave oscillator
    float sineWave(float frequency, float phase)
    {
        return sinf(2.0f * M_PI * frequency * phase);
        // return fastSin(2.0f * M_PI * frequency * phase);
    }

    float fmModulation(float freq, float phase)
    {
        float fmAmplitude = fmAmp.pct();
        float modulator = fmAmplitude > 0.0f ? sineWave(fmFreq.get(), phase) * fmAmplitude * 10.0f : 0.0f;
        return sineWave(freq + modulator, phase);
    }

    float resonator(float input, float freq, float decay, float& state)
    {
        state += 1.0f / props.sampleRate; // time in seconds
        float output = input * expf(-decay * state) * sinf(2.0f * M_PI * freq * state);

        // Optional: loudness compensation
        float compensation = sqrtf(220.0f / std::max(freq, 1.0f));
        output *= compensation;

        return output;
    }

    float getShape(float t)
    {
        float shape = envShape.get(); // Absolute shape parameter
        if (shape > 1.0f) {
            return pow(1.0f - t, shape); // Power curve decay
        } else if (shape > 0.0f) {
            return exp(-t * shape); // Exponential decay
        } else {
            return exp(-t); // Default exponential decay if shape is invalid
        }
    }

public:
    Val& baseFreq = val(100.0f, "BASE_FREQ", { .label = "Base Freq", .min = 10.0, .max = 400.0, .step = 1.0, .unit = "Hz" });
    Val& bodyResonance = val(0.8f, "RESONATOR", { .label = "Resonator", .min = 0.00f, .max = 1.5f, .step = 0.01f });
    Val& timbre = val(5.0f, "TIMBRE", { .label = "Timbre", .unit = "%" });
    Val& drive = val(0.0, "DRIVE", { .label = "Drive", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0, .step = 1.0, .unit = "%" });
    Val& toneDecay = val(0.02f, "TONE_DECAY", { .label = "Tone Decay", .min = 0.005f, .max = 1.0f, .step = 0.005f });
    Val& reverb = val(0.3f, "REVERB", { .label = "Reverb", .unit = "%" });
    Val& fmFreq = val(50.0f, "FM_FREQ", { .label = "Fm. Freq.", .min = 0.1, .max = 500.0, .step = 0.1, .unit = "Hz" });
    Val& fmAmp = val(0.0f, "FM_AMP", { .label = "Fm. Amp.", .step = 0.1, .unit = "%" });
    Val& envMod = val(0.0f, "ENV_MOD", { .label = "Env. Mod.", .unit = "%" });

    GraphPointFn envGraph = [&](float index) { return getShape(index); };
    Val& envShape = val(0.5f, "ENV_SHAPE", { .label = "Env. Shape", .min = 0.1, .max = 5.0, .step = 0.1, .graph = envGraph });

    MetalicDrumEngine(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : DrumEngine(props, config, "Metalic")
    {
        initValues();
        phaseIncrement = 1.0f / props.sampleRate;
    }

    float phase = 0.0f;
    float phaseIncrement = 0.0f;
    float resonatorState = 0.0f;
    float noteFreq = 440.0f;
    void sampleOn(float* buf, float envAmp, int sampleCounter, int totalSamples) override
    {
        float t = (float)sampleCounter / totalSamples; // Time normalized to [0, 1]
        float tone = 0.0f;

        // https://codesandbox.io/p/sandbox/green-platform-tzl4pn?file=%2Fsrc%2Findex.js%3A18%2C13
        float freq = noteFreq;
        if (envMod.pct() > 0.0f) {
            // TODO use EnvelopDrumAmp instead...
            float envFactor = getShape(t);
            freq = freq + freq * envMod.pct() * envFactor;
        }
        // Tonal component with resonance
        // tone = sineWave(freq, phase);
        tone = fmModulation(freq, phase);
        if (bodyResonance.get() > 0.0f) {
            tone = resonator(tone * envAmp, freq * bodyResonance.get(), toneDecay.get(), resonatorState);
        }

        if (timbre.pct() > 0.0f) {
            // Adjust timbre by filtering harmonics dynamically
            tone *= (1.0f - timbre.pct()) + timbre.pct() * sinf(2.0f * M_PI * freq * 0.5f * t);
        }

        float output = tone * envAmp * velocity;
        output = applyEffect(output);
        buf[track] = output;

        phase += phaseIncrement;
    }

    REVERB_BUFFER
    int reverbIndex = 0;
    float prevInput = 0.0f;
    float prevOutput = 0.0f;
    float applyEffect(float input)
    {
        float output = input;
        float amount = drive.pct() * 2 - 1.0f;
        if (amount > 0.0f) {
            // output = applyBoost(input, amount, prevInput, prevOutput);
            output = applyCompression(input, amount);
        } else if (amount < 0.0f) {
            output = applyDrive(input, -amount, props.lookupTable);
        }
        output = applyReverb(output, reverb.pct(), buffer, reverbIndex);
        return output;
    }

    void sampleOff(float* buf) override
    {
        buf[track] = applyReverb(buf[track], reverb.pct(), buffer, reverbIndex);
    }

    uint8_t baseNote = 60;
    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        phase = 0.0f;
        resonatorState = 0.0f;
        noteFreq = baseFreq.get() * powf(2.0f, (note - baseNote) / 12.0f);
    }
};
