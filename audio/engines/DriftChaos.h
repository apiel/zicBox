#pragma once

#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

class DriftChaos : public EngineBase<DriftChaos> {
private:
    float sampleRate = 44100.0f;
    float sampleRateDiv = 1.0f / 44100.0f;

    // State Variables
    float velocity = 1.0f;
    uint8_t currentNote = 48;
    float currentFreq = 130.81f; // C3

    // Envelopes & LFO
    float ampEnv = 0.0f;
    float pitchEnvState = 0.0f;
    float lfoPhase = 0.0f;

    // Oscillators & Phases
    float phase = 0.0f;
    float ringPhase = 0.0f;
    uint32_t cycleCounter = 0;

    // Lorenz Chaotic Attractor State
    float lx = 0.1f;
    float ly = 0.0f;
    float lz = 0.0f;

    // Feedback Loop Memory
    float lastOutput = 0.0f;

    // SVF Filter
    FilterSVF filter;

    // Fast Noise LCG
    uint32_t noiseState = 999123;
    float nextNoise()
    {
        noiseState = noiseState * 196314165 + 907633389;
        return (float)int32_t(noiseState) / 2147483648.f;
    }

    float lerp(float a, float b, float t) { return a + t * (b - a); }

    // Wavefolder DSP helper
    float wavefold(float input, float foldAmt)
    {
        if (foldAmt <= 0.001f) return input;
        float gain = 1.0f + foldAmt * 5.0f;
        float driven = input * gain;
        // Multi-stage sine fold
        return Math::fastSin2(driven * 1.5707963f);
    }

public:
    Param params[12];

