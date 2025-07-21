#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/effects/applyBoost.h"
#include "plugins/audio/utils/effects/applyDrive.h"
#include "plugins/audio/utils/effects/applyReverb.h"

class BassEngine : public DrumEngine {
public:
    Val& pitch = val(45.f, "PITCH", { "Pitch", .min = 20.f, .max = 120.f, .step = 1.f, .unit = "Hz" });
    Val& bend = val(0.4f, "BEND", { "Bend", .unit = "%" });
    Val& resonance = val(0.6f, "RESONANCE", { "Resonance", .unit = "%" });
    Val& cutoff = val(0.3f, "CUTOFF", { "Cutoff", .unit = "%" });
    Val& squelch = val(0.0f, "SQUELCH", { "Squelch", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });

    Val& decay = val(0.25f, "DECAY", { "Decay", .floatingPoint = 2, .unit = "%", .incrementationType = VALUE_INCREMENTATION_MULT });
    Val& mix = val(0.5f, "MIX", { "Mix", .unit = "%" });

    Val& drive = val(0.0f, "DRIVE", { "Drive", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& reverb = val(0.1f, "REVERB", { "Reverb", .unit = "%" });
    Val& punch = val(0.3f, "PUNCH", { "Punch", .unit = "%" });

    BassEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Bass")
    {
        initValues();
    }

    static constexpr int REVBUF = 48000;
    float reverbBuf[REVBUF] = {};
    int rIdx = 0;

    float phase = 0.f;
    float filterState = 0.f;
    float prevInput = 0.f;
    float prevOutput = 0.f;

    void sampleOn(float* buf, float envAmp, int sc, int ts) override
    {
        float t = float(sc) / ts;
        float bendAmt = bend.pct();
        float freq = pitch.get() * (1.f - bendAmt * t);

        // Basic saw oscillator
        float osc = 2.f * (phase / (2.f * M_PI)) - 1.f;
        phase += 2.f * M_PI * freq / props.sampleRate;
        if (phase > 2.f * M_PI)
            phase -= 2.f * M_PI;

        // Envelope
        float amp = envAmp;

        // Filter
        float cutoffFreq = cutoff.pct() * 0.9f + 0.05f; // 0.05 – 0.95
        float res = resonance.pct() * 0.9f + 0.05f;

        float f = cutoffFreq;
        float q = res;

        float hp = osc - filterState;
        filterState += f * hp + q * (filterState - prevOutput);
        prevOutput = filterState;

        // Squelch shaping: bitcrush or fuzz distortion
        float sAmt = squelch.pct() * 2.f - 1.f;
        float shaped = filterState;

        if (sAmt > 0.f) {
            float crush = roundf(filterState * (1 + sAmt * 15)) / (1 + sAmt * 15);
            shaped = crush * (1.f - sAmt) + filterState * sAmt;
        } else if (sAmt < 0.f) {
            shaped = tanhf(filterState * (1.f + fabsf(sAmt) * 4.f));
        }

        // Decay the amplitude
        float decayFactor = powf(1.f - decay.pct(), t * 10.f);
        float out = shaped * amp * decayFactor;

        // Mix with raw osc
        out = out * mix.pct() + osc * (1.f - mix.pct());

        // Punch (transient burst)
        if (t < 0.02f) {
            float pAmt = punch.pct() * 2.f - 1.f;
            if (pAmt > 0.f)
                out *= 1.f + pAmt * 2.f;
            else
                out *= 1.f - fabsf(pAmt);
        }

        out = applyEffects(out);
        buf[track] = out;
    }

    void sampleOff(float* buf) override
    {
        buf[track] = applyReverb(buf[track], reverb.pct(), reverbBuf, rIdx, REVBUF);
    }

    void noteOn(uint8_t note, float, void* = nullptr) override
    {
        phase = 0.f;
        pitch.set(pitch.get() * powf(2.f, (note - 60) / 12.f));
        filterState = 0.f;
        prevOutput = 0.f;
    }

private:
    float applyEffects(float x)
    {
        float d = drive.pct() * 2.f - 1.f;
        if (d > 0.f) {
            x = applyDrive(x, d, props.lookupTable);
        } else {
            x = applyBoost(x, -d, prevInput, prevOutput);
        }
        return applyReverb(x, reverb.pct(), reverbBuf, rIdx, REVBUF);
    }
};
