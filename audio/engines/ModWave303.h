#pragma once

#include "audio/engines/EngineBase.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

class ModWave303 : public EngineBase<ModWave303> {
public:
    // Declare exact parameter array size matching addParam calls
    Param params[10];

    Param& pitch = addParam({ .key = "pitch", .label = "Pitch", .value = 36.0f, .min = 24.0f, .max = 72.0f, .step = 1.0f });
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .value = 0.45f, .min = 0.02f, .max = 0.98f, .step = 0.01f });
    Param& resonance = addParam({ .key = "reso", .label = "Reso", .value = 0.70f, .min = 0.0f, .max = 0.98f, .step = 0.01f });
    Param& envAmt = addParam({ .key = "envAmt", .label = "Env Amt", .value = 0.60f, .min = 0.0f, .max = 1.0f, .step = 0.01f });
    Param& decayMs = addParam({ .key = "decay", .label = "Decay", .unit = "ms", .value = 300.0f, .min = 50.0f, .max = 2000.0f, .step = 10.0f });
    Param& accent = addParam({ .key = "accent", .label = "Accent", .value = 0.50f, .min = 0.0f, .max = 1.0f, .step = 0.01f });
    Param& drive = addParam({ .key = "drive", .label = "Drive", .value = 0.25f, .min = 0.0f, .max = 1.0f, .step = 0.01f });
    Param& waveMorph = addParam({ .key = "wave", .label = "Wave", .value = 0.0f, .min = 0.0f, .max = 1.0f, .step = 0.01f });
    Param& subLevel = addParam({ .key = "sub", .label = "Sub Lvl", .value = 0.30f, .min = 0.0f, .max = 1.0f, .step = 0.01f });
    Param& delaySend = addParam({ .key = "dlySend", .label = "Dly Send", .unit = "%", .value = 20.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    ModWave303(float sr = 44100.0f)
        : EngineBase(Synth, "Acid 303", params)
        , sampleRate(sr)
        , sampleRateInv(1.0f / sr)
    {
    }

    void setSampleRate(float sr)
    {
        sampleRate = sr;
        sampleRateInv = 1.0f / sr;
    }

    void trigger(float noteVal = -1.0f)
    {
        if (noteVal >= 0.0f) pitch.value = noteVal;
        freq = 440.0f * std::pow(2.0f, (pitch.value - 69.0f) / 12.0f);
        phaseInc = freq * sampleRateInv;
        ampEnv = 1.0f;
        filterEnv = 1.0f;
    }

    void noteOnImpl(uint8_t note, float velocity = 1.0f)
    {
        (void)velocity;
        trigger((float)note);
    }

    void noteOffImpl(uint8_t note)
    {
        (void)note;
    }

    float sampleImpl()
    {
        // Envelope decay
        float decaySec = std::max(0.050f, decayMs.value * 0.001f);
        float decayCoeff = std::exp(-1.0f / (decaySec * sampleRate));
        ampEnv *= decayCoeff;
        filterEnv *= decayCoeff;

        // Oscillator Generation: Saw -> Square with Sub Oscillator
        float p = phase;
        float saw = 2.0f * p - 1.0f - polyBlep(p, phaseInc);
        float sqr = (p < 0.5f ? 1.0f : -1.0f) + polyBlep(p, phaseInc) - polyBlep(std::fmod(p + 0.5f, 1.0f), phaseInc);
        float mainOsc = saw * (1.0f - waveMorph.value) + sqr * waveMorph.value;

        // Sub Oscillator (1 Octave down square wave)
        float pSub = std::fmod(phase * 0.5f, 1.0f);
        float subOsc = (pSub < 0.5f ? 1.0f : -1.0f) * subLevel.value;

        float oscVal = mainOsc + subOsc;

        phase += phaseInc;
        if (phase >= 1.0f) phase -= 1.0f;

        // Effective Cutoff with Env Mod & Accent
        float effCutoff = cutoff.value + filterEnv * envAmt.value * 0.45f * (1.0f + accent.value * 0.40f);
        effCutoff = std::clamp(effCutoff, 0.02f, 0.98f);

        // Chamberlin Resonant Filter with Squelchy Feedback
        float fCoeff = 0.012f + std::pow(effCutoff, 2.2f) * 0.988f;
        float resClamped = std::clamp(resonance.value, 0.0f, 0.98f);
        float fb = resClamped * 3.9f;

        float hp = oscVal - buf;
        float bp = buf - lp;
        float satBp = (fb > 0.001f) ? std::tanh(bp * 1.5f) : bp;
        buf += fCoeff * (hp + fb * satBp);
        lp += fCoeff * (buf - lp);

        float out = lp * ampEnv;

        // Tube Drive / Saturation Distortion
        if (drive.value > 0.01f) {
            float driveGain = 1.0f + drive.value * 4.5f;
            out = std::tanh(out * driveGain) / (1.0f + drive.value * 0.5f);
        }

        return out;
    }

    // Graph response curve for UI visualizer
    float drawImpl(float x)
    {
        float p = std::fmod(x, 1.0f);
        float effCutoff = std::clamp(cutoff.value + envAmt.value * 0.30f, 0.02f, 0.98f);
        float freqCenter = effCutoff;
        float resPeak = resonance.value * 1.5f;
        float dist = std::abs(p - freqCenter);
        float curve = std::exp(-dist * dist * 35.0f) * resPeak;
        float lpRoll = (p > freqCenter) ? std::exp(-(p - freqCenter) * 8.0f) : 1.0f;
        return std::clamp((lpRoll + curve) * 0.5f, 0.0f, 1.0f);
    }

private:
    float sampleRate = 44100.0f;
    float sampleRateInv = 1.0f / 44100.0f;

    float phase = 0.0f;
    float phaseInc = 0.01f;
    float freq = 110.0f;

    float ampEnv = 0.0f;
    float filterEnv = 0.0f;

    float buf = 0.0f;
    float lp = 0.0f;

    float polyBlep(float t, float dt)
    {
        if (t < dt) {
            t /= dt;
            return t + t - t * t - 1.0f;
        }
        if (t > 1.0f - dt) {
            t = (t - 1.0f) / dt;
            return t * t + t + t + 1.0f;
        }
        return 0.0f;
    }
};
