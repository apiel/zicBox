#pragma once

#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

class TeKSynth : public EngineBase<TeKSynth> {
public:
    static constexpr int DELAY_BUF_SIZE = 48000;
    static constexpr int REVERB_BUF_SIZE = 16384;

private:
    float sampleRate = 44100.0f;
    float sampleRateDiv = 1.0f / 44100.0f;

    // State Variables
    float velocity = 1.0f;
    uint8_t currentNote = 48;
    float currentFreq = 130.81f;
    float targetFreq = 130.81f;

    // Envelopes & Phases
    float ampEnv = 0.0f;
    float pitchEnvState = 0.0f;
    float driftPhase = 0.0f;
    float chaosPhase = 0.0f;
    float ringPhase = 0.0f;
    float fmPhase = 0.0f;

    // LFOs
    float chaosLfoPhase = 0.0f;
    float driftLfoPhase = 0.0f;

    // Lorenz Chaotic Attractor State
    float lx = 0.1f;
    float ly = 0.0f;
    float lz = 0.0f;

    // SVF Filters
    FilterSVF filter;
    FilterSVF chaosFilter;

    // Bitcrusher & S&H state
    float crushHeldSample = 0.0f;
    int crushCounter = 0;
    uint32_t shCounter = 0;
    float shValue = 0.0f;

    // Delay & Reverb buffer management
    float* delayBuf = nullptr;
    std::vector<float> internalDelayBuf;
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;

    float* reverbBuf = nullptr;
    std::vector<float> internalReverbBuf;
    static constexpr int COMB_LEN[4] = { 1559, 1617, 1685, 1751 };
    static constexpr int AP_LEN[3] = { 347, 113, 37 };
    int combOff[4], apOff[3], combIdx[4], apIdx[3];
    float combFb[4] = { 0 };

    // Fast Noise LCG
    uint32_t noiseState = 987654321;
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
        return Math::fastSin2(driven * 1.5707963f);
    }

