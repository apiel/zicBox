/** Description:
This C++ header file defines the blueprint for `PercussionEngine`, an advanced digital sound generator specifically designed to synthesize percussion sounds, such as kicks, snares, or toms, within an audio processing framework.

This engine inherits functionality from a base `DrumEngine`, providing a complex structure for generating high-quality rhythmic tones.

**Core Functionality:**

The engine creates sound by mixing two primary components: a **Tonal Body** and a **Noise Component**.

1.  **Tonal Body Generation:** This controls the fundamental pitch of the drum. It includes a `Pitch Bend` feature that allows the frequency to quickly sweep up or down right after the sound starts, essential for generating classic tight drum sounds. It also uses `Harmonics` to enrich the tone, adding brightness or complexity to the sound wave.
2.  **Noise Component (Snare/Attack):** This generates texture, crucial for attack transients or snare-like sounds. The `Noise Character` control is key, allowing the engine to manipulate basic white noise. It can filter the noise to be smoother and darker (similar to "Pink Noise") or digitally degrade it using a technique similar to bitcrushing for a harsh, aggressive sound. This shaped noise is then run through a digital **resonator** to give it a defined pitch and natural decay.

**User Controls and Effects:**

The `PercussionEngine` provides numerous controls to shape the final output:

*   **Pitch & Bend:** Determines the base frequency and how that frequency sweeps over time.
*   **Mix:** Balances the volume between the synthesized Tonal Body and the Noise Component.
*   **Punch:** Adds a momentary volume boost at the very beginning of the note, simulating the impact of a real drum hit.
*   **Drive & Boost:** Applies distortion or saturation effects to add warmth or grit to the signal.
*   **Reverb:** Adds spatial ambience to the final sound.

In essence, this class defines a highly flexible instrument that mathematically generates a complex percussive sound, blends tones and custom-shaped noise, and applies real-time audio effects.

sha: 08c5ec425f2d4fb8519b1401d7c3c198a1510f7c4fa81312b43575895e4a86f4 
*/
#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/effects/applyBoost.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyReverb.h"

// TODO optimize this
// might want to use fastSine

class PercussionEngine : public DrumEngine {
protected:
    float velocity = 1.0f;

