#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/effects/applyBoost.h"
#include "plugins/audio/utils/effects/applyDrive.h"
#include "plugins/audio/utils/effects/applyReverb.h"

// TODO optimize this
// might want to use fastSine

class PercussionEngine : public DrumEngine {
protected:
    float velocity = 1.0f;

public:
    Val& pitch = val(120.f, "PITCH", { "Pitch", .min = 40.f, .max = 400.f, .step = 1.f, .unit = "Hz" });
    Val& bend = val(0.4f, "BEND", { "Bend freq.", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& harmonics = val(0.3f, "HARMONICS", { "Harmonics", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });

    Val& snareTune = val(200.f, "NOISE_TUNE", { "Noise Tune", .min = 80.f, .max = 600.f, .step = 1.f, .unit = "Hz" });
    Val& noiseCharacter = val(0.0f, "NOISE_SHAPE", { "Noise Shape", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& snareDecay = val(0.15f, "NOISE_DEC", { "Noise Dec", .floatingPoint = 2, .unit = "%", .incType = INC_MULT | INC_ONE_BY_ONE });
    Val& mix = val(0.5f, "MIX", { "Mix", .unit = "%" });

    Val& punch = val(0.6f, "PUNCH", { "Punch", .unit = "%" });
    Val& drive = val(0.f, "DRIVE", { "Drive", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& reverb = val(0.2f, "REVERB", { "Reverb", .unit = "%" });

    PercussionEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Perc")
    {
        initValues();
    }

    static constexpr int REVBUF = 48000;
    float reverbBuf[REVBUF] = {};
    int rIdx = 0;

    float phase = 0.f;

    float snareState = 0.f;

    void sampleOn(float* buf, float envAmp, int sc, int ts) override
    {
        const float t = float(sc) / ts;
        // const float bendEnv = 1.f - bend.pct() * t;
        float bendEnv = 1.f;
        float bendAmt = bend.pct() * 2.f - 1.f;
        if (bendAmt > 0.f) {
            // Downward bend: start high, go low
            bendEnv = 1.f - bendAmt * t;
        } else if (bendAmt < 0.f) {
            // Upward bend: start low, go high
            bendEnv = 1.f + (-bendAmt) * t;
        }
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
        buf[track] = applyReverb(buf[track], reverb.pct(), reverbBuf, rIdx, props.sampleRate, REVBUF);
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
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
        return applyReverb(x, reverb.pct(), reverbBuf, rIdx, props.sampleRate, REVBUF);
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