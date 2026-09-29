#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

class ModWaveSynth {
public:
    enum ModSource {
        SRC_ENV = 0,
        SRC_LFO_TRI,
        SRC_LFO_SAW,
        SRC_LFO_SH
    };

    enum ModDest {
        DST_FILTER = 0,
        DST_PITCH,
        DST_MORPH,
        DST_LEVEL,
        DST_CRUSH_FM
    };

    struct ModRouting {
        const char* name;
        ModSource source;
        ModDest dest;
    };

    static constexpr int TOTAL_MOD_TYPES = 16;
    inline static const ModRouting modMatrix[TOTAL_MOD_TYPES] = {
        { "ENV Cutoff", SRC_ENV, DST_FILTER },
        { "ENV Pitch", SRC_ENV, DST_PITCH },
        { "ENV Wave", SRC_ENV, DST_MORPH },
        { "ENV Crsh/FM", SRC_ENV, DST_CRUSH_FM },
        { "LFO Tri Cut", SRC_LFO_TRI, DST_FILTER },
        { "LFO Tri Pit", SRC_LFO_TRI, DST_PITCH },
        { "LFO Tri Wave", SRC_LFO_TRI, DST_MORPH },
        { "LFO Tri Lvl", SRC_LFO_TRI, DST_LEVEL },
        { "LFO Tri CFM", SRC_LFO_TRI, DST_CRUSH_FM },
        { "LFO Saw Cut", SRC_LFO_SAW, DST_FILTER },
        { "LFO Saw Pit", SRC_LFO_SAW, DST_PITCH },
        { "LFO Saw Wave", SRC_LFO_SAW, DST_MORPH },
        { "LFO Saw CFM", SRC_LFO_SAW, DST_CRUSH_FM },
        { "LFO S&H Cut", SRC_LFO_SH, DST_FILTER },
        { "LFO S&H Pit", SRC_LFO_SH, DST_PITCH },
        { "LFO S&H CFM", SRC_LFO_SH, DST_CRUSH_FM }
    };

private:
    float sampleRate = 44100.0f;
    float sampleRateInv = 1.0f / 44100.0f;

    // Oscillator State
    float phase = 0.0f;
    float phaseInc = 0.01f;
    float targetFreq = 110.0f;
    float currentFreq = 110.0f;

    // Envelope State
    float ampEnv = 0.0f;
    float envDecay = 0.999f;

    // LFO State
    float lfoPhase = 0.0f;
    float lfoPhaseInc = 0.001f;
    float shHeldVal = 0.0f;
    uint32_t shCounter = 0;

    // Bitcrush / FM State
    int crushCounter = 0;
    float crushHeldSample = 0.0f;
    float fmPhase = 0.0f;
    uint32_t noiseState = 123456789;

    // Filter State (State Variable Filter)
    float ic1eq = 0.0f;
    float ic2eq = 0.0f;
    float filterG = 0.1f;
    float filterK = 1.0f;

    // Block Buffer (32 Samples)
    static constexpr int BLOCK_SIZE = 32;
    float blockBuf[BLOCK_SIZE];
    int blockIdx = BLOCK_SIZE; // Force initial block render

    // Fast PRNG Noise
    float nextNoise()
    {
        noiseState = noiseState * 196314165 + 907633389;
        return (float)int32_t(noiseState) / 2147483648.0f;
    }

    // PolyBLEP residual helper for anti-aliasing
    float polyBlep(float t, float dt)
    {
        if (t < dt) {
            t /= dt;
            return t + t - t * t - 1.0f;
        } else if (t > 1.0f - dt) {
            t = (t - 1.0f) / dt;
            return t * t + t + t + 1.0f;
        }
        return 0.0f;
    }

    // State safety & reset helper
    void checkFilterState()
    {
        if (std::isnan(ic1eq) || std::isinf(ic1eq) || std::abs(ic1eq) > 10.0f) {
            ic1eq = 0.0f;
        }
        if (std::isnan(ic2eq) || std::isinf(ic2eq) || std::abs(ic2eq) > 10.0f) {
            ic2eq = 0.0f;
        }
    }

public:
    // Raw Parameters (Controlled by UI / Pots)
    float pitchVal = 36.0f;
    float waveVal = 0.3f;
    float cutoffVal = 0.4f;
    float resVal = 0.3f;
    float releaseMs = 250.0f;
    float envAmtVal = 0.4f;
    float filterMorphVal = 0.0f;
    float crushFmVal = 0.0f;

    int modTypeIdx = 0;
    float modDepthVal = 0.0f;
    float modSpeedVal = 50.0f;
    float delaySendVal = 20.0f;

