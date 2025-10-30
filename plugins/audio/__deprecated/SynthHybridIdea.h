#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "audio/utils.h"

class SynthHybrid : public Mapping {
protected:
public:
    // Oscillator 1 (Subtractive)
    Val& osc1Waveform = val(0.0f, "OSC1_WAVE", { "Osc 1 Waveform", .min = 0.0, .max = 3.0, .step = 1.0, .unit = "" }); // 0 = Sine, 1 = Square, 2 = Saw, 3 = Triangle
    Val& osc1Pitch = val(440.0f, "OSC1_PITCH", { "Osc 1 Pitch", .min = 20.0, .max = 2000.0, .step = 1.0, .unit = "Hz" });

    // Oscillator 2 (FM Modulation)
    Val& osc2Waveform = val(0.0f, "OSC2_WAVE", { "Osc 2 Waveform", .min = 0.0, .max = 3.0, .step = 1.0, .unit = "" }); // 0 = Sine, 1 = Square, 2 = Saw, 3 = Triangle
    Val& osc2PitchRatio = val(1.0f, "OSC2_RATIO", { "Osc 2 Ratio", .min = 0.1, .max = 10.0, .step = 0.1, .unit = "" }); // Ratio relative to Osc 1
    Val& fmDepth = val(0.0f, "FM_DEPTH", { "FM Depth", .min = 0.0, .max = 1.0, .step = 0.01, .unit = "" }); // FM modulation depth

    // Filter
    Val& filterCutoff = val(1000.0f, "FILTER_CUT", { "Filter Cutoff", .min = 20.0, .max = 20000.0, .step = 10.0, .unit = "Hz" });
    Val& filterResonance = val(0.5f, "FILTER_RES", { "Filter Resonance", .min = 0.0, .max = 1.0, .step = 0.01, .unit = "" });

    // Amplitude Envelope (ADSR)
    Val& attack = val(0.01f, "ATTACK", { "Attack", .min = 0.01, .max = 1.0, .step = 0.01, .unit = "s" });
    Val& decay = val(0.1f, "DECAY", { "Decay", .min = 0.01, .max = 1.0, .step = 0.01, .unit = "s" });
    Val& sustain = val(0.7f, "SUSTAIN", { "Sustain", .min = 0.0, .max = 1.0, .step = 0.01, .unit = "" });
    Val& release = val(0.2f, "RELEASE", { "Release", .min = 0.01, .max = 1.0, .step = 0.01, .unit = "s" });

    // Internal state
    float phase1 = 0.0f; // Oscillator 1 phase
    float phase2 = 0.0f; // Oscillator 2 phase
    float envelopeLevel = 0.0f; // Current envelope level
    bool isNoteOn = false; // Note state

    SynthHybrid(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    }

    // Generate a waveform sample
    float generateWaveform(float phase, int waveform)
    {
        switch (waveform) {
        case 0:
            return sinf(phase * 2.0f * M_PI); // Sine
        case 1:
            return (fmodf(phase, 1.0f) < 0.5f) ? 1.0f : -1.0f; // Square
        case 2:
            return 2.0f * (phase - floorf(phase + 0.5f)); // Saw
        case 3:
            return 4.0f * fabsf(phase - floorf(phase + 0.75f) + 0.25f) - 1.0f; // Triangle
        default:
            return 0.0f;
        }
    }

    // Process the amplitude envelope
    float processEnvelope()
    {
        static float env = 0.0f;
        if (isNoteOn) {
            if (env < 1.0f) {
                env += 1.0f / (attack.get() * props.sampleRate); // Attack
            } else {
                env = sustain.get(); // Sustain
            }
        } else {
            env -= 1.0f / (release.get() * props.sampleRate); // Release
            if (env < 0.0f)
                env = 0.0f;
        }
        return env;
    }

    float freq1 = 0.0f;
    void sample(float* buf) override
    {
        // Calculate oscillator frequencies
        float freq2 = freq1 * osc2PitchRatio.get();

        // Update oscillator phases
        phase1 += freq1 / props.sampleRate;
        phase2 += freq2 / props.sampleRate;
        if (phase1 >= 1.0f)
            phase1 -= 1.0f;
        if (phase2 >= 1.0f)
            phase2 -= 1.0f;

        // Generate waveforms
        float osc1Out = generateWaveform(phase1, osc1Waveform.get()); // Subtractive oscillator
        float modOut = generateWaveform(phase2, osc2Waveform.get()); // FM modulator

        // Apply FM modulation to the subtractive oscillator
        float fmPhaseOffset = fmDepth.get() * modOut; // Modulation index * modulator output
        float fmOut = generateWaveform(phase1 + fmPhaseOffset, osc1Waveform.get());

        // Blend subtractive and FM signals (50/50 mix)
        float oscOut = 0.5f * osc1Out + 0.5f * fmOut;

        // Apply filter (simple low-pass)
        static float filterState = 0.0f;
        float cutoff = filterCutoff.get() / props.sampleRate;
        float resonance = filterResonance.get();
        filterState += cutoff * (oscOut - filterState);
        oscOut = filterState;

        // Apply envelope
        envelopeLevel = processEnvelope();
        oscOut *= envelopeLevel;

        // Output to buffer
        buf[track] = oscOut;
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        isNoteOn = true;
        freq1 = osc1Pitch.get() * powf(2.0f, (note - 69) / 12.0f); // Convert MIDI note to frequency
    }

    void noteOff(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        isNoteOn = false;
    }
};
