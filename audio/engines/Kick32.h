#pragma once

/**
 * Kick32 — one kick engine, 32 parameters, one pot each.
 *
 * Combines the wavetable body of KickWave (shape / skew / harmonics / fold / phase)
 * with the FM + sweep + resonator + wavefolder + crush toolbox of PotKick, then adds
 * a noise voice routed three ways, a sub layer and a 3-band EQ.
 *
 * Param index == pot index (row = index / 8):
 *   row 0  BODY    length + curve, sweep, bend, FM, damp, sub
 *   row 1  WAVE    wavetable shaping and resonator
 *   row 2  NOISE   one noise voice (type / color / reso / sweep / decay) sent to the
 *                  output (Noise), into the distortion (Crunch) and onto the pitch (Chaos)
 *   row 3  MASTER  drive, distortion, crush, punch, EQ, volume
 *
 * There is no Tune pot: the pitch comes from the note (CV on the module,
 * sequencer transpose in the emulator). Note 60 = ROOT_HZ.
 *
 * No dependency on any UI or on desktop APIs: runs as is on the Daisy.
 * Time based coefficients are latched on trigger, so the per-sample path
 * stays free of exp()/pow() (except the optional resonator tracking).
 */

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "audio/effects/applyCompression.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Kick32 : public EngineBase<Kick32> {
public:
    static constexpr int PARAM_COUNT = 32;
    static constexpr float ROOT_HZ = 52.0f; // frequency of note 60