    ModWaveSynth(float sr = 44100.0f)
        : sampleRate(sr)
        , sampleRateInv(1.0f / sr)
    {
        memset(blockBuf, 0, sizeof(blockBuf));
    }

    void setSampleRate(float sr)
    {
        sampleRate = sr;
        sampleRateInv = 1.0f / sr;
    }

    void trigger(float note = -1.0f)
    {
        if (note >= 0.0f) pitchVal = note;
        targetFreq = 440.0f * std::pow(2.0f, (pitchVal - 69.0f) / 12.0f);
        currentFreq = targetFreq;
        ampEnv = 1.0f;
    }

    void noteOn(uint8_t note, float velocity = 1.0f)
    {
        (void)velocity;
        trigger((float)note);
    }

    void noteOff(uint8_t note)
    {
        (void)note;
    }

    float draw(float x)
    {
        float p = std::fmod(x, 1.0f);
        if (waveVal <= 0.33f) {
            float t = waveVal / 0.33f;
            float tri = (p < 0.5f) ? (4.0f * p - 1.0f) : (3.0f - 4.0f * p);
            float saw = 2.0f * p - 1.0f;
            return tri * (1.0f - t) + saw * t;
        } else if (waveVal <= 0.66f) {
            float t = (waveVal - 0.33f) / 0.33f;
            float saw = 2.0f * p - 1.0f;
            float sqr = (p < 0.5f ? 1.0f : -1.0f);
            return saw * (1.0f - t) + sqr * t;
        } else {
            float t = (waveVal - 0.66f) / 0.34f;
            float sqr = (p < 0.5f ? 1.0f : -1.0f);
            float n = std::sin(x * 47.13f + 1.5f);
            return sqr * (1.0f - t) + n * t;
        }
    }

