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
    float params[4][4];
    int latestActiveMode = -1;

    Scatter()
    {
        std::memset(hist, 0, sizeof(hist));
        std::memset(reverbBuffer, 0, sizeof(reverbBuffer));
        std::memset(activeModes, 0, sizeof(activeModes));
        std::memset(modeMix, 0, sizeof(modeMix));
        sampleSqueeze = 0.0f;
        samplePosition = 0;
        gaterGateSmoothed = 0.0f;
        gaterLPState = 0.0f;
        for (int i = 0; i < 4; i++) {
            resetParams(i);
        }
    }

    void resetParams(int mode)
    {
        if (mode == 0) {
            params[0][0] = 0.8f;  // Comb FB
            params[0][1] = 4.0f;  // LFO Rate
            params[0][2] = 0.4f;  // Reso
            params[0][3] = 0.7f;  // LP Mix
        } else if (mode == 1) {
            params[1][0] = 0.25f; // Rate
            params[1][1] = 0.5f;  // Duty
            params[1][2] = 0.6f;  // Depth
            params[1][3] = 0.05f; // Slew
        } else if (mode == 2) {
            params[2][0] = 0.5f;  // Decimate
            params[2][1] = 0.0f;  // Squeeze
            params[2][2] = 0.0f;  // Drive
            params[2][3] = 0.0f;  // Waveshape
        } else if (mode == 3) {
            params[3][0] = 0.7f;  // Reverb
            params[3][1] = 0.0f;  // Delay Time
            params[3][2] = 0.0f;  // Delay FB
            params[3][3] = 0.0f;  // Delay Mix
        }
    }

    const char* getParamName(int mode, int paramIdx) const
    {
        static const char* names[4][4] = {
            { "Comb FB", "LFO Rate", "Reso", "LP Mix" },
            { "Rate", "Duty", "Depth", "Slew" },
            { "Decimate", "Squeeze", "Waveshape", "Drive" },
            { "Reverb", "Dly Time", "Dly FB", "Dly Mix" }
        };
        if (mode >= 0 && mode < 4 && paramIdx >= 0 && paramIdx < 4) {
            return names[mode][paramIdx];
        }
        return "";
    }

    void tweakParam(int mode, int paramIdx, int delta, bool shifted)
    {
        if (mode < 0 || mode >= 4 || paramIdx < 0 || paramIdx >= 4) return;

        float change = (shifted ? 0.05f : 0.01f) * delta;
        if (mode == 0) {
            if (paramIdx == 0) {
                params[0][0] = std::clamp(params[0][0] + change, 0.0f, 0.95f);
            } else if (paramIdx == 1) {
                float lfoChange = (shifted ? 1.0f : 0.1f) * delta;
                params[0][1] = std::clamp(params[0][1] + lfoChange, 0.1f, 20.0f);
            } else if (paramIdx == 2) {
                params[0][2] = std::clamp(params[0][2] + change, 0.0f, 0.99f);
            } else if (paramIdx == 3) {
                params[0][3] = std::clamp(params[0][3] + change, 0.0f, 1.0f);
            }
        } else if (mode == 1) {
            if (paramIdx == 0) {
                float rateChange = (shifted ? 0.25f : 0.05f) * delta;
                params[1][0] = std::clamp(params[1][0] + rateChange, 0.05f, 5.0f);
            } else if (paramIdx == 1) {
                params[1][1] = std::clamp(params[1][1] + change, 0.05f, 0.95f);
            } else if (paramIdx == 2) {
                params[1][2] = std::clamp(params[1][2] + change, 0.0f, 1.0f);
            } else if (paramIdx == 3) {
                params[1][3] = std::clamp(params[1][3] + change, 0.0f, 0.99f);
            }
        } else if (mode == 2) {
            params[2][paramIdx] = std::clamp(params[2][paramIdx] + change, 0.0f, 1.0f);
        } else if (mode == 3) {
            if (paramIdx == 0) {
                params[3][0] = std::clamp(params[3][0] + change, 0.0f, 1.0f);
            } else if (paramIdx == 1) {
                float dtChange = (shifted ? 0.5f : 0.05f) * delta;
                params[3][1] = std::clamp(params[3][1] + dtChange, 0.0f, 4.0f);
            } else if (paramIdx == 2) {
                params[3][2] = std::clamp(params[3][2] + change, 0.0f, 0.95f);
            } else if (paramIdx == 3) {
                params[3][3] = std::clamp(params[3][3] + change, 0.0f, 1.0f);
            }
        }
    }

    void toggleMode(int mode)
    {
        if (mode >= 0 && mode <= 3) {
            activeModes[mode] = !activeModes[mode];
            if (activeModes[mode]) {
                latestActiveMode = mode;
                resetParams(mode);
                if (mode == 1) {
                    gaterPhase = 0.0;
                    gaterGateSmoothed = 0.0f;
                }
            }
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
            gaterPhase = 0.0;
            gaterGateSmoothed = 0.0f;
            wasActive = true;
        }

        float out = input;
        if (modeMix[0] > 0.0f) {
            float feedback = params[0][0];
            float lfoRate = params[0][1];
            float resonance = params[0][2];
            float lpMix = params[0][3];

            lfoPhase += 1.0 / (samplesPerStep > 0.0 ? samplesPerStep * lfoRate : 40000.0);
            if (lfoPhase >= 1.0) lfoPhase -= 1.0;
            double lfoVal = sin(lfoPhase * 2.0 * 3.14159265358979323846);
            double delaySamples = 50.0 + 350.0 * (0.5 + 0.5 * lfoVal);
            float delayedSample = readHistAtDelay(delaySamples);
            float combOut = input + feedback * delayedSample;

            float cutoff = 0.15f + 0.3f * (float)(0.5 + 0.5 * lfoVal);
            filter.setCutoff(cutoff);
            filter.setResonance(resonance);
            filter.setSampleData(combOut, 0);

            float fxOut = filter.lp[0] * lpMix + filter.bp[0] * (1.0f - lpMix);
            out = modeMix[0] * fxOut + (1.0f - modeMix[0]) * out;
        }

        if (modeMix[1] > 0.0f) { // Gater FX
            float rate = params[1][0];
            float duty = params[1][1];
            float depth = params[1][2];
            float slew = params[1][3];

            double phaseStep = 1.0 / (samplesPerStep > 0.0 ? samplesPerStep : 44100.0);
            gaterPhase += phaseStep * rate;

            if (gaterPhase >= 1.0) {
                gaterPhase = std::fmod(gaterPhase, 1.0);
            }

            float gateTarget = (gaterPhase < duty) ? 1.0f : 0.0f;
            
            // Map slew: 0.0 is fastest, 0.99 is slowest.
            float gateCoeff = 0.0001f + std::pow(1.0f - slew, 3.0f) * 0.1999f;
            gaterGateSmoothed += (gateTarget - gaterGateSmoothed) * gateCoeff;

            // Low Pass Gate (LPG) emulation: sweep cutoff with gate.
            float lpCoeff = 0.005f + 0.995f * gaterGateSmoothed;
            gaterLPState += lpCoeff * (out - gaterLPState);

            float fxOut = gaterLPState * gaterGateSmoothed;

            float adjustedMix = modeMix[1] * depth;
            out = adjustedMix * fxOut + (1.0f - adjustedMix) * out;
        }

        if (modeMix[2] > 0.0f) {
            float decimate = params[2][0];
            float squeeze = params[2][1];
            float waveshape = params[2][2];
            float drive = params[2][3];

            float fxOut = out;
            if (decimate > 0.0f) {
                fxOut = applyDecimator(fxOut, decimate, fDataFx, iDataFx);
            }
            if (squeeze > 0.0f) {
                fxOut = applySampleReducer(fxOut, squeeze, sampleSqueeze, samplePosition);
            }
            if (drive > 0.0f) {
                fxOut = applyDrive(fxOut, drive);
            }
            if (waveshape > 0.0f) {
                fxOut = applyWaveshape(fxOut, waveshape);
            }

            out = modeMix[2] * fxOut + (1.0f - modeMix[2]) * out;
        }

        if (modeMix[3] > 0.0f) {
            float reverbMix = params[3][0];
            float delayTime = params[3][1];
            float delayFB = params[3][2];
            float delayMix = params[3][3];

            float fxOut = out;
            if (reverbMix > 0.0f) {
                fxOut = applyReverb(fxOut, reverbMix, reverbBuffer, reverbIndex);
            }

            if (delayMix > 0.0f && delayTime > 0.0f) {
                double delaySamples = delayTime * (samplesPerStep > 0.0 ? samplesPerStep : 44100.0 / 4.0);
                float delayed = readHistAtDelay(delaySamples);
                float delayOut = (fxOut * (1.0f - delayMix)) + (delayed * delayMix);
                fxOut = delayOut;
                size_t lastWritten = (writePtr + MAX_SCATTER_SAMPLES - 1) % MAX_SCATTER_SAMPLES;
                hist[lastWritten] += delayed * delayFB;
            }

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
    float gaterGateSmoothed = 0.0f;
    float gaterLPState = 0.0f;
    bool wasActive = false;

    float sampleSqueeze = 0.0f;
    int samplePosition = 0;

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