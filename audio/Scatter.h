#pragma once

#include "audio/effects/applyDecimator.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyReverb.h"
#include "audio/effects/applySampleReducer.h"
#include "audio/effects/applyWaveshape.h"
#include "audio/filterArray.h"
#include "helpers/clamp.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

#define MAX_SCATTER_SAMPLES 192000
#define FX_BUFFER_SIZE 131072

class Scatter {
public:
    Scatter()
    {
        std::memset(hist, 0, sizeof(hist));
        std::memset(reverbBuffer, 0, sizeof(reverbBuffer));
        std::memset(activeModes, 0, sizeof(activeModes));
        std::memset(modeMix, 0, sizeof(modeMix));
    }

    void toggleMode(int mode)
    {
        if (mode >= 0 && mode <= 3) {
            activeModes[mode] = !activeModes[mode];
        }
    }

    bool anyActive() const
    {
        return activeModes[0] || activeModes[1] || activeModes[2] || activeModes[3];
    }

    bool anyActiveOrReleasing() const
    {
        return anyActive() || modeMix[0] > 0.0f || modeMix[1] > 0.0f || modeMix[2] > 0.0f || modeMix[3] > 0.0f;
    }

    float process(float input, double samplesPerStep)
    {
        hist[writePtr] = input;
        writePtr = (writePtr + 1) % MAX_SCATTER_SAMPLES;

        double decay = (samplesPerStep > 0.0) ? (1.0 / (samplesPerStep * 1.5)) : 0.01;
        for (int i = 0; i < 4; i++) {
            if (activeModes[i]) {
                modeMix[i] = 1.0f;
            } else if (modeMix[i] > 0.0f) {
                modeMix[i] -= decay;
                if (modeMix[i] < 0.0f) {
                    modeMix[i] = 0.0f;
                }
            }
        }

        bool active = anyActiveOrReleasing();
        if (!active) {
            wasActive = false;
            return input;
        }

        if (!wasActive) {
            std::memset(reverbBuffer, 0, sizeof(reverbBuffer));
            wasActive = true;
        }

        float out = input;
        if (modeMix[0] > 0.0f) {
            // 0.15f is the baseline (minimum) cutoff frequency.
            // 0.3f is the modulation depth (how high the cutoff sweeps).
            // Other parameters you can tweak:
            //    Comb Feedback (0.8f): Determines the resonance/ringing intensity of the comb filter (higher = more metallic/string-like, lower = subtle chorus/flange).
            //    Delay Range (50.0 and 350.0): Changes the pitch range of the comb filter. Lower numbers result in a higher pitch, higher numbers result in a lower pitch.
            //    Filter Resonance (0.4f): Adjusts the peak of the SVF filter (higher = more squelchy/acid-like).
            //    LFO Rate (4.0): Controls the speed of the sweep relative to the step length.
            //    Output Mix (0.7f and 0.3f): The balance between Low-Pass warmth (lp[0]) and Band-Pass character (bp[0]).

            lfoPhase += 1.0 / (samplesPerStep > 0.0 ? samplesPerStep * 4.0 : 40000.0);
            if (lfoPhase >= 1.0) lfoPhase -= 1.0;
            double lfoVal = sin(lfoPhase * 2.0 * 3.14159265358979323846);
            double delaySamples = 50.0 + 350.0 * (0.5 + 0.5 * lfoVal);
            float delayedSample = readHistAtDelay(delaySamples);
            float feedback = 0.8f;
            float combOut = input + feedback * delayedSample;

            float cutoff = 0.15f + 0.3f * (float)(0.5 + 0.5 * lfoVal);
            filter.setCutoff(cutoff);
            filter.setResonance(0.4f);
            filter.setSampleData(combOut, 0);

            float fxOut = filter.lp[0] * 0.7f + filter.bp[0] * 0.3f;
            out = modeMix[0] * fxOut + (1.0f - modeMix[0]) * out;
        }

        if (modeMix[1] > 0.0f) {
            // Multiply by gaterRateMultiplier to speed up or slow down the gate rate
            double phaseStep = 1.0 / (samplesPerStep > 0.0 ? samplesPerStep : 44100.0);
            gaterPhase += phaseStep * gaterRateMultiplier;

            // Handle wrapping for rates higher than 1.0
            if (gaterPhase >= 1.0) {
                gaterPhase = std::fmod(gaterPhase, 1.0);
            }

            // Strict square wave chopping (50% duty cycle)
            float gateVal = (gaterPhase < 0.5) ? 1.0f : 0.0f;
            float fxOut = out * gateVal;

            float ajustedMix = modeMix[1] * 0.6f;
            out = ajustedMix * fxOut + (1.0f - ajustedMix) * out;
        }

        if (modeMix[2] > 0.0f) {
            float fxOut = applyDecimator(out, 0.5, fDataFx, iDataFx);
            out = modeMix[2] * fxOut + (1.0f - modeMix[2]) * out;
        }
        if (modeMix[3] > 0.0f) {
            float fxOut = applyReverb(out, 0.7f, reverbBuffer, reverbIndex);
            out = modeMix[3] * fxOut + (1.0f - modeMix[3]) * out;
        }

        return out;
    }

private:
    float hist[MAX_SCATTER_SAMPLES];
    size_t writePtr = 0;

    bool activeModes[8];
    float modeMix[4];
    float reverbBuffer[FX_BUFFER_SIZE];
    int reverbIndex = 0;
    EffectFilterArray<1> filter;
    double lfoPhase = 0.0;
    double gaterPhase = 0.0;
    bool wasActive = false;

    // 0.25f: 1/4 note rate (Slower, 1 gate cycle every 4 steps)
    // 1.0f: Straight 1/16th notes (1 cycle per step)
    // 2.0f: 1/32nd notes (Fast, 2 cycles per step)
    // 1.5f: Triplets (1.5 cycles per step)
    float gaterRateMultiplier = 0.25f;

    float readHistAtDelay(double delaySamples)
    {
        double ptr = (double)writePtr - delaySamples;
        while (ptr < 0.0)
            ptr += MAX_SCATTER_SAMPLES;
        while (ptr >= MAX_SCATTER_SAMPLES)
            ptr -= MAX_SCATTER_SAMPLES;
        size_t i0 = (size_t)ptr;
        size_t i1 = (i0 + 1) % MAX_SCATTER_SAMPLES;
        float frac = (float)(ptr - i0);
        return hist[i0] + frac * (hist[i1] - hist[i0]);
    }

    float fDataFx = 0.0;
    int iDataFx = 0;
};