    // Render a 32-sample block with pre-calculated control-rate parameters
    void renderBlock()
    {
        // 1. Calculate Envelope Decay Rate
        float relSec = std::max(0.010f, releaseMs * 0.001f);
        envDecay = std::exp(-1.0f / (relSec * sampleRate));

        // 2. Calculate LFO Phase Step & Update LFO Waveforms
        float lfoHz = 0.05f + (modSpeedVal * 0.01f) * (modSpeedVal * 0.01f) * 39.95f;
        lfoPhaseInc = lfoHz * sampleRateInv;
        lfoPhase += lfoPhaseInc * BLOCK_SIZE;
        lfoPhase = std::fmod(lfoPhase, 1.0f);

        float lfoTri = (lfoPhase < 0.5f) ? (4.0f * lfoPhase - 1.0f) : (3.0f - 4.0f * lfoPhase);
        float lfoSaw = 2.0f * lfoPhase - 1.0f;

        shCounter += BLOCK_SIZE;
        if (shCounter >= (uint32_t)(sampleRate * 0.1f)) {
            shCounter = 0;
            shHeldVal = nextNoise();
        }

        // 3. Modulation Routing Matrix
        auto route = modMatrix[std::clamp(modTypeIdx, 0, TOTAL_MOD_TYPES - 1)];
        float modSrcVal = 0.0f;
        switch (route.source) {
        case SRC_ENV: modSrcVal = ampEnv; break;
        case SRC_LFO_TRI: modSrcVal = lfoTri; break;
        case SRC_LFO_SAW: modSrcVal = lfoSaw; break;
        case SRC_LFO_SH: modSrcVal = shHeldVal; break;
        }

        float modAmount = modSrcVal * (modDepthVal * 0.01f);

        // Apply Dest Modulations
        float effectivePitch = pitchVal;
        float effectiveCutoff = cutoffVal;
        float effectiveWave = waveVal;
        float effectiveLevel = 1.0f;
        float effectiveCrushFm = crushFmVal;

        switch (route.dest) {
        case DST_PITCH: effectivePitch += modAmount * 12.0f; break;
        case DST_FILTER: effectiveCutoff = std::clamp(effectiveCutoff + modAmount * 0.45f, 0.02f, 0.98f); break;
        case DST_MORPH: effectiveWave = std::clamp(effectiveWave + modAmount * 0.40f, 0.0f, 1.0f); break;
        case DST_LEVEL: effectiveLevel = std::clamp(1.0f + modAmount * 0.50f, 0.0f, 1.8f); break;
        case DST_CRUSH_FM: effectiveCrushFm = std::clamp(effectiveCrushFm + modAmount * 50.0f, -100.0f, 100.0f); break;
        }

        // Frequency & Phase Increment
        currentFreq = 440.0f * std::pow(2.0f, (effectivePitch - 69.0f) / 12.0f);
        phaseInc = currentFreq * sampleRateInv;

        // 4. Precompute SVF Filter Coefficients
        checkFilterState();
        float cutHz = 80.0f + std::pow(effectiveCutoff, 2.4f) * 16000.0f;
        float normCut = std::clamp(cutHz * sampleRateInv, 0.002f, 0.45f);

        filterG = std::tan(3.14159265358979323846f * normCut);
        float resClamped = std::clamp(resVal, 0.0f, 0.98f);
        filterK = 2.0f - resClamped * 1.95f;
        float filterInvDenom = 1.0f / (1.0f + filterG * (filterG + filterK));

        // Bitcrush / FM setup
        float crushAmount = (effectiveCrushFm < 0.0f) ? (-effectiveCrushFm * 0.01f) : 0.0f;
        float fmAmount = (effectiveCrushFm > 0.0f) ? (effectiveCrushFm * 0.01f) : 0.0f;
        int crushHoldMax = 1 + (int)(crushAmount * 28.0f);

        // 5. Render 32 Audio Samples
        for (int i = 0; i < BLOCK_SIZE; ++i) {
            // Decay Envelope per sample
            ampEnv *= envDecay;

            // Oscillator Generation with PolyBLEP Anti-Aliasing
            float oscVal = 0.0f;

            // FM Phase Modulation
            float phaseMod = 0.0f;
            if (fmAmount > 0.001f) {
                fmPhase += phaseInc * 2.0f;
                fmPhase = std::fmod(fmPhase, 1.0f);
                phaseMod = std::sin(fmPhase * 6.2831853f) * (fmAmount * 0.5f);
            }

            float p = std::fmod(phase + phaseMod, 1.0f);

            if (effectiveWave <= 0.33f) {
                // Morph: Triangle -> Saw
                float t = effectiveWave / 0.33f;
                float tri = (p < 0.5f) ? (4.0f * p - 1.0f) : (3.0f - 4.0f * p);
                float saw = 2.0f * p - 1.0f - polyBlep(p, phaseInc);
                oscVal = tri * (1.0f - t) + saw * t;
            } else if (effectiveWave <= 0.66f) {
                // Morph: Saw -> Square / Pulse
                float t = (effectiveWave - 0.33f) / 0.33f;
                float saw = 2.0f * p - 1.0f - polyBlep(p, phaseInc);
                float sqr = (p < 0.5f ? 1.0f : -1.0f) + polyBlep(p, phaseInc) - polyBlep(std::fmod(p + 0.5f, 1.0f), phaseInc);
                oscVal = saw * (1.0f - t) + sqr * t;
            } else {
                // Morph: Square -> Noise Matrix Swarm
                float t = (effectiveWave - 0.66f) / 0.34f;
                float sqr = (p < 0.5f ? 1.0f : -1.0f) + polyBlep(p, phaseInc) - polyBlep(std::fmod(p + 0.5f, 1.0f), phaseInc);
                float n = nextNoise();
                oscVal = sqr * (1.0f - t) + n * t;
            }

            phase += phaseInc;
            if (phase >= 1.0f) phase -= 1.0f;

            // Bitcrush Sample & Hold Effect
            if (crushAmount > 0.001f) {
                crushCounter++;
                if (crushCounter >= crushHoldMax) {
                    crushCounter = 0;
                    crushHeldSample = oscVal;
                }
                oscVal = crushHeldSample;
            }

            // SVF Filter Processing (TPT Zero-Delay Feedback with Soft Saturation)
            float v1 = (oscVal - ic2eq - filterK * ic1eq) * filterInvDenom;
            float v1_sat = std::tanh(v1);

            float v2 = ic1eq + filterG * v1_sat;
            float v3 = ic2eq + filterG * v2;

            ic1eq = 2.0f * v2 - ic1eq;
            ic2eq = 2.0f * v3 - ic2eq;

            float lp = v3;
            float bp = v2;
            float hp = oscVal - filterK * v2 - v3;

            float filteredOut = 0.0f;
            if (filterMorphVal <= 0.5f) {
                float t = filterMorphVal * 2.0f;
                filteredOut = lp * (1.0f - t) + bp * t;
            } else {
                float t = (filterMorphVal - 0.5f) * 2.0f;
                filteredOut = bp * (1.0f - t) + hp * t;
            }

            blockBuf[i] = filteredOut * ampEnv * effectiveLevel;
        }

        blockIdx = 0;
    }

    // Process next sample
    float sample()
    {
        if (blockIdx >= BLOCK_SIZE) {
            renderBlock();
        }
        return blockBuf[blockIdx++];
    }
};
