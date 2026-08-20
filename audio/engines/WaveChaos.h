#pragma once

#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

class WaveChaos : public EngineBase<WaveChaos> {
public:
    enum ModSource {
        SRC_ENV,
        SRC_LFO_TRI,
        SRC_LFO_SAW,
        SRC_LFO_SH
    };

    enum ModDest {
        DST_FILTER,
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
    char modTypeName[32] = "ENV Cutoff";
    Param params[24];

    // Page 1: Tone, Wave & Filter (Drift Core)
    Param& pitch = addParam({ .key = "pitch", .label = "Pitch", .unit = "", .value = 48.0f, .min = 24.0f, .max = 96.0f, .step = 1.0f });
    Param& waveform = addParam({ .key = "waveform", .label = "Wave", .unit = "", .value = 0.3f, .min = 0.0f, .max = 1.0f, .step = 0.01f });
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .unit = "", .value = 0.2f, .min = 0.02f, .max = 0.98f, .step = 0.01f });
    Param& resonance = addParam({ .key = "resonance", .label = "Reso", .unit = "", .value = 0.8f, .min = 0.0f, .max = 0.95f, .step = 0.01f });

    // Page 2: Envelopes, Filter Morph & Mix
    Param& release = addParam({ .key = "release", .label = "Release", .unit = "ms", .value = 200.0f, .min = 10.0f, .max = 2000.0f, .step = 10.0f });
    Param& envAmt = addParam({ .key = "envAmt", .label = "Env Amt", .unit = "", .value = 0.2f, .min = 0.0f, .max = 1.0f, .step = 0.01f });
    Param& filterMorph = addParam({ .key = "filterMorph", .label = "Filt Morph", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f, .step = 0.01f });
    Param& chaosMix = addParam({ .key = "chaosMix", .label = "Chaos Mix", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    // Page 3: Drift Modulation & Distortion
    Param& modType = addParam({ .key = "modType", .label = "Mod Type", .string = modTypeName, .value = 0.0f, .min = 0.0f, .max = 15.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                    auto* s = (WaveChaos*)ctx;
                                    int idx = std::clamp((int)std::round(val), 0, TOTAL_MOD_TYPES - 1);
                                    strncpy(s->modTypeName, modMatrix[idx].name, sizeof(s->modTypeName) - 1);
                                } });
    Param& modDepth = addParam({ .key = "modDepth", .label = "Mod Depth", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f });
    Param& modSpeed = addParam({ .key = "modSpeed", .label = "Mod Speed", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& crushFm = addParam({ .key = "crushFm", .label = "Crsh / FM", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f });

    // Page 4: Digital Chaos & Pitch Glitch (Chaos Core)
    Param& fmDepth = addParam({ .key = "fmDepth", .label = "Chaos FM", .unit = "%", .value = 25.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& ringMod = addParam({ .key = "ringMod", .label = "Ring Mod", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& color = addParam({ .key = "color", .label = "Color", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& pitchGlitch = addParam({ .key = "pitchGlitch", .label = "PitchGlitch", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f });

    // Page 5: Chaos LFO & Reverb Effect
    Param& lfoSpeed = addParam({ .key = "lfoSpeed", .label = "Chaos LFO", .unit = "%", .value = 40.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& lfoDepth = addParam({ .key = "lfoDepth", .label = "LFO Depth", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& reverbMix = addParam({ .key = "rvbMix", .label = "Rvb Mix", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& reverbDamp = addParam({ .key = "rvbDamp", .label = "Rvb Damp", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    // Page 6: Delay & Drive Effects
    Param& dlyMix = addParam({ .key = "dlyMix", .label = "Dly Mix", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& dlyTime = addParam({ .key = "dlyTime", .label = "Dly Time", .unit = "ms", .value = 125.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f });
    Param& dlyFdbk = addParam({ .key = "dlyFdbk", .label = "Dly Fdbk", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    WaveChaos(float sr = 44100.0f, float* dlBuf = nullptr, float* rvBuf = nullptr)
        : EngineBase(Synth, "WaveChaos", params)
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

        modType.set(0.0f);
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

        // --- 2. Chaos LFO ---
        float chaosLfoHz = lerp(0.1f, 25.0f, lfoSpeed.value * 0.01f);
        chaosLfoPhase += chaosLfoHz * sampleRateDiv;
        if (chaosLfoPhase >= 1.0f) chaosLfoPhase -= 1.0f;
        float chaosLfoVal = Math::fastSin2(PI_X2 * chaosLfoPhase) * (lfoDepth.value * 0.01f);

        // --- 3. Drift Synth LFO & Modulation Matrix ---
        float msPct = modSpeed.value * 0.01f;
        float lfoHz = 0.05f + (msPct * msPct * 39.95f);
        driftLfoPhase += lfoHz * sampleRateDiv;
        if (driftLfoPhase >= 1.0f) driftLfoPhase -= 1.0f;

        int routeIdx = std::clamp((int)std::round(modType.value), 0, TOTAL_MOD_TYPES - 1);
        ModRouting currentRoute = modMatrix[routeIdx];

        float srcVal = 0.0f;
        switch (currentRoute.source) {
        case SRC_ENV:
            srcVal = ampEnv;
            break;
        case SRC_LFO_TRI:
            srcVal = (driftLfoPhase < 0.5f) ? (4.0f * driftLfoPhase - 1.0f) : (3.0f - 4.0f * driftLfoPhase);
            break;
        case SRC_LFO_SAW:
            srcVal = 2.0f * driftLfoPhase - 1.0f;
            break;
        case SRC_LFO_SH: {
            uint32_t samplesPerHold = std::max((uint32_t)1, (uint32_t)(sampleRate / std::max(0.1f, lfoHz)));
            if (++shCounter >= samplesPerHold) {
                shCounter = 0;
                shValue = nextNoise();
            }
            srcVal = shValue;
            break;
        }
        }

        float modulationAmount = srcVal * (modDepth.value * 0.01f);

        float finalCutoff = cutoff.value;
        float finalPitchInterval = 0.0f;
        float finalWaveform = waveform.value;
        float levelMod = 1.0f;
        float finalCrushFm = crushFm.value;

        if (currentRoute.dest == DST_FILTER) {
            finalCutoff = std::clamp(cutoff.value + modulationAmount, 0.01f, 0.99f);
        } else if (currentRoute.dest == DST_PITCH) {
            finalPitchInterval = modulationAmount * 12.0f;
        } else if (currentRoute.dest == DST_MORPH) {
            finalWaveform = std::clamp(waveform.value + modulationAmount, 0.0f, 1.0f);
        } else if (currentRoute.dest == DST_LEVEL) {
            levelMod = std::clamp(1.0f + modulationAmount, 0.0f, 2.0f);
        } else if (currentRoute.dest == DST_CRUSH_FM) {
            finalCrushFm = std::clamp(crushFm.value + (modulationAmount * 100.0f), -100.0f, 100.0f);
        }

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

        float morphNorm = std::clamp(finalWaveform + chaosLfoVal * 0.2f, 0.0f, 1.0f);
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

        float colVal = std::clamp((color.value * 0.01f) + chaosLfoVal * 0.2f, 0.01f, 0.99f);
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

        // --- 11. Main SVF Filter Stage ---
        float cutEnv = std::clamp(finalCutoff + (ampEnv * envAmt.value * 0.5f), 0.02f, 0.98f);
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
            dlyFbSmooth += 0.001f * (dlyFdbk.value * 0.0085f - dlyFbSmooth);
            delayBuf[delayWrite] = outSig + del * dlyFbSmooth;
            delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;
            outSig = lerp(outSig, outSig + del, dlyMix.value * 0.01f);
        }

        // --- 15. Internal Reverb Effect ---
        if (reverbMix.value > 0.001f && reverbBuf != nullptr) {
            float wet = 0.0f, d = 0.2f + (reverbDamp.value * 0.007f);
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
