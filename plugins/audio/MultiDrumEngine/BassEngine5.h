#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/effects/applyBoost.h"
#include "plugins/audio/utils/effects/applyWaveshape.h"
#include "plugins/audio/utils/effects/applyCompression.h"
#include "plugins/audio/utils/effects/applyReverb.h"
#include "plugins/audio/utils/filterArray.h"

class BassEngine : public DrumEngine {
protected:
    static constexpr int REVERB_BUFFER_SIZE = 48000;
    float reverbBuffer[REVERB_BUFFER_SIZE] = { 0.0f };
    int reverbIndex = 0;

    EffectFilterArray<2> filter;

    float velocity = 1.0f;
    float phase = 0.0f;
    float freq = 100.0f;

    inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

    float morphOscillator(float morph)
    {
        float phaseNorm = fmodf(phase, 1.0f);

        // Basic shapes
        float sine = sinf(phaseNorm * 2.0f * M_PI);
        float tri = 4.f * fabsf(phaseNorm - 0.5f) - 1.f;
        float saw = 2.f * phaseNorm - 1.f;
        float square = phaseNorm < 0.5f ? 1.f : -1.f;
        float pulse = phaseNorm < (0.05f + 0.9f * morph) ? 1.f : -1.f;

        // Morph logic
        if (morph < 0.25f) {
            return lerp(sine, tri, morph / 0.25f);
        } else if (morph < 0.5f) {
            return lerp(tri, saw, (morph - 0.25f) / 0.25f);
        } else if (morph < 0.75f) {
            return lerp(saw, square, (morph - 0.5f) / 0.25f);
        } else {
            return lerp(square, pulse, (morph - 0.75f) / 0.25f);
        }
    }

    float applyEffects(float input)
    {
        float output = applyReverb(input, reverb.pct(), reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE);

        if (boost.pct() == 0.5f)
            return output;

        if (boost.pct() > 0.5f) {
            float amt = boost.pct() * 2.f - 1.f;
            return applyCompression(output, amt);
        }

        float amt = 1.f - boost.pct() * 2.f;
        return applyWaveshapeLut(output, amt, props.lookupTable);
    }

public:
    // === 10 PARAMETERS ===
    Val& pitch = val(0, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24, .max = 24, .skipJumpIncrements = true });
    Val& bend = val(0.4f, "BEND", { "Bend", .unit = "%" });
    Val& cutoff = val(50.0f, "CUTOFF", { "Cutoff", .unit = "%" }, [&](auto p) {
        float val = p.value;
        p.val.setFloat(val);
        filter.setCutoff(0.85f * p.val.pct() + 0.1f);
    });
    Val& resonance = val(0.0f, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        float val = p.value;
        p.val.setFloat(val);
        float res = 0.95f * (1.0f - powf(1.0f - p.val.pct(), 2));
        filter.setResonance(res);
    });
    Val& clipping = val(0.0f, "GAIN_CLIPPING", { "Gain Clipping", .unit = "%" });
    Val& boost = val(0.0f, "BOOST", { "Boost", VALUE_CENTERED, .min = -100.f, .max = 100.f, .step = 1.f, .unit = "%" });
    Val& reverb = val(0.3f, "REVERB", { "Reverb", .unit = "%" });
    Val& waveMorph = val(0.0f, "WAVE_MORPH", { "Wave Morph", .unit = "%" });
    Val& decay = val(0.5f, "DECAY", { "Decay", .unit = "%" });
    Val& punch = val(0.0f, "PUNCH", { "Punch", VALUE_CENTERED, .unit = "%" });

    BassEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Bass")
    {
        initValues();
    }

    void sampleOn(float* buf, float envAmp, int sc, int ts) override
    {
        float t = float(sc) / ts;
        float bAmt = bend.pct();
        float localFreq = freq * (1.0f - bAmt * t);

        phase += localFreq / props.sampleRate;
        if (phase > 1.0f)
            phase -= 1.0f;

        float osc = morphOscillator(waveMorph.pct());
        float out = osc * velocity * envAmp;

        filter.setCutoff(0.85f * cutoff.pct() * envAmp + 0.1f);
        filter.setSampleData(out, 0);
        filter.setSampleData(filter.lp[0], 1);
        out = filter.lp[1];

        out = range(out + out * clipping.pct() * 8.f, -1.f, 1.f);

        // Optional punch on attack
        float punchAmt = punch.pct() * 2.f - 1.f;
        if (fabsf(punchAmt) > 0.001f && t < 0.02f) {
            out *= (punchAmt > 0.f) ? 1.f + punchAmt * 2.f : 1.f - fabsf(punchAmt);
        }

        buf[track] = applyEffects(out);
    }

    void sampleOff(float* buf) override
    {
        buf[track] = applyReverb(buf[track], reverb.pct(), reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE);
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        velocity = _velocity;
        phase = 0.0f;
        freq = 55.f * powf(2.f, ((note + pitch.get() - 69) / 12.f)); // A4 = 440Hz
    }
};