protected:
    const float sampleRate;
    float velocity = 1.0f;
    float notePitchMult = 1.0f;

    // Body
    float carrierPhase = 0.0f, modulatorPhase = 0.0f;
    float sweepEnv = 0.0f, sweepCoef = 0.0f;
    float fmEnv = 0.0f, fmCoef = 0.0f;
    float bendMult = 1.0f, bendStep = 1.0f;
    int ampPos = 0, ampLen = 1;
    float dampLp = 0.0f, dampBase = 18000.0f, dampCached = -1.0f;

    // Noise voice (one source, three destinations: output / crunch / chaos)
    uint32_t noiseState = 34567;
    float noiseEnv = 0.0f, noiseCoef = 0.0f;
    float nsIc1 = 0.0f, nsIc2 = 0.0f, nsA1 = 0.0f, nsA2 = 0.0f, nsA3 = 0.0f, nsK = 1.1f;
    int nsCtl = 0;
    float nsHeld = 0.0f;
    int nsHoldCnt = 0;
    float metalPhase[6] = { 0, 0, 0, 0, 0, 0 };
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

    // Time constant in ms -> per-sample multiplier (reaches 37% after `ms`).
    float tauCoef(float ms) const
    {
        return std::exp(-1.0f / (sampleRate * std::max(ms, 0.1f) * 0.001f));
    }

    // Amplitude curve. t = 0..1 over the Decay time.
    // Curve 0 = fat (stays up, falls at the end), 50 = linear, 100 = punchy (fast fall, long thin tail).
    float ampShape(float t) const
    {
        float u = 1.0f - t;
        float c = curve.value * 0.01f;
        float fat = 1.0f - t * t, lin = u, punchy = u * u * u;
        return c < 0.5f ? lerp(fat, lin, c * 2.0f) : lerp(lin, punchy, (c - 0.5f) * 2.0f);
    }

    // Raw noise source. Type: white -> stepped (digital grit) -> metal (6 inharmonic squares, hat-like)
    float noiseSource()
    {
        float type = pct(noiseType);
        float white = nextNoise();
        if (type < 0.001f) return white;

        if (++nsHoldCnt >= 5) {
            nsHoldCnt = 0;
            nsHeld = nextNoise();
        }
        if (type <= 0.5f) return lerp(white, nsHeld, type * 2.0f);

        static const float metalHz[6] = { 205.3f, 304.4f, 369.6f, 522.7f, 540.0f, 800.0f };
        float metal = 0.0f;
        for (int i = 0; i < 6; ++i) {
            metalPhase[i] += metalHz[i] * 2.7f / sampleRate;
            metalPhase[i] -= std::floor(metalPhase[i]);
            metal += metalPhase[i] < 0.5f ? 0.3f : -0.3f;
        }
        return lerp(nsHeld, metal, (type - 0.5f) * 2.0f);
    }

    // Noise voice: source -> state variable filter.
    //  Color  : LP -> BP -> HP while the cutoff rises (90 Hz .. 10 kHz)
    //  Reso   : from plain noise to a pitched ring
    //  Sweep  : the noise envelope moves the cutoff (+ = falls from high, - = rises from low)
    float noiseVoice()
    {
        float color = pct(noiseColor);
        if (nsCtl-- <= 0) { // coefficients at control rate, tan() is too expensive per sample
            nsCtl = 15;
            float sw = noiseSweep.value * 0.01f;
            float fc = 90.0f * std::exp2(color * 6.8f + sw * 4.0f * noiseEnv);
            fc = std::clamp(fc, 40.0f, sampleRate * 0.45f);
            float g = std::tan((float)M_PI * fc / sampleRate);
            float r = pct(noiseReso);
            nsK = lerp(1.4f, 0.06f, std::sqrt(r));
            nsA1 = 1.0f / (1.0f + g * (g + nsK));
            nsA2 = g * nsA1;
            nsA3 = g * nsA2;
        }
        float in = noiseSource();
        float v3 = in - nsIc2;
        float v1 = nsA1 * nsIc1 + nsA2 * v3;
        float v2 = nsIc2 + nsA2 * nsIc1 + nsA3 * v3;
        nsIc1 = 2.0f * v1 - nsIc1;
        nsIc2 = 2.0f * v2 - nsIc2;
        float hp = in - nsK * v1 - v2;
        float wLp = std::clamp(1.0f - color * 2.0f, 0.0f, 1.0f);
        float wHp = std::clamp(color * 2.0f - 1.0f, 0.0f, 1.0f);
        float wBp = 1.0f - std::abs(color * 2.0f - 1.0f);
        float out = (v2 * wLp + v1 * nsK * 1.5f * wBp + hp * wHp) * (0.5f + 2.0f * pct(noiseReso));
        return Math::fastTanh(out * 1.5f);
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
    Param& duration = addParam({ .key = "duration", .label = "Decay", .unit = "ms", .value = 350.0f, .min = 50.0f, .max = 1500.0f, .step = 10.0f });
    Param& curve = addParam({ .key = "curve", .label = "Curve", .unit = "%", .value = 75.0f });
    Param& sweep = addParam({ .key = "sweep", .label = "Sweep", .unit = "%", .value = 50.0f });
    Param& bend = addParam({ .key = "bend", .label = "Bend", .unit = "%" });
    Param& fmDepth = addParam({ .key = "fmDepth", .label = "FM Depth", .unit = "%", .value = 20.0f });
    Param& fmRatio = addParam({ .key = "fmRatio", .label = "FM Ratio", .unit = "x", .value = 1.5f, .min = 0.5f, .max = 8.0f, .step = 0.25f });
    Param& damp = addParam({ .key = "damp", .label = "Damp", .unit = "%" });
    Param& subAmt = addParam({ .key = "subAmt", .label = "Sub", .unit = "%" });

    // ---------------------------------------------------------------- row 1 : WAVE
    Param& waveShape = addParam({ .key = "waveShape", .label = "Wave Shape", .unit = "%" });
    Param& skew = addParam({ .key = "skew", .label = "Skew", .unit = "%", .value = 50.0f, .min = 5.0f, .max = 95.0f });
    Param& harmonic2 = addParam({ .key = "harmonic2", .label = "Harmonic 2", .unit = "%", .min = -100.0f, .type = VALUE_CENTERED });
    Param& harmonic3 = addParam({ .key = "harmonic3", .label = "Harmonic 3", .unit = "%", .min = -100.0f, .type = VALUE_CENTERED });
    Param& fold = addParam({ .key = "fold", .label = "Fold", .unit = "%" });
    Param& phaseOffset = addParam({ .key = "phaseOffset", .label = "Phase", .unit = "%" });
    Param& resonator = addParam({ .key = "resonator", .label = "Resonator", .unit = "%" });
    Param& resTune = addParam({ .key = "resTune", .label = "Res Tune", .unit = "st", .value = 0.0f, .min = -12.0f, .max = 36.0f });

    // ---------------------------------------------------------------- row 2 : NOISE
    Param& noiseAmt = addParam({ .key = "noiseAmt", .label = "Noise", .unit = "%" });
    Param& noiseType = addParam({ .key = "noiseType", .label = "Noise Type", .unit = "%" });
    Param& noiseColor = addParam({ .key = "noiseColor", .label = "Noise Color", .unit = "%", .value = 50.0f });
    Param& noiseReso = addParam({ .key = "noiseReso", .label = "Noise Reso", .unit = "%" });
    Param& noiseSweep = addParam({ .key = "noiseSweep", .label = "Noise Sweep", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED });
    Param& noiseDecay = addParam({ .key = "noiseDecay", .label = "Noise Decay", .unit = "ms", .value = 150.0f, .min = 3.0f, .max = 1200.0f, .step = 1.0f });
    Param& crunch = addParam({ .key = "crunch", .label = "Crunch", .unit = "%" });
    Param& chaos = addParam({ .key = "chaos", .label = "Chaos", .unit = "%" });

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

        carrierPhase = modulatorPhase = subPhase = 0.0f;
        svfLp = svfBp = 0.0f;
        compressionEnv = 0.0f;
        crushPhase = crushHold = 0.0f;

        ampLen = std::max(1, static_cast<int>(sampleRate * duration.value * 0.001f));
        ampPos = 0;

        float s = pct(sweep);
        sweepEnv = 1.0f;
        sweepCoef = tauCoef(6.0f + 40.0f * s); // a deeper sweep also lasts longer
        fmEnv = 1.0f;
        fmCoef = tauCoef(45.0f);
        bendMult = 1.0f;
        bendStep = std::exp2(-pct(bend) * 1.5f / ampLen); // down to -1.5 octave at the end of Decay

        noiseEnv = 1.0f;
        noiseCoef = decayCoef(noiseDecay.value);
        nsIc1 = nsIc2 = 0.0f;
        nsCtl = 0;
        subEnv = 1.0f;
        subCoef = decayCoef(duration.value * 1.6f); // the sub outlives the body
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

    /** Pitch over the Decay time as a ratio of the root (>= 1 on the sweep, < 1 with Bend).
     *  x = 0..1 is mapped with x^2 so the first milliseconds of the sweep stay readable. UI only. */
    float pitchRatio(float x)
    {
        float s = pct(sweep);
        float ms = duration.value * x * x;
        float env = std::exp(-ms / (6.0f + 40.0f * s));
        return std::exp2(-pct(bend) * 1.5f * x * x) * (1.0f + 24.0f * s * s * env);
    }

    float sampleImpl()
    {
        // ---- 1. body: wavetable + FM + sweep + bend + damp + resonator
        float body = 0.0f;
        float envAmp = 0.0f;
        if (ampPos < ampLen) {
            envAmp = ampShape((float)ampPos / ampLen);
            ampPos++;
        }
        float rootFreq = ROOT_HZ * notePitchMult;

        // noise voice, computed once and used three times (Noise / Crunch / Chaos)
        float nz = 0.0f;
        if (noiseEnv > 0.0005f) {
            noiseEnv *= noiseCoef;
            if (noiseAmt.value > 0.0f || crunch.value > 0.0f || chaos.value > 0.0f) nz = noiseVoice() * noiseEnv;
        }

        if (envAmp > 0.0001f) {
            sweepEnv *= sweepCoef;
            fmEnv *= fmCoef;
            bendMult *= bendStep;

            float s = pct(sweep);
            float freq = rootFreq * bendMult * (1.0f + 24.0f * s * s * sweepEnv);

            // FM as phase modulation: stable even with a huge index
            modulatorPhase += freq * fmRatio.value / sampleRate;
            modulatorPhase -= std::floor(modulatorPhase);
            float fd = pct(fmDepth);
            float pm = Math::fastSin2(PI_X2 * modulatorPhase) * fd * fd * 2.0f * fmEnv;

            // Chaos: the noise voice modulates the pitch. Colour decides the character
            // (low = wobble, high = fizz) and Noise Decay how long it lasts.
            float cx = pct(chaos);
            if (cx > 0.0f) freq *= std::max(0.1f, 1.0f + nz * cx * 0.6f);

            carrierPhase += freq / sampleRate;
            carrierPhase -= std::floor(carrierPhase);
            float sig = waveSample(carrierPhase + pm);

            // Damp: low-pass that starts open on the attack and closes with the sweep
            if (damp.value > 0.0f) {
                if (damp.value != dampCached) {
                    dampCached = damp.value;
                    dampBase = 18000.0f * std::pow(0.004f, damp.value * 0.01f); // 18 kHz .. 72 Hz
                }
                float fc = dampBase * (1.0f + 3.0f * sweepEnv);
                dampLp += (fc / (fc + sampleRate * 0.159155f)) * (sig - dampLp);
                sig = dampLp;
            }

            if (resonator.value > 0.0f) {
                sig = resonate(sig, freq * std::exp2(resTune.value / 12.0f), resonator.value * 0.01f);
            }
            body = sig * envAmp;
        }

        // ---- 2. sub: clean fundamental, no sweep, longer tail
        if (subEnv > 0.00002f) {
            subEnv *= subCoef;
            subPhase += rootFreq / sampleRate;
            subPhase -= std::floor(subPhase);
            body += Math::fastSin2(PI_X2 * subPhase) * subEnv * subAmt.value * 0.01f;
        }

        // ---- 3. saturation chain (Crunch = noise goes through the distortion with the kick)
        float out = body + nz * crunch.value * 0.008f;
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

        // ---- 4. clean noise on top
        out += nz * noiseAmt.value * 0.01f;

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
