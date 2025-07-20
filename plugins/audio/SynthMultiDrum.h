#pragma once

#include "audioPlugin.h"
#include "mapping.h"

#include "plugins/audio/utils/EnvelopDrumAmp.h"
#include "plugins/audio/utils/effects/applyWaveshape.h"
#include "plugins/audio/utils/effects/applyReverb.h"
#include "plugins/audio/utils/utils.h"

#include "log.h"

/*md
## SynthMultiDrum

Synth engine to generate multiple kind of drum sounds.
*/

class DrumEngine : public Mapping {
public:
    std::string name = "Engine";

    DrumEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, std::string name)
        : Mapping(props, config)
        , name(name)
    {
    }

    void sample(float* buf) override
    {
    }

    virtual void sampleOn(float* buf, float envAmp, int sampleCounter, int totalSamples) = 0;
    virtual void sampleOff(float* buf) { }
};

class MetalicDrumEngine : public DrumEngine {
protected:
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

    // Resonance simulation for body tone
    float resonator(float input, float freq, float decay, float& state)
    {
        state += freq / props.sampleRate;
        float output = input * expf(-decay * state) * sinf(2.0f * M_PI * freq * state);
        return output;
    }

    static constexpr int REVERB_BUFFER_SIZE = 48000; // 1 second buffer at 48kHz
    float reverbBuffer[REVERB_BUFFER_SIZE] = { 0.0f };
    int reverbIndex = 0;

public:
    Val& baseFreq = val(100.0f, "BASE_FREQ", { "Base Freq", .min = 10.0, .max = 400.0, .step = 1.0, .unit = "Hz" });
    Val& bodyResonance = val(0.8f, "RESONATOR", { "Resonator", .min = 0.00f, .max = 1.5f, .step = 0.01f, .floatingPoint = 2 });
    Val& timbre = val(5.0f, "TIMBRE", { "Timbre", .unit = "%" });
    Val& waveshape = val(0.0, "WAVESHAPE", { "Waveshape", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0, .step = 1.0, .unit = "%" });
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

        float output = applyWaveshape(tone, waveshape.pct()) * envAmp;
        output = applyReverb(output, reverb.pct(), reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE);
        buf[track] = output;

        phase += phaseIncrement;
    }
    void sampleOff(float* buf) override
    {
        buf[track] = applyReverb(buf[track], reverb.pct(), reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE);
    }

    uint8_t baseNote = 60;
    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        phase = 0.0f;
        resonatorState = 0.0f;
        noteFreq = baseFreq.get() * powf(2.0f, (note - baseNote) / 12.0f);
    }
};

class SynthMultiDrum : public Mapping {
protected:
    EnvelopDrumAmp envelopAmp;

    MetalicDrumEngine metalDrumEngine;

    DrumEngine *drumEngines[2] = {
        &metalDrumEngine,
        &metalDrumEngine,
    };
    DrumEngine *drumEngine = drumEngines[0];

public:
    /*md **Values**: */

    Val* values[10] = {
        &val(0.0f, "VAL_1", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[0]->set(p.val.get()); }),
        &val(0.0f, "VAL_2", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[1]->set(p.val.get()); }),
        &val(0.0f, "VAL_3", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[2]->set(p.val.get()); }),
        &val(0.0f, "VAL_4", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[3]->set(p.val.get()); }),
        &val(0.0f, "VAL_5", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[4]->set(p.val.get()); }),
        &val(0.0f, "VAL_6", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[5]->set(p.val.get()); }),
        &val(0.0f, "VAL_7", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[6]->set(p.val.get()); }),
        &val(0.0f, "VAL_8", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[7]->set(p.val.get()); }),
        &val(0.0f, "VAL_9", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[8]->set(p.val.get()); }),
        &val(0.0f, "VAL_10", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[9]->set(p.val.get()); }),
    };

    /*md - `ENGINE` select the drum engine. */
    Val& engine = val(0, "ENGINE", { "Engine", VALUE_STRING, .min = 0, .max = 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        drumEngine = drumEngines[(int)p.val.get()];
        p.val.setString(drumEngine->name);

        // loop through values and update their type
        for (int i = 0; i < 10 && i < drumEngine->mapping.size(); i++) {
            ValueInterface* val = drumEngine->mapping[i];
            values[i]->copy(val);
        }
    });
    
    /*md - `DURATION` controls the duration of the envelope. */
    Val& duration = val(500.0f, "DURATION", { "Duration", .min = 50.0, .max = 3000.0, .step = 10.0, .unit = "ms" });

    /*md - `AMP_MORPH` morph on the shape of the envelop of the amplitude.*/
    Val& ampMorph = val(0.0f, "AMP_MORPH", { "Amp. Morph", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        envelopAmp.morph(p.val.pct());
    });

    SynthMultiDrum(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , metalDrumEngine(props, config)
    {
        initValues();
    }

    int totalSamples = 0;
    int i = 0;
    // float phase = 0.0f;
    // float phaseIncrement = 0.0f;
    // float resonatorState = 0.0f;
    // float noteFreq = 440.0f;
    void sample(float* buf) override
    {
        if (i < totalSamples) {
            float envAmp = envelopAmp.next();
            drumEngine->sampleOn(buf, envAmp, i, totalSamples);
            i++;
        } else {
            // buf[track] = applyReverb(buf[track]);
            drumEngine->sampleOff(buf);
        }
    }

    uint8_t baseNote = 60;
    uint8_t currentNote = 0;

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        const float sampleRate = props.sampleRate;
        totalSamples = static_cast<int>(sampleRate * (duration.get() / 1000.0f));
        envelopAmp.reset(totalSamples);
        // printf("totalSamples: %d transientSamples: %d\n", totalSamples, transientSamples);
        // phase = 0.0f;
        // resonatorState = 0.0f;
        // noteFreq = baseFreq.get() * powf(2.0f, (note - baseNote) / 12.0f);
        i = 0;
        drumEngine->noteOn(note, _velocity);
    }

     DataFn dataFunctions[1] = {
        { "ENV_AMP_FORM", [this](void* userdata) {
             float* index = (float*)userdata;
             return (void*)envelopAmp.getMorphShape(*index);
         } },
    };
    DEFINE_GETDATAID_AND_DATA
};