    float getBend(float t)
    {
        if (bend.pct() == 0.5f) {
            return 1.f;
        }
        float bendAmt = bend.pct() * 2.f - 1.f;
        if (bendAmt < 0.f) {
            return 1.f + bendAmt * (1.f - t);
        }
        return 1.f - bendAmt * t; // ramp down when positive
    }

public:
    Val& pitch = val(120.f, "PITCH", { .label = "Pitch", .min = 40.f, .max = 400.f, .step = 1.f, .unit = "Hz" });
    GraphPointFn bendGraph = [&](float index) { return getBend(index); };
    Val& bend = val(0.4f, "BEND", { .label = "Bend freq.", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%", .graph = bendGraph });
    Val& harmonics = val(0.3f, "HARMONICS", { .label = "Harmonics", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });

    Val& snareTune = val(200.f, "NOISE_TUNE", { .label = "Noise Tune", .min = 80.f, .max = 600.f, .step = 1.f, .unit = "Hz" });
    Val& noiseCharacter = val(0.0f, "NOISE_SHAPE", { .label = "Noise Shape", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& snareDecay = val(0.15f, "NOISE_DEC", { .label = "Noise Dec", .unit = "%", .incType = INC_MULT | INC_ONE_BY_ONE });
    Val& mix = val(0.5f, "MIX", { .label = "Mix", .unit = "%" });

    Val& punch = val(0.6f, "PUNCH", { .label = "Punch", .unit = "%" });
    Val& drive = val(0.f, "DRIVE", { .label = "Drive", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& reverb = val(0.2f, "REVERB", { .label = "Reverb", .unit = "%" });

    PercussionEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Perc")
    {
        initValues();
    }

    REVERB_BUFFER
    int rIdx = 0;

    float phase = 0.f;

    float snareState = 0.f;

    void sampleOn(float* buf, float envAmp, int sc, int ts) override
    {
        const float t = float(sc) / ts;
        // const float bendEnv = 1.f - bend.pct() * t;
        // float bendEnv = 1.f;
        // float bendAmt = bend.pct() * 2.f - 1.f;
        // if (bendAmt > 0.f) {
        //     // Downward bend: start high, go low
        //     bendEnv = 1.f - bendAmt * t;
        // } else if (bendAmt < 0.f) {
        //     // Upward bend: start low, go high
        //     bendEnv = 1.f + (-bendAmt) * t;
        // }
        float bendEnv = getBend(t);
        const float freq = pitch.get() * bendEnv;

        float out = 0.f;
        float harmonicsAmt = harmonics.pct() * 2.f - 1.f;
        if (harmonicsAmt >= 0.f) {
            // --- Fundamental + 2 harmonics
            for (int h = 1; h <= 3; ++h) {
                float amp = (h == 1) ? 1.f : (harmonicsAmt / h);
                out += amp * sinf(phase * h);
            }
        } else {
            out = sinf(phase) + harmonicsAmt * sinf(phase * 3.f); // Folded
        }
        phase += 2.f * M_PI * freq / props.sampleRate;
        out *= envAmp;

        float noise = (customNoise(whiteNoise()) * 2.f - 1.f) * envAmp;
        float snare = resonator(noise, snareTune.get(), snareDecay.pct(), snareState);
        out = out * (1.f - mix.pct()) + snare * mix.pct();

        // --- Punch transient
        if (t < 0.02f) {
            out *= 1.f + punch.pct() * 2.f;
        }

        out = out * velocity;
        out = applyEffects(out);
        buf[track] = out;
    }

    void sampleOff(float* buf) override
    {
        buf[track] = applyReverb(buf[track], reverb.pct(), buffer, rIdx);
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        phase = 0.f;
        pitch.set(pitch.get() * powf(2.f, (note - 60) / 12.f));
        snareState = 0.f;
    }

private:
    float prevInput = 0.0f;
    float prevOutput = 0.0f;
    float applyEffects(float x)
    {
        float d = drive.pct() * 2.f - 1.f;
        if (d > 0.f) {
            x = applyDrive(x, d, props.lookupTable);
        } else {
            x = applyBoost(x, -d, prevInput, prevOutput);
        }
        return applyReverb(x, reverb.pct(), buffer, rIdx);
    }

    float whiteNoise()
    {
        return props.lookupTable->getNoise();
    }

    // For Pink noise, make encoder for pink noise amount...
    // Pink noise approximation (simple filter-based approach)
    float pinkNoiseState = 0.0f;
    float crushedNoise = 0.f;
    int bitcrushCounter = 0;
    float customNoise(float input)
    {
        float amt = noiseCharacter.pct() * 2.f - 1.f; // [-1.0, 1.0]

        if (amt < 0.f) {
            // Pink noise (low-pass)
            float blend = -amt;
            pinkNoiseState = blend * pinkNoiseState + (1.f - blend) * input;
            return pinkNoiseState;
        } else if (amt > 0.f) {
            // Bitcrusher

            // Shape amt to be more aggressive earlier (e.g., use sqrt or curve)
            float shapedAmt = sqrtf(amt); // More audible changes at low values

            float depth = 1.f - shapedAmt; // Invert for resolution
            int resolution = int(256 * depth);
            if (resolution < 2)
                resolution = 2;

            int crushRate = int(8 + (1.f - depth) * 120); // 8 to 128 samples
            if (++bitcrushCounter >= crushRate) {
                bitcrushCounter = 0;
                crushedNoise = roundf(input * resolution) / resolution;
            }

            // Mix more effect in sooner
            float mix = shapedAmt; // 0 → 1 as amt → 1, but faster
            return crushedNoise * mix + input * (1.f - mix);
        }

        return input;
    }

    float resonator(float input, float frequency, float decay, float& time)
    {
        // Advance time in seconds
        time += frequency / props.sampleRate;

        // Exponentially decaying sine wave, modulated by input
        float envelope = expf(-decay * time);
        float sineWave = sinf(2.f * M_PI * frequency * time);

        return input * envelope * sineWave;
    }
};