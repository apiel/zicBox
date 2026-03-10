#pragma once

#include "audio/effects/applyDrive.h"
#include "audio/effects/applyReverb.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"

#include <algorithm>
#include <cmath>

class DrumHiHat23 : public EngineBase<DrumHiHat23> {

protected:
    const float sampleRate;

    float* reverbBuffer = nullptr;
    int reverbIndex = 0;

    float velocity = 1.0f;
    float ampEnv = 0.0f;
    float ampStep = 0.0f;
    float accentEnv = 0.0f; // fast transient burst
    float bodyEnv = 0.0f; // slower metallic body
    float tailEnv = 0.0f; // long open-hat tail
    int sampleCounter = 0;

    // 6-oscillator inharmonic bank — two detuned sets of 3 for density
    float osc1Phases[6] = { 0.0f };
    float osc2Phases[6] = { 0.0f };

    // Bandpass filter chain (two cascaded SVF stages)
    float bp1Lp = 0.0f, bp1Bp = 0.0f;
    float bp2Lp = 0.0f, bp2Bp = 0.0f;

    // Hi-shelf "air" filter
    float airLp = 0.0f;

    // DC / low-cut
    float dcState = 0.0f;

public:
    Param params[22] = {
        // --- PAGE 1: CORE ---
        { .label = "Duration", .unit = "ms", .value = 80.0f, .min = 5.0f, .max = 2000.0f, .step = 5.0f }, // 0
        { .label = "Open", .unit = "%", .value = 0.0f }, // 1  open-hat tail length (0=closed, 100=open)
        { .label = "Metal Freq", .unit = "Hz", .value = 3500.0f, .min = 800.0f, .max = 9000.0f, .step = 50.0f }, // 2
        { .label = "Inharmonic", .unit = "%", .value = 40.0f }, // 3  spread of the 6 partials
        { .label = "Detune", .unit = "%", .value = 20.0f }, // 4  osc2 detuned against osc1 for beating
        { .label = "FM Amt", .unit = "%", .value = 25.0f }, // 5  inter-oscillator FM → adds fizz/chaos
        { .label = "Tone", .unit = "%", .value = 50.0f }, // 6  sq→tri morph per oscillator
        { .label = "Attack", .unit = "ms", .value = 3.0f, .min = 0.5f, .max = 20.0f }, // 7  accent/transient snap length
        { .label = "Accent", .unit = "%", .value = 50.0f }, // 8  strength of initial metallic burst
        { .label = "Body Dec", .unit = "%", .value = 40.0f }, // 9  metallic body decay
        { .label = "Tail Dec", .unit = "%", .value = 60.0f }, // 10 tail decay (only audible when Open > 0)
        { .label = "Noise Mix", .unit = "%", .value = 20.0f }, // 11 blend of white noise into the metallic signal

        // --- PAGE 2: FILTER & FX ---
        { .label = "BP Freq", .unit = "Hz", .value = 5000.0f, .min = 1000.0f, .max = 14000.0f, .step = 100.0f }, // 12 bandpass centre
        { .label = "BP Width", .unit = "%", .value = 60.0f }, // 13 bandpass Q (width)
        { .label = "Air", .unit = "%", .value = 50.0f }, // 14 hi-shelf brightness boost
        { .label = "Low Cut", .unit = "%", .value = 80.0f }, // 15 HP to remove any low rumble
        { .label = "Drive", .unit = "%", .value = 15.0f }, // 16
        { .label = "Grit", .unit = "%", .value = 0.0f }, // 17 sample-hold crunch
        { .label = "Tightness", .unit = "%", .value = 50.0f }, // 18
        { .label = "Choke", .unit = "%", .value = 0.0f }, // 19 sharpens amp envelope tail curve
        { .label = "Level", .unit = "%", .value = 80.0f }, // 20
        { .label = "Reverb", .unit = "%", .value = 0.0f }, // 21
    };

    // --- References ---
    Param& duration = params[0];
    Param& openAmt = params[1];
    Param& metalFreq = params[2];
    Param& inharmonic = params[3];
    Param& detune = params[4];
    Param& fmAmt = params[5];
    Param& tone = params[6];
    Param& attackTime = params[7];
    Param& accent = params[8];
    Param& bodyDec = params[9];
    Param& tailDec = params[10];
    Param& noiseMix = params[11];
    Param& bpFreq = params[12];
    Param& bpWidth = params[13];
    Param& air = params[14];
    Param& lowCut = params[15];
    Param& drive = params[16];
    Param& grit = params[17];
    Param& tightness = params[18];
    Param& choke = params[19];
    Param& level = params[20];
    Param& reverb = params[21];

