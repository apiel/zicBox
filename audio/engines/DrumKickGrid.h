#pragma once

#include "audio/MultiFx.h"
#include "audio/effects/applyBoost.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"
#include "helpers/clamp.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class DrumKickGrid : public EngineBase<DrumKickGrid> {
public:
    MultiFx multiFx;

protected:
    const float sampleRate;
    const float sampleRateDiv;

    float velocity = 1.0f;
    float notePitchMod = 1.0f;
    float speedRatio = 1.0f;

    // Phases and envelopes
    float carrierPhase = 0.0f;
    float modulatorPhase = 0.0f;
    float pitchEnv = 1.0f;
    float bodyEnv = 0.0f;
    float bodyStep = 0.0f;
    float fmEnv = 0.0f;
    float clickEnv = 0.0f;

    // Filter states
    FilterSVF mainFilter;
    float clickFilterState = 0.0f;

    // Fast noise generator (LCG)
    uint32_t noiseState = 34567;
    float nextNoise()
    {
        noiseState = noiseState * 196314165 + 907633389;
        return (float)int32_t(noiseState) / 2147483648.f;
    }

    // Dynamic Pink Noise filter state
    float pinkState = 0.0f;

    // --- Sub-Bass Rumble Buffer Replay State ---
    static const int RUMBLE_BUF_SIZE = 44100;
    float kickBuffer[RUMBLE_BUF_SIZE];
    int kickWritePos = 0;
    float kickReadPos = 0.0f;
    double kickElapsedSamples = 0.0;
    float rumbleLP1 = 0.0f;
    float rumbleLP2 = 0.0f;

    // FX and Dynamics state
    float driveFeedback = 0.0f;
    float bassBoostPrevInput = 0.0f;
    float bassBoostPrevOutput = 0.0f;
    float compressionEnv = 0.0f;

    char clickModeName[24] = "Noise Snap";
    char fxName[24] = "Off";

    static float lerp(float a, float b, float t) { return a + t * (b - a); }

    // Waveform Morphing: Sine (0%) -> Tri (33%) -> Saw (66%) -> Square (100%)
    float getVCO(float ph, float morphNorm)
    {
        float s = Math::fastSin2(PI_X2 * ph);
        if (morphNorm <= 0.0f) return s;

        float tri = 2.0f * std::abs(2.0f * (ph - std::floor(ph + 0.5f))) - 1.0f;
        float saw = 2.0f * (ph - std::floor(ph + 0.5f));
        float sq = (s > 0.0f) ? 0.75f : -0.75f;

        if (morphNorm < 0.333f) return lerp(s, tri, morphNorm * 3.0f);
        if (morphNorm < 0.666f) return lerp(tri, saw, (morphNorm - 0.333f) * 3.0f);
        return lerp(saw, sq, (morphNorm - 0.666f) * 3.0f);
    }

    float getShapedPitch(float p, float shape)
    {
        if (shape < 0.20f) {
            return lerp(std::sqrt(p), p, shape * 5.0f);
        } else if (shape < 0.40f) {
            return lerp(p, p * p, (shape - 0.20f) * 5.0f);
        } else if (shape < 0.60f) {
            float t = (shape - 0.40f) * 5.0f;
            float sCurve = p * p * (3.0f - 2.0f * p);
            return lerp(p * p, sCurve * sCurve, t);
        } else if (shape < 0.80f) {
            float t = (shape - 0.80f) * 5.0f;
            float sCurve = p * p * (3.0f - 2.0f * p);
            float subDive = std::pow(p, 4.0f);
            return lerp(sCurve * sCurve, subDive, t);
        } else {
            float t = (shape - 0.80f) * 5.0f;
            float sCurve = p * p * (3.0f - 2.0f * p);
            float bounce = sCurve * sCurve + (0.15f * std::sin(M_PI * p) * p);
            float subDive = std::pow(p, 4.0f);
            return lerp(subDive, bounce, t);
        }
    }

