#pragma once

/**
 * Kick32 — one kick engine, 32 parameters, one pot each.
 *
 * Combines the wavetable body of KickWave (shape / skew / harmonics / fold / phase)
 * with the FM + sweep + resonator + wavefolder + crush toolbox of PotKick, then adds
 * click / noise / sub layers and a 3-band EQ.
 *
 * Param index == pot index (row = index / 8):
 *   row 0  BODY    pitch, length and FM
 *   row 1  WAVE    wavetable shaping and resonator
 *   row 2  LAYERS  click, noise and sub
 *   row 3  MASTER  drive, distortion, crush, punch, EQ, volume
 *
 * No dependency on any UI or on desktop APIs: runs as is on the Daisy.
 * Everything that depends on a decay time is latched on trigger, so the
 * per-sample path has no exp()/pow() beyond the optional resonator tracking.
 */

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "audio/EnvelopDrumAmp.h"
#include "audio/effects/applyCompression.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Kick32 : public EngineBase<Kick32> {
public:
    static constexpr int PARAM_COUNT = 32;

    EnvelopDrumAmp envelopAmp;

protected:
    const float sampleRate;
    float velocity = 1.0f;
    float notePitchMult = 1.0f;

    // Body
    float carrierPhase = 0.0f;
    float modulatorPhase = 0.0f;
    float pitchEnv = 0.0f;
    float fmEnv = 0.0f;
    float pitchCoef = 0.0f; // per-sample multiplier, already raised to the sweep-shape power
    float fmCoef = 0.0f;

    // Layers
    uint32_t noiseState = 34567;
    float clickEnv = 0.0f, clickCoef = 0.0f, clickLp = 0.0f;
    float noiseEnv = 0.0f, noiseCoef = 0.0f, noiseLp = 0.0f;
    float subEnv = 0.0f, subCoef = 0.0f, subPhase = 0.0f;

    // FX state
    float svfLp = 0.0f, svfBp = 0.0f;
    float foldFilterBuf = 0.0f;
    float compressionEnv = 0.0f;
    float crushPhase = 0.0f, crushHold = 0.0f;

    // EQ (one-pole crossovers, 3 bands that sum back to the input at 0 dB)
    float eqLowLp = 0.0f, eqHighLp = 0.0f;
    float eqLowCoef, eqHighCoef;
    float eqGain[3] = { 1.0f, 1.0f, 1.0f };
    float eqDb[3] = { 0.0f, 0.0f, 0.0f };

    static float lerp(float a, float b, float t) { return a + t * (b - a); }

    float nextNoise()
    {
        noiseState = noiseState * 196314165 + 907633389;
        return (float)int32_t(noiseState) / 2147483648.f;
    }

    // Decay time in ms -> per-sample multiplier reaching -60 dB after `ms`.
    float decayCoef(float ms) const
    {
        return std::exp(-6.9078f / (sampleRate * std::max(ms, 0.1f) * 0.001f));
    }

    // Resonant peak that follows the pitch (from KickWave / PotKick)
    float resonate(float input, float cutoffHz, float amount)
    {
        float f = 2.0f * std::sin((float)M_PI * std::clamp(cutoffHz, 30.0f, 3500.0f) / sampleRate);
        float q = 1.0f - amount * 0.96f;
        svfLp += f * svfBp;
        float hp = input - svfLp - q * svfBp;
        svfBp += f * hp;
        float peak = svfLp + svfBp * 1.8f;
        float sat = Math::fastTanh(peak * (1.0f + amount * 2.5f));
        return lerp(input, sat, std::min(amount * 1.25f, 1.0f));
    }

    // Gabber style wavefolder (from PotKick)
    float wavefold(float sig, float amount)
    {
        float foldAmt = (amount <= 0.70f) ? amount : (0.70f + (amount - 0.70f) * 0.25f);
        // asymmetry bias fades out with the signal, otherwise silence would output DC
        float driven = sig * (1.0f + amount * 5.5f) + amount * 0.15f * std::min(1.0f, std::abs(sig) * 8.0f);
        float folded = std::sin(driven * (1.0f + foldAmt * 1.2f));
        float saturated = Math::fastTanh(folded * 1.3f);
        float out = Math::fastTanh(lerp(sig, saturated, amount * 0.85f) * 1.1f);
        if (amount > 0.70f) {
            float t = (amount - 0.70f) / 0.30f;
            foldFilterBuf += (1.0f - t * 0.18f) * (out - foldFilterBuf);
            out = foldFilterBuf;
        } else {
            foldFilterBuf = out;
        }
        return out;
    }

    // Level-neutral tanh drive: full scale in -> full scale out
    static float softDrive(float x, float amount)
    {
        float g = 1.0f + amount * 9.0f;
        return Math::fastTanh(x * g) / Math::fastTanh(g);
    }

