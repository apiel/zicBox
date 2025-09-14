#pragma once

#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/effects/applyCompression.h"
#include "plugins/audio/utils/effects/applyDrive.h"
#include "plugins/audio/utils/effects/applyReverb.h"

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

public:
    Val& baseFreq = val(100.0f, "BASE_FREQ", { "Base Freq", .min = 10.0, .max = 400.0, .step = 1.0, .unit = "Hz" });
    Val& bodyResonance = val(0.8f, "RESONATOR", { "Resonator", .min = 0.00f, .max = 1.5f, .step = 0.01f, .floatingPoint = 2 });
    Val& timbre = val(5.0f, "TIMBRE", { "Timbre", .unit = "%" });
    Val& drive = val(0.0, "DRIVE", { "Drive", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0, .step = 1.0, .unit = "%" });
    Val& toneDecay = val(0.02f, "TONE_DECAY", { "Tone Decay", .min = 0.005f, .max = 1.0f, .step = 0.005f, .floatingPoint = 2 });
    Val& reverb = val(0.3f, "REVERB", { "Reverb", .unit = "%" });
    Val& fmFreq = val(50.0f, "FM_FREQ", { "Fm. Freq.", .min = 0.1, .max = 500.0, .step = 0.1, .floatingPoint = 1, .unit = "Hz" });
    Val& fmAmp = val(0.0f, "FM_AMP", { "Fm. Amp.", .step = 0.1, .floatingPoint = 1, .unit = "%" });
    Val& envMod = val(0.0f, "ENV_MOD", { "Env. Mod.", .unit = "%" });
    Val& envShape = val(0.5f, "ENV_SHAPE", { "Env. Shape", .min = 0.1, .max = 5.0, .step = 0.1, .floatingPoint = 1 });

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
            float shape = envShape.get(); // Absolute shape parameter
            float envFactor;

            // Ensure the envelope always decays
            if (shape > 1.0f) {
                envFactor = pow(1.0f - t, shape); // Power curve decay
            } else if (shape > 0.0f) {
                envFactor = exp(-t * shape); // Exponential decay
            } else {
                envFactor = exp(-t); // Default exponential decay if shape is invalid
            }

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

    static constexpr int REVERB_BUFFER_SIZE = 48000; // 1 second buffer at 48kHz
    float reverbBuffer[REVERB_BUFFER_SIZE] = { 0.0f };
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
        output = applyReverb(output, reverb.pct(), reverbBuffer, reverbIndex, props.sampleRate, REVERB_BUFFER_SIZE);
        return output;
    }

    void sampleOff(float* buf) override
    {
        buf[track] = applyReverb(buf[track], reverb.pct(), reverbBuffer, reverbIndex, props.sampleRate, REVERB_BUFFER_SIZE);
    }

    uint8_t baseNote = 60;
    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        velocity = _velocity;
        phase = 0.0f;
        resonatorState = 0.0f;
        noteFreq = baseFreq.get() * powf(2.0f, (note - baseNote) / 12.0f);
    }
};