    // sample-hold state (grit)
    float sampleHoldState = 0.0f;
    int sampleHoldCounter = 0;

    DrumHiHat23(const float sampleRate, float* rvBuffer)
        : EngineBase(Drum, "HiHat23", params)
        , sampleRate(sampleRate)
        , reverbBuffer(rvBuffer)
    {
        init();
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        ampEnv = 1.0f;
        accentEnv = 1.0f;
        bodyEnv = 1.0f;
        tailEnv = 1.0f;
        sampleCounter = 0;
        bp1Lp = bp1Bp = bp2Lp = bp2Bp = dcState = airLp = 0.0f;

        float durSamples = std::max(1.0f, sampleRate * (duration.value * 0.001f));
        ampStep = 1.0f / durSamples;
    }

    float sampleImpl()
    {
        if (ampEnv < 0.001f) return applyRvb(0.0f);
        float currentAmp = ampEnv;
        ampEnv -= ampStep;

        // ----------------------------------------------------------------
        // 1. THREE-TIER ENVELOPE
        //    accentEnv : fast initial burst  (few ms)
        //    bodyEnv   : metallic ring body  (tens of ms)
        //    tailEnv   : open-hat sustain    (hundreds of ms)
        // ----------------------------------------------------------------
        float accentTime = attackTime.value * 0.001f;
        accentEnv *= Math::exp(-1.0f / (sampleRate * accentTime));

        float bodyTime = 0.005f + bodyDec.value * 0.002f;
        bodyEnv *= Math::exp(-1.0f / (sampleRate * bodyTime));

        float tailTime = 0.02f + tailDec.value * 0.015f;
        tailEnv *= Math::exp(-1.0f / (sampleRate * tailTime));

        // Choke squeezes the tail curve
        float chokeShape = 1.0f + choke.value * 0.06f;
        float shapedAmp = Math::pow(currentAmp, chokeShape);

        // ----------------------------------------------------------------
        // 2. INHARMONIC OSCILLATOR BANK
        //    Two sets of 6 oscillators. Set 2 is detuned against set 1.
        //    Classic cymbal ratios — none are integer multiples.
        // ----------------------------------------------------------------
        static const float ratios[6] = { 1.0f, 1.413f, 1.854f, 2.278f, 3.014f, 4.127f };

        float inhSpread = inharmonic.value * 12.0f; // Hz spread per partial
        float detuneHz = metalFreq.value * detune.value * 0.0005f; // set2 offset
        float fmStrength = fmAmt.value * 0.04f;
        float toneBlend = tone.value * 0.01f;

        float metalOut1 = 0.0f, metalOut2 = 0.0f;
        float lastSig1 = 0.0f, lastSig2 = 0.0f;

        for (int i = 0; i < 6; ++i) {
            // --- Set 1 ---
            float freq1 = metalFreq.value * ratios[i] + i * inhSpread;
            freq1 = std::min(freq1, sampleRate * 0.47f);
            osc1Phases[i] += freq1 / sampleRate + lastSig1 * fmStrength;
            if (osc1Phases[i] > 1.0f) osc1Phases[i] -= 1.0f;

            float sq1 = osc1Phases[i] > 0.5f ? 1.0f : -1.0f;
            float tri1 = 2.0f * std::abs(2.0f * (osc1Phases[i] - std::floor(osc1Phases[i] + 0.5f))) - 1.0f;
            float s1 = sq1 + (tri1 - sq1) * toneBlend;
            lastSig1 = s1;
            metalOut1 += (i % 2 == 0) ? s1 : -s1 * 0.8f;

            // --- Set 2 (detuned) ---
            float freq2 = (metalFreq.value + detuneHz) * ratios[i] + i * inhSpread;
            freq2 = std::min(freq2, sampleRate * 0.47f);
            osc2Phases[i] += freq2 / sampleRate + lastSig2 * fmStrength;
            if (osc2Phases[i] > 1.0f) osc2Phases[i] -= 1.0f;

            float sq2 = osc2Phases[i] > 0.5f ? 1.0f : -1.0f;
            float tri2 = 2.0f * std::abs(2.0f * (osc2Phases[i] - std::floor(osc2Phases[i] + 0.5f))) - 1.0f;
            float s2 = sq2 + (tri2 - sq2) * toneBlend;
            lastSig2 = s2;
            metalOut2 += (i % 2 == 0) ? s2 : -s2 * 0.8f;
        }

        // Normalise and combine sets — beating between them gives shimmer
        float metalSig = (metalOut1 + metalOut2) * (1.0f / 12.0f);

        // ----------------------------------------------------------------
        // 3. NOISE INJECTION
        //    Small amount of white noise blended IN to the metallic signal
        //    (not added separately — colours the metal, not a separate layer)
        // ----------------------------------------------------------------
        float rawNoise = Noise::sample();
        metalSig = metalSig * (1.0f - noiseMix.value * 0.01f)
            + rawNoise * (noiseMix.value * 0.01f);

        // ----------------------------------------------------------------
        // 4. ENVELOPE APPLICATION
        //    accent burst + body ring + open tail blended by openAmt
        // ----------------------------------------------------------------
        float openBlend = openAmt.value * 0.01f;
        float sig = metalSig * (accentEnv * pct(accent) * 1.5f // initial crack
                        + bodyEnv // metallic ring
                        + tailEnv * openBlend * 1.2f // open sustain
                    );

        // ----------------------------------------------------------------
        // 5. BANDPASS FILTER (two cascaded SVF stages)
        //    This is what makes it a cymbal, not a snare.
        //    Sculpts the spectrum around the metallic sweet-spot.
        // ----------------------------------------------------------------
        {
            float q = 0.3f + bpWidth.value * 0.008f; // wider Q = narrower band
            float fb = 1.0f / q;

            // Stage 1 centred at bpFreq
            float f1 = std::min(2.0f * Math::sin(M_PI * bpFreq.value / sampleRate), 0.49f);
            float hp1 = sig - bp1Bp * fb - bp1Lp;
            bp1Bp = std::clamp(bp1Bp + f1 * hp1, -4.0f, 4.0f);
            bp1Lp = std::clamp(bp1Lp + f1 * bp1Bp, -4.0f, 4.0f);

            // Stage 2 slightly offset for a broader, more natural shape
            float f2 = std::min(2.0f * Math::sin(M_PI * bpFreq.value * 1.22f / sampleRate), 0.49f);
            float hp2 = sig - bp2Bp * fb - bp2Lp;
            bp2Bp = std::clamp(bp2Bp + f2 * hp2, -4.0f, 4.0f);
            bp2Lp = std::clamp(bp2Lp + f2 * bp2Bp, -4.0f, 4.0f);

            // Blend dry + bandpassed (bandpass sculpts but doesn't kill everything)
            float wet = 0.4f + bpWidth.value * 0.005f;
            sig = sig * (1.0f - wet) + (bp1Bp + bp2Bp) * wet * 0.5f;
        }

        // ----------------------------------------------------------------
        // 6. LOW CUT (removes any low-end rumble / sub content)
        // ----------------------------------------------------------------
        {
            float hpCoeff = 0.001f + lowCut.value * 0.009f;
            dcState += hpCoeff * (sig - dcState);
            sig = sig - dcState;
        }

        // ----------------------------------------------------------------
        // 7. AIR / HI-SHELF (adds sparkle above bpFreq)
        // ----------------------------------------------------------------
        if (air.value > 0.0f) {
            float airCoeff = 0.3f + air.value * 0.006f;
            airLp += airCoeff * (sig - airLp);
            float shelf = sig - airLp; // high-shelf approximation
            sig += shelf * (air.value * 0.015f);
        }

        // ----------------------------------------------------------------
        // 8. DRIVE
        // ----------------------------------------------------------------
        if (drive.value > 0.0f) {
            sig = applyDrive(sig, pct(drive) * 3.0f);
        }

        // ----------------------------------------------------------------
        // 9. GRIT (sample-hold bit-crush)
        // ----------------------------------------------------------------
        if (grit.value > 0.0f) {
            int holdSamples = std::max(1, (int)(grit.value * 0.25f));
            if (++sampleHoldCounter >= holdSamples) {
                sampleHoldState = sig;
                sampleHoldCounter = 0;
            }
            sig = sampleHoldState;
        }

        // ----------------------------------------------------------------
        // 10. TIGHTNESS + FINAL AMP
        // ----------------------------------------------------------------
        float tightFactor = Math::pow(shapedAmp, 1.0f + pct(tightness) * 3.0f);

        sampleCounter++;
        return applyRvb(sig * tightFactor * velocity * pct(level));
    }

    float applyRvb(float out)
    {
        return applyReverb(out, reverb.value * 0.01f, reverbBuffer, reverbIndex);
    }
};