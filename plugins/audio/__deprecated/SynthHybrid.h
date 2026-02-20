/** Description:
This C++ header defines the blueprint for a sophisticated software audio generator, or "Hybrid Synthesizer."

The foundation of the design is the **Oscillator (Osc)**, which functions as a complete sound voice. It employs **Wavetable Synthesis**, allowing it to create rich and complex tones by cycling through pre-defined sound patterns (waveforms). It manages the loudness shape using an **ADSR Envelope** (Attack, Decay, Sustain, Release), defining precisely how the sound fades in and out when a note is played and released.

The main **SynthHybrid** unit combines two of these independent oscillators (`osc1` and `osc2`). Users can precisely control the mix ratio between the two voices and apply **Frequency Modulation (FM)**, where one oscillator dynamically alters the pitch of the other, enabling the creation of complex, metallic, or aggressive textures.

The final sound signal is then routed through two integrated effects:
1.  **Distortion:** Used to add warmth, saturation, or aggressive grit by intensely shaping the waveform.
2.  **Reverb:** A simple delay effect applied to the output that adds spaciousness and depth to simulate various environments.

The synthesizer works by continuously calculating tiny audio slices (samples) in real-time. It combines the output of the two tone generators based on their frequency and current envelope stage, applies the mix and FM settings, and finally processes the sound through the distortion and reverb effects before outputting the finished audio stream.

sha: b5959e6f2e98dac1a04a15f84dec29944469e378be966379d82fb35ba644b1ff 
*/
#pragma once

#include "./utils/Wavetable.h"
#include "audioPlugin.h"
#include "mapping.h"
#include "audio/AdsrEnvelop.h"

class Osc {
public:
    Val attack;
    Val decay;
    Val sustain;
    Val release;
    Val shape;
    Val morph;
    Val freq;
    AdsrEnvelop envelop;
    Wavetable wavetable;

    void setAttack(float ms)
    {
        attack.setFloat(ms);
        envelop.setAttack(attack.get());
    }

    void setDecay(float ms)
    {
        decay.setFloat(ms);
        envelop.setDecay(decay.get());
    }

    void setSustain(float v)
    {
        sustain.setFloat(v);
        envelop.setSustain(sustain.pct());
    }

    void setRelease(float ms)
    {
        release.setFloat(ms);
        envelop.setRelease(release.get());
    }

    void setShape(float value)
    {
        shape.setFloat(range(value, 1.0f, wavetable.fileBrowser.count));

        int position = shape.get();
        wavetable.open(position, false);
        shape.setString(wavetable.fileBrowser.getFileWithoutExtension(position));
    }

    void setMorph(float value)
    {
        morph.setFloat(range(value, 1.0f, ZIC_WAVETABLE_WAVEFORMS_COUNT));
        wavetable.morph((int)morph.get() - 1);
        morph.setString(std::to_string((int)morph.get()) + "/" + std::to_string(ZIC_WAVETABLE_WAVEFORMS_COUNT));
    }

    void noteOn()
    {
        envelop.reset();
        wavetable.sampleIndex = 0;
    }

    void noteOff()
    {
        envelop.release();
    }
};