public:
    Param params[24];

    // Page 1: Tone, Wave & Filter
    Param& pitch = addParam({ .key = "pitch", .label = "Pitch", .unit = "", .value = 48.0f, .min = 24.0f, .max = 96.0f, .step = 1.0f });
    Param& waveform = addParam({ .key = "waveform", .label = "Wave", .unit = "", .value = 0.3f, .min = 0.0f, .max = 1.0f, .step = 0.01f });
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .unit = "", .value = 0.2f, .min = 0.02f, .max = 0.98f, .step = 0.01f });
    Param& resonance = addParam({ .key = "resonance", .label = "Reso", .unit = "", .value = 0.8f, .min = 0.0f, .max = 0.95f, .step = 0.01f });

    // Page 2: Envelopes, Filter Morph & Mix
    Param& release = addParam({ .key = "release", .label = "Release", .unit = "ms", .value = 200.0f, .min = 10.0f, .max = 2000.0f, .step = 10.0f });
    Param& envAmt = addParam({ .key = "envAmt", .label = "Env Cutoff", .unit = "%", .value = 20.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& filterMorph = addParam({ .key = "filterMorph", .label = "Filt Morph", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f, .step = 0.01f });

    // Page 3: Unified Morphable LFO Controls
    Param& lfoSpeed = addParam({ .key = "lfoSpeed", .label = "LFO Speed", .unit = "%", .value = 40.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& lfoShape = addParam({ .key = "lfoShape", .label = "LFO Shape", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& lfoToCutoff = addParam({ .key = "lfoToCutoff", .label = "LFO Cutoff", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f });
    Param& lfoToPitch = addParam({ .key = "lfoToPitch", .label = "LFO Pitch", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f });

    // Page 4: LFO Wave / Level / Crush Depths & Crush Control
    Param& lfoToMorph = addParam({ .key = "lfoToMorph", .label = "LFO Wave", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f });
    Param& lfoToLevel = addParam({ .key = "lfoToLevel", .label = "LFO Level", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f });
    Param& lfoToCrushFm = addParam({ .key = "lfoToCrushFm", .label = "LFO CrshFM", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f });
    Param& crushFm = addParam({ .key = "crushFm", .label = "Crsh / FM", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f });

    // Page 5: Digital Chaos & Pitch Glitch
    Param& chaosMix = addParam({ .key = "chaosMix", .label = "Chaos Mix", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& fmDepth = addParam({ .key = "fmDepth", .label = "Chaos FM", .unit = "%", .value = 25.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& ringMod = addParam({ .key = "ringMod", .label = "Ring Mod", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& color = addParam({ .key = "color", .label = "Color", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& pitchGlitch = addParam({ .key = "pitchGlitch", .label = "PitchGlitch", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f });

    // Page 6: Effects (Drive, Delay, Reverb)
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& reverbMix = addParam({ .key = "rvbMix", .label = "Rvb Mix", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& dlyMix = addParam({ .key = "dlyMix", .label = "Dly Mix", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& dlyTime = addParam({ .key = "dlyTime", .label = "Dly Time", .unit = "ms", .value = 125.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f });

    TeKSynth(float sr = 44100.0f, float* dlBuf = nullptr, float* rvBuf = nullptr)
        : EngineBase(Synth, "TeKSynth", params)
        , sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
        , delayBuf(dlBuf)
        , reverbBuf(rvBuf)
    {
        if (!delayBuf) {
            internalDelayBuf.assign(DELAY_BUF_SIZE, 0.0f);
            delayBuf = internalDelayBuf.data();
        }
        if (!reverbBuf) {
            internalReverbBuf.assign(REVERB_BUF_SIZE, 0.0f);
            reverbBuf = internalReverbBuf.data();
        }

        int pos = 0;
        for (int i = 0; i < 4; ++i) {
            combOff[i] = pos;
            pos += COMB_LEN[i];
            combIdx[i] = 0;
        }
        for (int i = 0; i < 3; ++i) {
            apOff[i] = pos;
            pos += AP_LEN[i];
            apIdx[i] = 0;
        }
    }

    void trigger(float vel = 1.0f)
    {
        noteOnImpl(currentNote, vel);
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        velocity = vel;
        currentNote = note;

        // Trigger envelopes
        ampEnv = 1.0f;
        pitchEnvState = 1.0f;

        // Perturb Lorenz chaotic attractor on trigger
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
        bool envActive = (ampEnv >= 0.0001f);

        if (!envActive && dlyMix.value <= 0.001f && reverbMix.value <= 0.001f) {
            ampEnv = 0.0f;
            return 0.0f;
        }

        if (envActive) {
            float relMs = std::clamp(release.value, 10.0f, 2000.0f);
            float ampDecayRate = std::exp(-1.0f / (relMs * 0.001f * sampleRate));
            ampEnv *= ampDecayRate;
        } else {
            ampEnv = 0.0f;
        }

        // --- 1. Lorenz Chaotic Attractor Step ---
        float dt = 0.014f;
        float sigma = 10.0f;
        float rho = 28.0f;
        float beta = 8.0f / 3.0f;

        float dx = sigma * (ly - lx);
        float dy = lx * (rho - lz) - ly;
        float dz = lx * ly - beta * lz;

        lx += dx * dt;
        ly += dy * dt;
        lz += dz * dt;

        lx = std::clamp(lx, -30.0f, 30.0f);
        ly = std::clamp(ly, -30.0f, 30.0f);
        lz = std::clamp(lz, 0.0f, 50.0f);

        float chaosSignal = std::clamp(lx * 0.05f, -1.0f, 1.0f);

        // --- 2. Unified Morphable LFO Generator ---
        float lfoSpeedPct = lfoSpeed.value * 0.01f;
        float lfoHz = 0.05f + (lfoSpeedPct * lfoSpeedPct * 29.95f);
        driftLfoPhase += lfoHz * sampleRateDiv;
        if (driftLfoPhase >= 1.0f) driftLfoPhase -= 1.0f;

        float sineVal = Math::fastSin2(PI_X2 * driftLfoPhase);
        float triVal = (driftLfoPhase < 0.5f) ? (4.0f * driftLfoPhase - 1.0f) : (3.0f - 4.0f * driftLfoPhase);
        float sawVal = 2.0f * driftLfoPhase - 1.0f;
        float sqVal = (driftLfoPhase < 0.5f) ? 0.8f : -0.8f;

        // Smoothly morph LFO shape across 4 continuous ranges:
        // 0.00 - 0.25: Sine <-> Triangle
        // 0.25 - 0.50: Triangle <-> Sawtooth
        // 0.50 - 0.75: Sawtooth <-> Square
        // 0.75 - 1.00: Square <-> Lorenz Chaos
        float lfoVal = sineVal;
        float sNorm = std::clamp(lfoShape.value * 0.01f, 0.0f, 1.0f);
        if (sNorm < 0.25f) {
            lfoVal = lerp(sineVal, triVal, sNorm * 4.0f);
        } else if (sNorm < 0.50f) {
            lfoVal = lerp(triVal, sawVal, (sNorm - 0.25f) * 4.0f);
        } else if (sNorm < 0.75f) {
            lfoVal = lerp(sawVal, sqVal, (sNorm - 0.50f) * 4.0f);
        } else {
            lfoVal = lerp(sqVal, chaosSignal, (sNorm - 0.75f) * 4.0f);
        }

        // --- 3. Per-Parameter LFO Modulation Depths ---
        float finalCutoff = std::clamp(cutoff.value + lfoVal * (lfoToCutoff.value * 0.005f), 0.01f, 0.99f);
        float finalPitchInterval = lfoVal * (lfoToPitch.value * 0.01f) * 12.0f;
        float finalWaveform = std::clamp(waveform.value + lfoVal * (lfoToMorph.value * 0.01f), 0.0f, 1.0f);
        float levelMod = std::clamp(1.0f + lfoVal * (lfoToLevel.value * 0.01f), 0.0f, 2.0f);
        float finalCrushFm = std::clamp(crushFm.value + lfoVal * (lfoToCrushFm.value * 1.0f), -100.0f, 100.0f);

        // --- 4. Pitch Envelope & Pitch Glitch ---
        float pEnvDecayRate = std::exp(-1.0f / (0.05f * sampleRate)); // 50ms pitch sweep
        pitchEnvState *= pEnvDecayRate;

        float randomGlitch = (std::abs(pitchGlitch.value) > 50.0f) ? (nextNoise() * (pitchGlitch.value * 0.01f) * 12.0f) : 0.0f;
        float noteOffset = static_cast<float>(currentNote) - 48.0f;
        float effectivePitch = pitch.value + noteOffset + finalPitchInterval + (pitchGlitch.value * 0.36f) * pitchEnvState + randomGlitch;
        effectivePitch = std::clamp(effectivePitch, 12.0f, 127.0f);
        float effectiveFreq = 440.0f * std::pow(2.0f, (effectivePitch - 69.0f) / 12.0f);

        // --- 5. Drift Operator FM Modulation (when crushFm > 0) ---
        float csVal = finalCrushFm;
        float driftFmVal = (csVal > 0.0f) ? (csVal * 0.01f) : 0.0f;
        float calculatedFmAmt = 0.0f;
        float calculatedFmRatio = 1.0f;

        if (driftFmVal > 0.001f) {
            if (driftFmVal <= 0.5f) {
                calculatedFmAmt = (driftFmVal / 0.5f) * 5.0f;
                calculatedFmRatio = 1.0f;
            } else {
                calculatedFmAmt = 5.0f;
                float normRatio = (driftFmVal - 0.5f) / 0.5f;
                calculatedFmRatio = 1.0f + std::round(normRatio * 7.0f);
            }

            float modFreq = effectiveFreq * calculatedFmRatio;
            fmPhase += modFreq * sampleRateDiv;
            if (fmPhase > 1.0f) fmPhase -= 1.0f;
        }

        float modOsc = std::sin(2.0f * M_PI * fmPhase);
        float fmDeviation = modOsc * calculatedFmAmt;

        // --- 6. Oscillator 1: Drift Synth Oscillator ---
        driftPhase += effectiveFreq * sampleRateDiv;
        if (driftPhase > 1.0f) driftPhase -= 1.0f;

        float modulatedCarrierPhase = driftPhase + fmDeviation;
        modulatedCarrierPhase = modulatedCarrierPhase - std::floor(modulatedCarrierPhase);

        float ph = modulatedCarrierPhase;
        float tri = 2.0f * std::abs(2.0f * (ph - std::floor(ph + 0.5f))) - 1.0f;
        float saw = 2.0f * ph - 1.0f;
        float sq = (ph < 0.5f) ? 0.8f : -0.8f;
        float ns = nextNoise();

        float wf = finalWaveform;
        float driftOscSig = 0.0f;
        if (wf < 0.333f) {
            driftOscSig = lerp(tri, saw, wf * 3.0f);
        } else if (wf < 0.666f) {
            driftOscSig = lerp(saw, sq, (wf - 0.333f) * 3.0f);
        } else {
            driftOscSig = lerp(sq, ns, (wf - 0.666f) * 3.0f);
        }

        // --- 7. Oscillator 2: Chaos Oscillator ---
        float chaosFmVal = fmDepth.value * 0.01f;
        float chaosFmMod = (chaosSignal * 1.5f) * chaosFmVal;

        chaosPhase += (effectiveFreq * (1.0f + chaosFmMod * 0.5f)) * sampleRateDiv;
        if (chaosPhase >= 1.0f) chaosPhase -= 1.0f;

        float morphNorm = std::clamp(finalWaveform + lfoVal * 0.2f, 0.0f, 1.0f);
        float cSinVal = Math::fastSin2(PI_X2 * chaosPhase);
        float cTriVal = 2.0f * std::abs(2.0f * (chaosPhase - std::floor(chaosPhase + 0.5f))) - 1.0f;
        float cSawVal = 2.0f * chaosPhase - 1.0f;
        float cSqVal = (cSinVal >= 0.0f) ? 0.8f : -0.8f;

        float chaosOscSig = cSinVal;
        if (morphNorm < 0.333f) {
            chaosOscSig = lerp(cSinVal, cTriVal, morphNorm * 3.0f);
        } else if (morphNorm < 0.666f) {
            chaosOscSig = lerp(cTriVal, cSawVal, (morphNorm - 0.333f) * 3.0f);
        } else {
            chaosOscSig = lerp(cSawVal, cSqVal, (morphNorm - 0.666f) * 3.0f);
        }

        // --- 8. Chaos Wavefolding & Color SVF Filter Stage ---
        float chaosFolded = wavefold(chaosOscSig, 0.6f);

        float colVal = std::clamp((color.value * 0.01f) + lfoVal * 0.2f, 0.01f, 0.99f);
        float cutFreq = lerp(80.0f, 16000.0f, colVal);
        float cutNorm = std::clamp(cutFreq * 2.0f * sampleRateDiv, 0.01f, 0.98f);
        float resoNorm = lerp(0.0f, 0.92f, std::abs(colVal - 0.5f) * 2.0f);

        chaosFilter.setCutoff(cutNorm);
        chaosFilter.setResonance(resoNorm);
        FilterSVF::Data& chaosSvf = chaosFilter.process12(chaosFolded);

        float chaosFilteredSig = 0.0f;
        if (colVal < 0.5f) {
            chaosFilteredSig = lerp(chaosSvf.lp, chaosSvf.bp, colVal * 2.0f);
        } else {
            chaosFilteredSig = lerp(chaosSvf.bp, chaosSvf.hp, (colVal - 0.5f) * 2.0f);
        }

        // --- 9. Mix Stage (Drift Synth vs Chaos Engine) ---
        float cMix = chaosMix.value * 0.01f;
        float mixedSig = lerp(driftOscSig, chaosFilteredSig, cMix);

        // --- 10. Ring Modulator Stage ---
        float rmVal = ringMod.value * 0.01f;
        if (rmVal > 0.001f) {
            float ringFreq = effectiveFreq * 1.4142f;
            ringPhase += ringFreq * sampleRateDiv;
            if (ringPhase >= 1.0f) ringPhase -= 1.0f;
            float carrier = Math::fastSin2(PI_X2 * ringPhase);
            float ringed = mixedSig * carrier;
            mixedSig = lerp(mixedSig, ringed, rmVal);
        }

        // --- 11. Main SVF Filter Stage with Dedicated Envelope ---
        float cutEnv = std::clamp(finalCutoff + (ampEnv * (envAmt.value * 0.01f) * 0.5f), 0.02f, 0.98f);
        filter.setCutoff(cutEnv);
        filter.setResonance(resonance.value);
        FilterSVF::Data& svf = filter.process12(mixedSig);

        float outSig = 0.0f;
        float fMorph = filterMorph.value;
        if (fMorph < 0.5f) {
            outSig = lerp(svf.lp, svf.bp, fMorph * 2.0f);
        } else {
            outSig = lerp(svf.bp, svf.hp, (fMorph - 0.5f) * 2.0f);
        }

        // --- 11b. Drive Stage ---
        float drvVal = drive.value * 0.01f;
        if (drvVal > 0.001f) {
            float driveGain = 1.0f + drvVal * 4.0f;
            outSig = std::tanh(outSig * driveGain);
        }

        // --- 12. Bitcrusher Stage (when crushFm < -0.1f) ---
        if (csVal < -0.1f) {
            float crushAmt = -csVal * 0.01f;
            int holdPeriod = 1 + (int)(crushAmt * 12.0f);
            crushCounter++;
            if (crushCounter >= holdPeriod) {
                crushCounter = 0;
                float bits = 16.0f - crushAmt * 13.0f;
                float steps = std::pow(2.0f, bits);
                crushHeldSample = std::round(outSig * steps) / steps;
            }
            outSig = lerp(outSig, crushHeldSample, crushAmt);
        }

        // --- 13. Output Amplification ---
        outSig = outSig * ampEnv * velocity * levelMod;

        // --- 14. Internal Feedback Delay Effect ---
        if (dlyMix.value > 0.001f && delayBuf != nullptr) {
            int n = (int)(dlyTime.value * 0.001f * sampleRate);
            n = std::clamp(n, 1, DELAY_BUF_SIZE - 1);
            float del = delayBuf[(delayWrite - n + DELAY_BUF_SIZE) % DELAY_BUF_SIZE];
            dlyFbSmooth += 0.001f * (0.35f - dlyFbSmooth);
            delayBuf[delayWrite] = outSig + del * dlyFbSmooth;
            delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;
            outSig = lerp(outSig, outSig + del, dlyMix.value * 0.01f);
        }

        // --- 15. Internal Reverb Effect ---
        if (reverbMix.value > 0.001f && reverbBuf != nullptr) {
            float wet = 0.0f, d = 0.55f;
            for (int i = 0; i < 4; ++i) {
                float val = reverbBuf[combOff[i] + combIdx[i]];
                combFb[i] = val * (1.0f - d) + combFb[i] * d;
                reverbBuf[combOff[i] + combIdx[i]] = outSig + combFb[i] * 0.88f;
                if (++combIdx[i] >= COMB_LEN[i]) combIdx[i] = 0;
                wet += val;
            }
            outSig = lerp(outSig, wet * 0.25f, reverbMix.value * 0.01f);
        }

        return std::clamp(outSig, -1.0f, 1.0f);
    }
};
