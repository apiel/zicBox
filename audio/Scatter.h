#pragma once

#include "helpers/clamp.h"
#include <algorithm>
#include <cmath>
#include <cstring>

#define MAX_SCATTER_SAMPLES 192000

class Scatter {
public:
    Scatter()
    {
        std::memset(histDrums, 0, sizeof(histDrums));
        std::memset(histSynth, 0, sizeof(histSynth));
        std::memset(grainDrums, 0, sizeof(grainDrums));
        std::memset(grainSynth, 0, sizeof(grainSynth));
    }

    float process(float drumIn, float synthIn, int mode, double samplesPerStep)
    {
        // 1. Record both streams
        histDrums[writePtr] = drumIn;
        histSynth[writePtr] = synthIn;
        writePtr = (writePtr + 1) % MAX_SCATTER_SAMPLES;

        if (mode == 0) {
            isCaptured = false;
            return drumIn + synthIn;
        }

        // 2. Capture both on Trigger
        if (!isCaptured) {
            captureLen = (size_t)(samplesPerStep * 16); // Capture 1 full 16-step loop
            if (captureLen > MAX_SCATTER_SAMPLES) captureLen = MAX_SCATTER_SAMPLES;

            for (size_t i = 0; i < captureLen; i++) {
                size_t hPtr = (writePtr + MAX_SCATTER_SAMPLES - captureLen + i) % MAX_SCATTER_SAMPLES;
                grainDrums[i] = histDrums[hPtr];
                grainSynth[i] = histSynth[hPtr];
            }
            isCaptured = true;
            readPtrDrums = 0.0;
            readPtrSynth = 0.0;
        }

        return apply12Modes(mode, samplesPerStep);
    }

private:
    float histDrums[MAX_SCATTER_SAMPLES], histSynth[MAX_SCATTER_SAMPLES];
    float grainDrums[MAX_SCATTER_SAMPLES], grainSynth[MAX_SCATTER_SAMPLES];
    size_t writePtr = 0;
    double readPtrDrums = 0.0, readPtrSynth = 0.0;
    size_t captureLen = 0;
    bool isCaptured = false;

    // Helper for linear interpolation
    float readBuffer(float* buf, double& ptr)
    {
        size_t i0 = (size_t)ptr;
        size_t i1 = (i0 + 1) % captureLen;
        float frac = (float)(ptr - i0);
        return buf[i0] + frac * (buf[i1] - buf[i0]);
    }

    float apply12Modes(int mode, double sPS)
    {
        float outD = 0, outS = 0;
        double speedD = 1.0, speedS = 1.0;

        switch (mode) {
        case 1: // "The Classic": Drum 1-step retrig, Synth normal loop
            readPtrDrums = fmod(readPtrDrums + 1.0, sPS);
            readPtrSynth = fmod(readPtrSynth + 1.0, (double)captureLen);
            break;

        case 2: // "High Octave": Both double speed (1 octave up)
            speedD = 2.0;
            speedS = 2.0;
            readPtrDrums = fmod(readPtrDrums + speedD, (double)captureLen);
            readPtrSynth = fmod(readPtrSynth + speedS, (double)captureLen);
            break;

        case 3: // "Drum Breaker": Drums 1/2 step retrig, Synth 1/2 speed (octave down)
            readPtrDrums = fmod(readPtrDrums + 1.0, sPS * 0.5);
            speedS = 0.5;
            readPtrSynth = fmod(readPtrSynth + speedS, (double)captureLen);
            break;

        case 4: // "Riser": Synth pitches up continuously, Drums 1/4 step stutter
            static float riserSpeed = 1.0f;
            riserSpeed += 0.0001f; // Slowly pitch up
            readPtrDrums = fmod(readPtrDrums + 1.0, sPS * 0.25);
            readPtrSynth = fmod(readPtrSynth + riserSpeed, (double)captureLen);
            if (riserSpeed > 4.0f) riserSpeed = 1.0f;
            break;

        case 5: // "Half-Time": Everything at 0.5 speed
            speedD = 0.5;
            speedS = 0.5;
            readPtrDrums = fmod(readPtrDrums + speedD, (double)captureLen);
            readPtrSynth = fmod(readPtrSynth + speedS, (double)captureLen);
            break;

        case 6: // "Syncopated": Drums normal, Synth shifted by 1.5 steps
            readPtrDrums = fmod(readPtrDrums + 1.0, (double)captureLen);
            readPtrSynth = fmod(readPtrDrums + (sPS * 1.5), (double)captureLen);
            break;

        case 7: // "Machine Gun": Both 1/16th note stutter (very fast)
            readPtrDrums = fmod(readPtrDrums + 1.0, sPS * 0.125);
            readPtrSynth = fmod(readPtrSynth + 1.0, sPS * 0.125);
            break;

        case 8: // "Bass Drop": Drums stop (mute), Synth pitches down to 0
            static float fall = 1.0f;
            fall = std::max(0.0f, fall - 0.001f);
            return readBuffer(grainSynth, readPtrSynth = fmod(readPtrSynth + fall, (double)captureLen));

        case 9: // "Robot": Bitcrushed drums, Synth fixed at very small grain
            readPtrDrums = fmod(readPtrDrums + 1.0, (double)captureLen);
            readPtrSynth = fmod(readPtrSynth + 1.0, 100.0); // 100 samples = metallic tone
            break;

        case 10: // "Triplets": Drums into 1/3 step loop
            readPtrDrums = fmod(readPtrDrums + 1.0, sPS * 0.333);
            readPtrSynth = fmod(readPtrSynth + 1.0, (double)captureLen);
            break;

        case 11: // "Micro-pitch": Drums normal, Synth vibrating pitch
            speedS = 1.0 + (0.05 * sin(readPtrSynth * 0.01));
            readPtrDrums = fmod(readPtrDrums + 1.0, (double)captureLen);
            readPtrSynth = fmod(readPtrSynth + speedS, (double)captureLen);
            break;

        case 12: // "The End": Everything retrigs at 4 steps (1 bar)
            readPtrDrums = fmod(readPtrDrums + 1.0, sPS * 4);
            readPtrSynth = readPtrDrums;
            break;

        default:
            readPtrDrums = fmod(readPtrDrums + 1.0, (double)captureLen);
            readPtrSynth = fmod(readPtrSynth + 1.0, (double)captureLen);
            break;
        }

        outD = readBuffer(grainDrums, readPtrDrums);
        outS = readBuffer(grainSynth, readPtrSynth);

        return outD + outS;
    }
};