public:
    // Declare exact parameter array size (24 params matching addParam calls)
    Param params[24];

    // --- PAGE 1: LIVE TWEAKING & PERFORMANCE (Encoders 0..11) ---
    Param& baseFreq = addParam({ .key = "baseFreq", .label = "Sub Freq", .unit = "Hz", .value = 50.0f, .min = 30.0f, .max = 120.0f, .step = 1.0f });
    Param& duration = addParam({ .key = "duration", .label = "Duration", .unit = "ms", .value = 350.0f, .min = 30.0f, .max = 1500.0f, .step = 10.0f });
    Param& sweepDep = addParam({ .key = "sweepDep", .label = "Sweep Dep", .unit = "%", .value = 60.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& sweepLen = addParam({ .key = "sweepLen", .label = "Sweep Len", .unit = "ms", .value = 25.0f, .min = 2.0f, .max = 150.0f, .step = 1.0f, .module = MODULE_ENV_SWEEP,
        .onUpdate = [](void* ctx, float v) {
            auto* engine = static_cast<DrumKickGrid*>(ctx);
            float spd = engine->lerp(0.002f, 0.15f, (v * 0.9f) * 0.01f);
            engine->speedRatio = Math::exp(-1.0f / (engine->sampleRate * spd));
        },
        .graph = [](void* ctx, float val) {
            auto* engine = static_cast<DrumKickGrid*>(ctx);
            float timeScale = val * 50000.0f;
            float pEnv = std::pow(engine->speedRatio, timeScale);
            float curve = engine->getShapedPitch(pEnv, engine->sweepShp.value * 0.01f);
            return curve * 2.0f - 1.0f;
        } });

    Param& bodyLevel = addParam({ .key = "bodyLevel", .label = "Body Vol", .unit = "%", .value = 100.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& clickAmt = addParam({ .key = "clickAmt", .label = "Click Amt", .unit = "%", .value = 40.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& clickDecay = addParam({ .key = "clickDecay", .label = "Click Dec", .unit = "ms", .value = 10.0f, .min = 1.0f, .max = 50.0f, .step = 1.0f });
    Param& clickTone = addParam({ .key = "clickTone", .label = "Click Tone", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    Param& vcoMorph = addParam({ .key = "vcoMorph", .label = "VCO Morph", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f,
        .graph = [](void* ctx, float val) {
            auto* engine = static_cast<DrumKickGrid*>(ctx);
            return engine->getVCO(val, engine->vcoMorph.value * 0.01f);
        } });
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 20.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .unit = "%", .value = 100.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& resonance = addParam({ .key = "resonance", .label = "Resonance", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 95.0f, .step = 1.0f });

    // --- PAGE 2: DEEP SYNTHESIS, SUB-RUMBLE & MASTER FX (Encoders 0..11) ---
    Param& sweepShp = addParam({ .key = "sweepShp", .label = "Sweep Shp", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f, .module = MODULE_ENV_SWEEP });
    Param& fmDepth = addParam({ .key = "fmDepth", .label = "FM Depth", .unit = "%", .value = 25.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& fmRatio = addParam({ .key = "fmRatio", .label = "FM Ratio", .unit = "x", .value = 1.5f, .min = 0.5f, .max = 8.0f, .step = 0.25f });
    Param& fmSnap = addParam({ .key = "fmSnap", .label = "FM Decay", .unit = "ms", .value = 25.0f, .min = 2.0f, .max = 150.0f, .step = 1.0f });

    Param& clickMode = addParam({ .key = "clickMode", .label = "Click Mode", .string = clickModeName, .value = 0.0f, .min = 0.0f, .max = 3.0f, .step = 1.0f,
        .setStringFn = [](void* ctx, float val, char* str) {
            int idx = static_cast<int>(val + 0.5f);
            static const char* names[] = { "Noise Snap", "Pitch Imp", "Pink Noise", "Metal Snap" };
            if (idx >= 0 && idx < 4) strcpy(str, names[idx]);
            else strcpy(str, "Noise Snap");
        } });
    Param& rumbleAmt = addParam({ .key = "rumbleAmt", .label = "Rumble", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& rumbleGap = addParam({ .key = "rumbleGap", .label = "Rum Gap", .unit = "ms", .value = 100.0f, .min = 10.0f, .max = 300.0f, .step = 5.0f });
    Param& rumbleTone = addParam({ .key = "rumbleTone", .label = "Rum Tone", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    Param& bassBoost = addParam({ .key = "bassBoost", .label = "Bass Boost", .unit = "%", .value = 30.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& fxType = addParam({ .key = "fxType", .label = "FX Type", .string = fxName, .value = 0.0f, .max = (float)MultiFx::FX_COUNT - 1, .step = 1.0f,
        .onUpdate = [](void* ctx, float v) { static_cast<DrumKickGrid*>(ctx)->multiFx.setEffect(v); },
        .setStringFn = [](void* ctx, float value, char* str) { strcpy(str, static_cast<DrumKickGrid*>(ctx)->multiFx.getEffectName(value)); },
        .stringToFloatFn = [](void* ctx, const char* valStr) { return (float)static_cast<DrumKickGrid*>(ctx)->multiFx.getEffect(valStr); } });
    Param& fxAmt = addParam({ .key = "fxAmt", .label = "FX Amt", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& compress = addParam({ .key = "compress", .label = "Compressor", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    DrumKickGrid(const float sampleRate = 44100.0f, float* fxBuffer = nullptr)
        : EngineBase(Drum, "Kick Grid", params)
        , multiFx(sampleRate, fxBuffer)
        , sampleRate(sampleRate)
        , sampleRateDiv(1.0f / sampleRate)
    {
        std::fill_n(kickBuffer, RUMBLE_BUF_SIZE, 0.0f);

        float defaultSweep = sweepLen.value;
        float spd = lerp(0.002f, 0.15f, (defaultSweep * 0.9f) * 0.01f);
        speedRatio = Math::exp(-1.0f / (sampleRate * spd));

        init();
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        notePitchMod = std::pow(2.0f, (static_cast<float>(note) - 60.0f) / 12.0f);

        carrierPhase = 0.0f;
        modulatorPhase = 0.0f;
        pitchEnv = 1.0f;
        fmEnv = 1.0f;
        clickEnv = 1.0f;

        float durSamples = std::max(1.0f, sampleRate * (duration.value * 0.001f));
        bodyEnv = 1.0f;
        bodyStep = 1.0f / durSamples;

        // Reset sub-rumble replay buffer & state
        kickWritePos = 0;
        kickReadPos = 0.0f;
        kickElapsedSamples = 0.0;
        rumbleLP1 = 0.0f;
        rumbleLP2 = 0.0f;
        compressionEnv = 0.0f;
        driveFeedback = 0.0f;
        clickFilterState = 0.0f;
        pinkState = 0.0f;
    }

    float sampleImpl()
    {
        if (bodyEnv <= 0.0f && clickEnv <= 0.0001f) {
            float tail = multiFx.apply(0.0f, fxAmt.value * 0.01f);
            if (compress.value > 0.0f) {
                tail = applyCompression2(tail, compress.value * 0.01f, compressionEnv);
            }
            return tail;
        }

        // 1. Tonal Body Synthesis
        float tonalSig = 0.0f;
        if (bodyEnv > 0.0f && bodyLevel.value > 0.0f) {
            float currentBodyEnv = bodyEnv;
            bodyEnv -= bodyStep;
            if (bodyEnv < 0.0f) bodyEnv = 0.0f;

            pitchEnv *= speedRatio;
            float pMorph = getShapedPitch(pitchEnv, sweepShp.value * 0.01f);

            float rootFreq = (baseFreq.value * notePitchMod) + (sweepDep.value * 4.0f * pMorph);
            fmEnv *= Math::exp(-1.0f / (sampleRate * (fmSnap.value * 0.001f)));

            float modFreq = rootFreq * fmRatio.value;
            modulatorPhase += modFreq * sampleRateDiv;
            if (modulatorPhase > 1.0f) modulatorPhase -= 1.0f;

            float modSig = Math::fastSin2(PI_X2 * modulatorPhase);
            float fmIntensity = (fmDepth.value * 0.01f) * fmEnv * 0.25f;

            carrierPhase += (rootFreq * sampleRateDiv) + (modSig * fmIntensity);
            if (carrierPhase > 1.0f) carrierPhase -= 1.0f;

            float rawWave = getVCO(carrierPhase, vcoMorph.value * 0.01f);
            tonalSig = rawWave * currentBodyEnv * (bodyLevel.value * 0.01f);
        }

        // 2. Click / Transient Synthesis
        float clickSig = 0.0f;
        if (clickEnv > 0.0001f && clickAmt.value > 0.0f) {
            clickEnv *= Math::exp(-1.0f / (sampleRate * (clickDecay.value * 0.001f)));

            float rawClick = 0.0f;
            int mode = static_cast<int>(clickMode.value + 0.5f);

            if (mode == 0) {
                // Snappy LCG Noise transient (DriftKick style)
                rawClick = nextNoise();
            } else if (mode == 1) {
                // Pitch impulse transient
                rawClick = Math::fastSin2(PI_X2 * std::pow(clickEnv, 0.3f) * 3.0f);
            } else if (mode == 2) {
                // Pink noise thud
                float white = nextNoise();
                pinkState = 0.95f * pinkState + 0.05f * white;
                rawClick = pinkState * 2.0f;
            } else if (mode == 3) {
                // Metal inharmonic snap
                float ph1 = clickEnv * 12.0f;
                float ph2 = clickEnv * 19.3f;
                rawClick = (Math::fastSin2(PI_X2 * ph1) + Math::fastSin2(PI_X2 * ph2)) * 0.5f;
            }

            // Click tone filter (One-pole LP/HP morph)
            float toneNorm = clickTone.value * 0.01f;
            float coeff = std::clamp(0.01f + toneNorm * 0.8f, 0.01f, 0.9f);
            clickFilterState += coeff * (rawClick - clickFilterState);

            float filteredClick = (toneNorm > 0.5f) ? (rawClick - clickFilterState) : clickFilterState;
            clickSig = filteredClick * clickEnv * (clickAmt.value * 0.01f);
        }

        // 3. Sum Kick Body + Click Transient
        float kickOut = tonalSig + clickSig;

        // Store kick sample into buffer for sub-rumble replay
        if (kickWritePos < RUMBLE_BUF_SIZE) {
            kickBuffer[kickWritePos++] = kickOut;
        }

        // 4. Sub-Bass Rumble Tail Replay (0.7x Speed Pitch Down Buffer)
        float rumbleOut = 0.0f;
        float rAmt = rumbleAmt.value * 0.01f;

        if (rAmt > 0.001f) {
            double targetGapSamples = (rumbleGap.value * 0.001f) * sampleRate;
            if (kickElapsedSamples >= targetGapSamples) {
                float rawReplaySample = 0.0f;
                int idxA = (int)kickReadPos;
                int idxB = idxA + 1;

                if (idxA < kickWritePos) {
                    float frac = kickReadPos - (float)idxA;
                    float sA = kickBuffer[idxA];
                    float sB = (idxB < kickWritePos) ? kickBuffer[idxB] : sA;
                    rawReplaySample = sA + frac * (sB - sA);

                    kickReadPos += 0.70f;
                }

                float cutoffHz = 30.0f + ((rumbleTone.value * 0.01f) * 90.0f);
                float lpfCoeff = std::clamp((float)(1.0f - std::exp(-2.0f * M_PI * cutoffHz / sampleRate)), 0.001f, 0.35f);

                rumbleLP1 += lpfCoeff * (rawReplaySample - rumbleLP1);
                rumbleLP2 += lpfCoeff * (rumbleLP1 - rumbleLP2);

                float dirtySub = std::tanh(rumbleLP2 * 4.5f);

                float timeSinceGap = static_cast<float>(kickElapsedSamples - targetGapSamples) / sampleRate;
                float riseEnv = 1.0f - std::exp(-timeSinceGap / 0.020f);
                float decayEnv = std::exp(-timeSinceGap / 0.350f);

                rumbleOut = dirtySub * riseEnv * decayEnv * (rAmt * 1.1f);
            }
            kickElapsedSamples += 1.0;
        }

        // 5. FX Signal Chain
        float sig = kickOut + rumbleOut;

        // Drive Overdrive
        if (drive.value > 0.0f) {
            sig = applyDrive(sig, (drive.value * 0.01f) * 3.0f);
        }

        // Bass EQ Boost
        if (bassBoost.value > 0.0f) {
            sig = applyBoost(sig, bassBoost.value * 0.01f, bassBoostPrevInput, bassBoostPrevOutput);
        }

        // Main Filter Cutoff & Resonance
        if (cutoff.value < 100.0f || resonance.value > 0.0f) {
            float normCutoff = std::clamp(0.01f + (cutoff.value * 0.01f) * 0.98f, 0.01f, 0.99f);
            float normRes = std::clamp(resonance.value * 0.01f, 0.0f, 0.95f);
            mainFilter.setCutoff(normCutoff);
            mainFilter.setResonance(normRes);
            sig = mainFilter.process12(sig).lp;
        }

        // MultiFX
        sig = multiFx.apply(sig, fxAmt.value * 0.01f);

        // 6. Compressor applied at the VERY END!
        if (compress.value > 0.0f) {
            sig = applyCompression2(sig, compress.value * 0.01f, compressionEnv);
        }

        return sig * velocity;
    }

    const char* getNameXYImpl() { return "Pitch & Decay"; }

    void setXYImpl(XY xy)
    {
        baseFreq.set(30.0f + xy.x * 90.0f);
        duration.set(30.0f + xy.y * 1470.0f);
    }

    XY getXYImpl()
    {
        return { (baseFreq.value - 30.0f) / 90.0f, (duration.value - 30.0f) / 1470.0f };
    }

    float drawImpl(float x)
    {
        float durBody = duration.value / 1500.0f;
        float envBody = std::exp(-x * (3.0f + (1.0f - durBody) * 8.0f));
        float pSweep = std::exp(-x * (5.0f + (sweepDep.value * 0.01f) * 15.0f));
        float ph = x * (2.0f + pSweep * 8.0f);
        float tonalSig = getVCO(ph - std::floor(ph), vcoMorph.value * 0.01f) * envBody * (bodyLevel.value * 0.01f);

        float durClick = clickDecay.value / 50.0f;
        float envClick = std::exp(-x * (15.0f + (1.0f - durClick) * 40.0f));
        float clickSig = (std::sin(x * 200.0f) * std::cos(x * 450.0f)) * envClick * (clickAmt.value * 0.01f);

        float sig = tonalSig + clickSig;
        return std::clamp(sig, -1.0f, 1.0f);
    }
};