class SynthHybrid : public Mapping {
    Osc osc1 = {
        { 50.0f, "ATTACK_1", { "Attack 1", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { osc1.setAttack(p.value); } },
        { 20.0f, "DECAY_1", { "Decay 1", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { osc1.setDecay(p.value); } },
        { 80.0f, "SUSTAIN_1", { "Sustain 1", .unit = "%" }, [&](auto p) { osc1.setSustain(p.value); } },
        { 50.0f, "RELEASE_1", { "Release 1", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { osc1.setRelease(p.value); } },
        { 0.0f, "SHAPE_1", { "SHAPE 1", VALUE_STRING, .max = 1000 }, [&](auto p) { osc1.setShape(p.value); } },
        { 0.0f, "MORPH_1", { "MORPH 1", VALUE_STRING }, [&](auto p) { osc1.setMorph(p.value); } },
        // TODO under 10 step 0.1 and over 10 step 1, over 100 step 10.
        { 200.0f, "FREQ_1", { "Freq. 1", .min = 0.1, .max = 8000.0, .step = 0.1, .unit = "Hz" }, [&](auto p) {
             osc1.freq.setFloat(p.value);
             if (osc1.freq.get() < 11.0f) {
                 osc1.freq.props().step = 0.1f;
             } else if (osc1.freq.get() > 80.0f) {
                 osc1.freq.props().step = 10.0f;
             } else {
                 osc1.freq.props().step = 1.0f;
             }
         } },
    };
    Osc osc2 = {
        { 50.0f, "ATTACK_2", { "Attack 2", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { osc2.setAttack(p.value); } },
        { 20.0f, "DECAY_2", { "Decay 2", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { osc2.setDecay(p.value); } },
        { 80.0f, "SUSTAIN_2", { "Sustain 2", .unit = "%" }, [&](auto p) { osc2.setSustain(p.value); } },
        { 50.0f, "RELEASE_2", { "Release 2", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { osc2.setRelease(p.value); } },
        { 0.0f, "SHAPE_2", { "SHAPE 2", VALUE_STRING, .max = 1000 }, [&](auto p) { osc2.setShape(p.value); } },
        { 0.0f, "MORPH_2", { "MORPH 2", VALUE_STRING }, [&](auto p) { osc2.setMorph(p.value); } },
        { 200.0f, "FREQ_2", { "Freq. 2", .min = 10.0, .max = 8000.0, .step = 10.0, .unit = "Hz" } },
    };

    float tanhLookup(float x)
    {
        x = CLAMP(x, -1.0f, 1.0f);
        int index = static_cast<int>((x + 1.0f) * 0.5f * (props.lookupTable->size - 1));
        return props.lookupTable->tanh[index];
    }
    float sineLookupInterpolated(float x)
    {
        x -= std::floor(x);
        return linearInterpolation(x, props.lookupTable->size, props.lookupTable->sine);
    }
    float applyDistortion(float input)
    {
        float amount = distortion.pct() * 2 - 1.0f;
        if (amount > 0.0f) {
            return tanhLookup(input * (1.0f + amount * 5.0f));

            // return std::pow(input, 1.0f - amount * 0.8f);
        }
        if (amount < 0.0f) {
            float sineValue = sineLookupInterpolated(input);
            return input + (-amount) * sineValue;
        }
        return input;
    }

    static constexpr int REVERB_BUFFER_SIZE = 48000; // 1 second buffer at 48kHz
    float reverbBuffer[REVERB_BUFFER_SIZE] = { 0.0f };
    int reverbIndex = 0;
    float applyReverb(float signal)
    {
        float reverbAmount = reverb.pct();
        if (reverbAmount == 0.0f) {
            return signal;
        }
        int reverbSamples = static_cast<int>((reverbAmount * 0.5f) * props.sampleRate); // Reverb duration scaled
        float feedback = reverbAmount * 0.7f; // Feedback scaled proportionally
        float mix = reverbAmount * 0.5f; // Mix scaled proportionally

        if (reverbSamples > REVERB_BUFFER_SIZE) {
            reverbSamples = REVERB_BUFFER_SIZE; // Cap the reverb duration to buffer size
        }

        float reverbSignal = reverbBuffer[reverbIndex];
        reverbBuffer[reverbIndex] = signal + reverbSignal * feedback;
        reverbIndex = (reverbIndex + 1) % reverbSamples;

        return signal * (1.0f - mix) + reverbSignal * mix;
    }

    int totalSamples = 0;
    int i = 0;

public:
    /*md **Values**: */

    /*md - `DISTORTION` to set distortion. */
    Val& distortion = val(0.0, "DISTORTION", { "Distortion", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0, .step = 1.0, .unit = "%" });
    /*md - REVERB controls delay time, feedback, and mix with one parameter. */
    Val& reverb = val(0.3f, "REVERB", { "Reverb", .unit = "%" });
    /*md - `OSC_MIX` to set oscillator substractive mix. */
    Val& oscMix = val(50.0f, "OSC_MIX", { "Osc1 Osc2", .type = VALUE_CENTERED, .unit = "%" });
    /*md - `FM_AMOUNT` to set FM amount. */
    Val& fmAmount = val(0.0, "FM_AMOUNT", { "FM Amount", .unit = "%" });
    // IDEA should there be a ratio for the fm frequency...

    SynthHybrid(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config, {
                                    // clang-format off
        &osc1.attack, &osc1.decay, &osc1.sustain, &osc1.release, &osc1.shape, &osc1.morph, &osc1.freq,
        &osc2.attack, &osc2.decay, &osc2.sustain, &osc2.release, &osc2.shape, &osc2.morph, &osc2.freq,
                                    // clang-format on
                                })
    {
        initValues();
    }

    float noteFrquency = 220.0f;
    void sample(float* buf) override
    {
        float env1 = osc1.envelop.next();
        float env2 = osc2.envelop.next();

        if (env1 || env2) {
            float freq1 = osc1.freq.get() * noteFactor * osc1.wavetable.sampleCount;
            float wave1 = osc1.wavetable.sample(&osc1.wavetable.sampleIndex, freq1) * env1;

            float freq2 = osc2.freq.get() * noteFactor * osc2.wavetable.sampleCount;
            float wave2 = osc2.wavetable.sample(&osc2.wavetable.sampleIndex, freq2 + freq2 * wave1 * fmAmount.pct()) * env2;

            float output = wave2 * oscMix.pct() + wave1 * (1.0f - oscMix.pct());

            output = applyDistortion(output);

            output = applyReverb(output);

            buf[track] = output;
            i++;
        } else {
            buf[track] = applyReverb(buf[track]);
        }
    }

    uint8_t baseNote = 60;
    uint8_t currentNote = 0;
    double noteFactor = 0.0;
    float currentVelocity = 1.0f;
    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        currentNote = note;
        currentVelocity = _velocity;

        // Precompute note step factor (frequency scaling per sample)
        // to check right frequency:https://onlinetonegenerator.com/
        noteFactor = (pow(2.0f, (note - baseNote) / 12.0f) / props.sampleRate) / props.channels;
        // printf("note %d noteFactor: %f\n", note, noteFactor);
        osc1.noteOn();
        osc2.noteOn();
    }

    void noteOff(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        osc1.noteOff();
        osc2.noteOff();
    }

    std::vector<float> waveformData;
    DataFn dataFunctions[2] = {
        { "WAVEFORM1", [this](void* userdata) {
             float* index = (float*)userdata;
             return osc1.wavetable.sample(index);
         } },
        { "WAVEFORM2", [this](void* userdata) {
             float* index = (float*)userdata;
             return osc2.wavetable.sample(index);
         } },
    };
    DEFINE_GETDATAID_AND_DATA
};