public:
    Param params[PARAM_COUNT];

    // ---------------------------------------------------------------- row 0 : BODY
    Param& baseFreq = addParam({ .key = "baseFreq", .label = "Tune", .unit = "Hz", .value = 52.0f, .min = 30.0f, .max = 120.0f });
    Param& duration = addParam({ .key = "duration", .label = "Decay", .unit = "ms", .value = 350.0f, .min = 50.0f, .max = 1500.0f, .step = 10.0f });
    Param& sweepDepth = addParam({ .key = "sweepDepth", .label = "Sweep", .unit = "%", .value = 50.0f });
    Param& sweepTime = addParam({ .key = "sweepTime", .label = "Sweep Time", .unit = "ms", .value = 25.0f, .min = 5.0f, .max = 200.0f });
    Param& sweepShape = addParam({
        .key = "sweepShape",
        .label = "Sweep Shape",
        .unit = "%",
        .value = 50.0f,
        .graph = [](void* ctx, float phase) {
            auto* self = static_cast<Kick32*>(ctx);
            float shape = self->sweepShape.value * 0.01f;
            return std::pow(std::exp(-phase * 4.0f), 1.0f + shape * 5.0f) * 2.0f - 1.0f;
        } });
    Param& fmDepth = addParam({ .key = "fmDepth", .label = "FM Depth", .unit = "%", .value = 35.0f });
    Param& fmRatio = addParam({ .key = "fmRatio", .label = "FM Ratio", .unit = "x", .value = 1.5f, .min = 0.5f, .max = 8.0f, .step = 0.25f });
    Param& fmSnap = addParam({ .key = "fmSnap", .label = "FM Snap", .unit = "ms", .value = 25.0f, .min = 2.0f, .max = 150.0f });

    // ---------------------------------------------------------------- row 1 : WAVE
    Param& waveShape = addParam({ .key = "waveShape", .label = "Wave Shape", .unit = "%" });
    Param& skew = addParam({ .key = "skew", .label = "Skew", .unit = "%", .value = 50.0f, .min = 5.0f, .max = 95.0f });
    Param& harmonic2 = addParam({ .key = "harmonic2", .label = "Harmonic 2", .unit = "%", .min = -100.0f, .type = VALUE_CENTERED });
    Param& harmonic3 = addParam({ .key = "harmonic3", .label = "Harmonic 3", .unit = "%", .min = -100.0f, .type = VALUE_CENTERED });
    Param& fold = addParam({ .key = "fold", .label = "Fold", .unit = "%" });
    Param& phaseOffset = addParam({ .key = "phaseOffset", .label = "Phase", .unit = "%" });
    Param& resonator = addParam({ .key = "resonator", .label = "Resonator", .unit = "%" });
    Param& resTune = addParam({ .key = "resTune", .label = "Res Tune", .unit = "st", .value = 0.0f, .min = -12.0f, .max = 36.0f });

    // ---------------------------------------------------------------- row 2 : LAYERS
    Param& clickAmt = addParam({ .key = "clickAmt", .label = "Click", .unit = "%", .value = 30.0f });
    Param& clickDecay = addParam({ .key = "clickDecay", .label = "Click Decay", .unit = "ms", .value = 10.0f, .min = 1.0f, .max = 100.0f });
    Param& clickTone = addParam({ .key = "clickTone", .label = "Click Tone", .unit = "%", .value = 60.0f });
    Param& noiseAmt = addParam({ .key = "noiseAmt", .label = "Noise", .unit = "%" });
    Param& noiseDecay = addParam({ .key = "noiseDecay", .label = "Noise Decay", .unit = "ms", .value = 120.0f, .min = 20.0f, .max = 800.0f });
    Param& noiseTone = addParam({ .key = "noiseTone", .label = "Noise Tone", .unit = "%", .value = 40.0f });
    Param& subAmt = addParam({ .key = "subAmt", .label = "Sub", .unit = "%" });
    Param& subDecay = addParam({ .key = "subDecay", .label = "Sub Decay", .unit = "ms", .value = 500.0f, .min = 50.0f, .max = 1500.0f, .step = 10.0f });

    // ---------------------------------------------------------------- row 3 : MASTER
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 35.0f });
    Param& distortion = addParam({ .key = "distortion", .label = "Distortion", .unit = "%" });
    Param& crush = addParam({ .key = "crush", .label = "Crush", .unit = "%" });
    Param& punch = addParam({ .key = "punch", .label = "Punch", .unit = "%", .value = 65.0f });
    Param& eqLow = addParam({ .key = "eqLow", .label = "EQ Low", .unit = "dB", .value = 0.0f, .min = -12.0f, .max = 12.0f, .step = 0.5f, .type = VALUE_CENTERED });
    Param& eqMid = addParam({ .key = "eqMid", .label = "EQ Mid", .unit = "dB", .value = 0.0f, .min = -12.0f, .max = 12.0f, .step = 0.5f, .type = VALUE_CENTERED });
    Param& eqHigh = addParam({ .key = "eqHigh", .label = "EQ High", .unit = "dB", .value = 0.0f, .min = -12.0f, .max = 12.0f, .step = 0.5f, .type = VALUE_CENTERED });
    Param& volume = addParam({ .key = "volume", .label = "Volume", .unit = "%", .value = 80.0f });

    Kick32(const float sampleRate = 44100.0f)
        : EngineBase(Drum, "Kick32", params)
        , sampleRate(sampleRate)
    {
        eqLowCoef = 1.0f - std::exp(-2.0f * (float)M_PI * 120.0f / sampleRate);
        eqHighCoef = 1.0f - std::exp(-2.0f * (float)M_PI * 2500.0f / sampleRate);
    }

    void trigger(float vel = 1.0f) { noteOnImpl(60, vel); }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        notePitchMult = std::pow(2.0f, (static_cast<float>(note) - 60.0f) / 12.0f);

        carrierPhase = 0.0f;
        modulatorPhase = 0.0f;
        subPhase = 0.0f;
        svfLp = svfBp = 0.0f;
        compressionEnv = 0.0f;
        crushPhase = crushHold = 0.0f;

        // latch every time based coefficient
        float shape = pct(sweepShape);
        pitchEnv = 1.0f;
        pitchCoef = std::pow(decayCoef(sweepTime.value * 0.33f), 1.0f + shape * 5.0f);
        fmEnv = 1.0f;
        fmCoef = decayCoef(fmSnap.value * 0.33f);

        clickEnv = 1.0f;
        clickCoef = decayCoef(clickDecay.value);
        noiseEnv = 1.0f;
        noiseCoef = decayCoef(noiseDecay.value);
        subEnv = 1.0f;
        subCoef = decayCoef(subDecay.value);

        envelopAmp.reset(static_cast<int>(sampleRate * (duration.value * 0.001f)));
    }

    void noteOffImpl(uint8_t) { }

    /** One cycle of the wavetable (phase 0..1) -> -1..1. Also used by the UI preview. */
    float waveSample(float phase)
    {
        float p = phase + phaseOffset.value * 0.01f;
        p -= std::floor(p);

        float skewNorm = std::clamp(skew.value * 0.01f, 0.05f, 0.95f);
        float w = (p < skewNorm) ? 0.5f * (p / skewNorm) : 0.5f + 0.5f * ((p - skewNorm) / (1.0f - skewNorm));

        float shapeVal = waveShape.value * 0.01f;
        float sinV = std::sin(w * 2.0f * (float)M_PI);
        float triV = (w < 0.5f) ? (4.0f * w - 1.0f) : (3.0f - 4.0f * w);
        float sawV = 1.0f - 2.0f * w;
        float sqV = (w < 0.5f) ? 1.0f : -1.0f;

        float s;
        if (shapeVal <= 0.3333f) s = lerp(sinV, triV, shapeVal / 0.3333f);
        else if (shapeVal <= 0.6666f) s = lerp(triV, sawV, (shapeVal - 0.3333f) / 0.3333f);
        else s = lerp(sawV, sqV, (shapeVal - 0.6666f) / 0.3333f);

        float h2 = harmonic2.value * 0.01f;
        if (std::abs(h2) > 0.001f) s += h2 * std::sin(w * 4.0f * (float)M_PI);
        float h3 = harmonic3.value * 0.01f;
        if (std::abs(h3) > 0.001f) s += h3 * std::sin(w * 6.0f * (float)M_PI);

        float f = fold.value * 0.01f;
        if (f > 0.001f) s = std::sin(s * (1.0f + f * 3.5f) * ((float)M_PI * 0.5f));

        return std::clamp(s, -1.0f, 1.0f);
    }

    float drawImpl(float x) { return waveSample(x); }

    float sampleImpl()
    {
        // ---- 1. body (wavetable + FM + sweep + resonator)
        float body = 0.0f;
        float envAmp = envelopAmp.next();
        float rootFreq = baseFreq.value * notePitchMult;

        if (envAmp > 0.0001f) {
            pitchEnv *= pitchCoef;
            fmEnv *= fmCoef;

            float freq = rootFreq * (1.0f + pitchEnv * sweepDepth.value * 0.05f);

            modulatorPhase += freq * fmRatio.value / sampleRate;
            modulatorPhase -= std::floor(modulatorPhase);
            float mod = Math::fastSin2(PI_X2 * modulatorPhase);

            carrierPhase += freq / sampleRate + mod * pct(fmDepth) * 0.75f * fmEnv * 0.025f;
            carrierPhase -= std::floor(carrierPhase);

            float sig = waveSample(carrierPhase);
            if (resonator.value > 0.0f) {
                sig = resonate(sig, freq * std::exp2(resTune.value / 12.0f), resonator.value * 0.01f);
            }
            body = sig * envAmp;
        }

        // ---- 2. sub layer: clean fundamental, no sweep, its own tail
        if (subEnv > 0.0005f) {
            subEnv *= subCoef;
            subPhase += rootFreq / sampleRate;
            subPhase -= std::floor(subPhase);
            body += Math::fastSin2(PI_X2 * subPhase) * subEnv * subAmt.value * 0.01f;
        }

        // ---- 3. saturation chain
        float out = body;
        if (distortion.value > 0.0f) out = wavefold(out, distortion.value * 0.01f);
        if (drive.value > 0.0f) out = softDrive(out, drive.value * 0.01f);
        out = applyCompression2(out, punch.value * 0.01f, compressionEnv);

        if (crush.value > 0.0f) {
            float dec = 1.0f + crush.value * 0.15f;
            crushPhase += 1.0f;
            if (crushPhase >= dec) {
                crushPhase -= dec;
                crushHold = out;
            }
            out = crushHold;
        }

        // ---- 4. click + noise are added after the chain to stay crisp
        if (clickEnv > 0.0005f) {
            clickEnv *= clickCoef;
            float a = 0.02f + 0.98f * pct(clickTone) * pct(clickTone);
            clickLp += a * (nextNoise() - clickLp);
            out += clickLp * clickEnv * clickAmt.value * 0.01f;
        }
        if (noiseEnv > 0.0005f) {
            noiseEnv *= noiseCoef;
            float a = 0.02f + 0.98f * pct(noiseTone) * pct(noiseTone);
            noiseLp += a * (nextNoise() - noiseLp);
            out += noiseLp * noiseEnv * noiseAmt.value * 0.01f;
        }

        // ---- 5. 3 band EQ (gains cached, recomputed only when a pot moves)
        updateEqGain(0, eqLow.value);
        updateEqGain(1, eqMid.value);
        updateEqGain(2, eqHigh.value);
        eqLowLp += eqLowCoef * (out - eqLowLp);
        eqHighLp += eqHighCoef * (out - eqHighLp);
        float low = eqLowLp;
        float high = out - eqHighLp;
        float mid = out - low - high;
        out = low * eqGain[0] + mid * eqGain[1] + high * eqGain[2];

        // ---- 6. volume + soft limiter
        return Math::fastTanh(out * volume.value * 0.012f) * velocity;
    }

private:
    void updateEqGain(int band, float db)
    {
        if (db != eqDb[band]) {
            eqDb[band] = db;
            eqGain[band] = std::pow(10.0f, db / 20.0f);
        }
    }
};