    // Page 1: Chaotic Core & Pitch
    Param& pitch = addParam({ .key = "pitch", .label = "Pitch", .unit = "", .value = 48.0f, .min = 24.0f, .max = 96.0f, .step = 1.0f });
    Param& chaos = addParam({ .key = "chaos", .label = "Chaos", .unit = "%", .value = 35.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& feedback = addParam({ .key = "feedback", .label = "Feedback", .unit = "%", .value = 30.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& decay = addParam({ .key = "decay", .label = "Decay", .unit = "ms", .value = 400.0f, .min = 10.0f, .max = 4000.0f, .step = 10.0f });

    // Page 2: Circuit-Bend & Wavefolding
    Param& fold = addParam({ .key = "fold", .label = "Wavefold", .unit = "%", .value = 25.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& subHarm = addParam({ .key = "subHarm", .label = "Sub-Harm", .unit = "%", .value = 20.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& ringMod = addParam({ .key = "ringMod", .label = "Ring Mod", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& color = addParam({ .key = "color", .label = "Color", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    // Page 3: Dynamics, Modulation & Space
    Param& pitchGlitch = addParam({ .key = "pitchGlitch", .label = "PitchGlitch", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f });
    Param& lfoSpeed = addParam({ .key = "lfoSpeed", .label = "LFO Speed", .unit = "%", .value = 40.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& lfoDepth = addParam({ .key = "lfoDepth", .label = "LFO Depth", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& delaySend = addParam({ .key = "delaySend", .label = "Dly Send", .unit = "%", .value = 20.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    DriftChaos(float sr = 44100.0f)
        : EngineBase(Synth, "DriftChaos", params)
        , sampleRate(sr)
    {
        sampleRateDiv = 1.0f / sampleRate;
    }

    void trigger(float vel = 1.0f)
    {
        noteOnImpl(static_cast<uint8_t>(std::clamp(pitch.value, 24.0f, 96.0f)), vel);
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        velocity = vel;
        currentNote = note;

        // Trigger envelopes
        ampEnv = 1.0f;
        pitchEnvState = 1.0f;

        // Perturb Lorenz chaotic attractor on trigger for non-repeating hit timbre
        lx = 0.1f + nextNoise() * 0.5f;
        ly = nextNoise() * 0.5f;
        lz = 0.1f;
    }

    void noteOffImpl(uint8_t note)
    {
        ampEnv *= 0.5f;
    }

    float sampleImpl()
    {
        if (ampEnv < 0.0001f) {
            ampEnv = 0.0f;
            return 0.0f;
        }

        // --- 1. LFO & Modulation ---
        float speedHz = lerp(0.1f, 25.0f, lfoSpeed.value * 0.01f);
        lfoPhase += speedHz * sampleRateDiv;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
        float lfoVal = Math::fastSin2(PI_X2 * lfoPhase);
        float modAmt = (lfoDepth.value * 0.01f) * lfoVal;

        // --- 2. Lorenz Chaotic Attractor Step ---
        float chVal = chaos.value * 0.01f;
        if (chVal > 0.01f) {
            float dt = (0.002f + chVal * 0.012f);
            float sigma = 10.0f;
            float rho = 28.0f;
            float beta = 8.0f / 3.0f;

            float dx = sigma * (ly - lx);
            float dy = lx * (rho - lz) - ly;
            float dz = lx * ly - beta * lz;

            lx += dx * dt;
            ly += dy * dt;
            lz += dz * dt;

            // Clamp to avoid numerical explosion
            lx = std::clamp(lx, -30.0f, 30.0f);
            ly = std::clamp(ly, -30.0f, 30.0f);
            lz = std::clamp(lz, 0.0f, 50.0f);
        }
        float chaosSignal = std::clamp(lx * 0.05f, -1.0f, 1.0f);

        // --- 3. Pitch Envelope & Glitch Calculation ---
        float decayMs = std::clamp(decay.value, 10.0f, 4000.0f);
        float ampDecayRate = std::exp(-1.0f / (decayMs * 0.001f * sampleRate));
        ampEnv *= ampDecayRate;

        float pEnvDecayRate = std::exp(-1.0f / (0.05f * sampleRate)); // 50ms pitch sweep
        pitchEnvState *= pEnvDecayRate;

        float randomGlitch = (std::abs(pitchGlitch.value) > 50.0f) ? (nextNoise() * (pitchGlitch.value * 0.01f) * 12.0f) : 0.0f;
        float effectivePitch = pitch.value + (pitchGlitch.value * 0.36f) * pitchEnvState + randomGlitch;
        effectivePitch = std::clamp(effectivePitch, 12.0f, 108.0f);
        currentFreq = 440.0f * std::pow(2.0f, (effectivePitch - 69.0f) / 12.0f);

        // --- 4. Core Oscillator with Feedback FM & Chaos ---
        float fbAmount = (feedback.value * 0.01f);
        float fmMod = (lastOutput * fbAmount * 2.0f) + (chaosSignal * chVal * 1.5f);

        phase += (currentFreq * (1.0f + fmMod * 0.5f)) * sampleRateDiv;
        if (phase >= 1.0f) {
            phase -= 1.0f;
            cycleCounter++;
        }

        // Primary VCO waveform (Tri-Saw morph with chaotic FM)
        float triVal = 2.0f * std::abs(2.0f * (phase - std::floor(phase + 0.5f))) - 1.0f;
        float sawVal = 2.0f * phase - 1.0f;
        float oscSig = lerp(triVal, sawVal, std::clamp(chVal + modAmt, 0.0f, 1.0f));

        // --- 5. Sub-Harmonic Logic & Bit-Divider ---
        float subVal = subHarm.value * 0.01f;
        float subSig = 0.0f;
        if (subVal > 0.001f) {
            float sub1 = (phase < 0.5f) ? 0.8f : -0.8f;
            float sub2 = ((cycleCounter / 2) % 2 == 0) ? 0.8f : -0.8f;
            float sub4 = ((cycleCounter / 4) % 2 == 0) ? 0.8f : -0.8f;
            subSig = (sub1 * 0.5f + sub2 * 0.3f + sub4 * 0.2f);
            oscSig = lerp(oscSig, oscSig * subSig, subVal);
        }

        // --- 6. Ring Modulator ---
        float rmVal = ringMod.value * 0.01f;
        if (rmVal > 0.001f) {
            float ringFreq = currentFreq * 1.4142f; // Non-integer root-2 carrier
            ringPhase += ringFreq * sampleRateDiv;
            if (ringPhase >= 1.0f) ringPhase -= 1.0f;
            float carrier = Math::fastSin2(PI_X2 * ringPhase);
            float ringed = oscSig * carrier;
            oscSig = lerp(oscSig, ringed, rmVal);
        }

        // --- 7. Multi-stage Wavefolder ---
        float foldVal = std::clamp((fold.value * 0.01f) + modAmt * 0.3f, 0.0f, 1.0f);
        float foldedSig = wavefold(oscSig, foldVal);

        // --- 8. State-Variable Filter (Color) ---
        float colVal = std::clamp((color.value * 0.01f) + modAmt * 0.2f, 0.01f, 0.99f);
        float cutFreq = lerp(40.0f, 16000.0f, colVal);
        float cutNorm = std::clamp(cutFreq * 2.0f * sampleRateDiv, 0.01f, 0.98f);
        float resoNorm = lerp(0.0f, 0.92f, std::abs(colVal - 0.5f) * 2.0f);

        filter.setCutoff(cutNorm);
        filter.setResonance(resoNorm);
        FilterSVF::Data& svf = filter.process12(foldedSig);

        // Filter Morphing: LP (0%) -> BP (50%) -> HP (100%)
        float filteredOut = 0.0f;
        if (colVal < 0.5f) {
            filteredOut = lerp(svf.lp, svf.bp, colVal * 2.0f);
        } else {
            filteredOut = lerp(svf.bp, svf.hp, (colVal - 0.5f) * 2.0f);
        }

        // --- 9. Final Output & Feedback Loop ---
        float finalOut = filteredOut * ampEnv * velocity;
        lastOutput = finalOut;

        return std::clamp(finalOut, -1.0f, 1.0f);
    }
};
