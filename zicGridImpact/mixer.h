#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>

class Mixer {
public:
    // Master Page 1: Volume & Mixer Levels
    float volume = 0.70f; // 0.0 to 1.0 (Drive active when > 0.60)
    float kickLevel = 0.65f; // 0.0 to 1.0
    float synth1Level = 0.80f; // 0.0 to 1.0
    float synth2Level = 0.80f; // 0.0 to 1.0
    float chaosLevel = 0.80f; // 0.0 to 1.0

    // Master Page 2: Shared Delay
    float delayTimeMs = 250.0f; // 10 to 1000 ms
    float delayFeedback = 0.35f; // 0.0 to 0.95

    // Master Drive & Filter
    float masterCutoff = 1.0f; // 0.0 to 1.0
    float masterResonance = 0.0f; // 0.0 to 0.95

    // Live Output Audio Channel Peak Levels for VU Metering
    std::atomic<float> peakKick { 0.0f };
    std::atomic<float> peakSynth1 { 0.0f };
    std::atomic<float> peakSynth2 { 0.0f };
    std::atomic<float> peakChaos { 0.0f };
    std::atomic<float> peakMaster { 0.0f };

private:
    static const int DELAY_BUF_SIZE = 48000;
    float delayBuffer[DELAY_BUF_SIZE] = { 0.0f };
    int delayWrite = 0;
    float sampleRate = 44100.0f;

public:
    Mixer(float sr = 44100.0f)
        : sampleRate(sr)
    {
    }

    float process(float kickSample, float synth1Sample, float synth1DelaySend, float synth2Sample, float synth2DelaySend, float chaosSample = 0.0f, float chaosDelaySend = 0.0f)
    {
        float k = kickSample * kickLevel;
        float s1 = synth1Sample * synth1Level;
        float s2 = synth2Sample * synth2Level;
        float ch = chaosSample * chaosLevel;

        // 1. Process Shared Master Delay Line for Synths & Chaos
        float delaySendSum = (s1 * (synth1DelaySend * 0.01f)) + (s2 * (synth2DelaySend * 0.01f)) + (ch * (chaosDelaySend * 0.01f));
        int delaySamples = std::clamp((int)(delayTimeMs * 0.001f * sampleRate), 1, DELAY_BUF_SIZE - 1);
        int readPos = (delayWrite - delaySamples + DELAY_BUF_SIZE) % DELAY_BUF_SIZE;
        float delayOut = delayBuffer[readPos];

        delayBuffer[delayWrite] = delaySendSum + (delayOut * delayFeedback);
        delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;

        float summed = (k + s1 + s2 + ch + delayOut) / 4.0f;

        // 2. Master Volume Dual Function (0.0 to 0.60 clean, >0.60 adds Overdrive Saturation)
        float output = 0.0f;
        if (volume <= 0.60f) {
            float gain = volume / 0.60f;
            output = summed * gain;
        } else {
            float overdriveAmt = (volume - 0.60f) / 0.40f;
            float gain = 1.0f + (overdriveAmt * 3.0f);
            float driven = std::tanh(summed * gain);
            output = driven;
        }

        // 3. Live Output Audio Channel Peak Level Followers
        float kAbs = std::abs(k);
        float pK = peakKick.load(std::memory_order_relaxed);
        if (kAbs > pK) peakKick.store(kAbs, std::memory_order_relaxed);
        else peakKick.store(pK * 0.9994f, std::memory_order_relaxed);

        float s1Abs = std::abs(s1);
        float pS1 = peakSynth1.load(std::memory_order_relaxed);
        if (s1Abs > pS1) peakSynth1.store(s1Abs, std::memory_order_relaxed);
        else peakSynth1.store(pS1 * 0.9994f, std::memory_order_relaxed);

        float s2Abs = std::abs(s2);
        float pS2 = peakSynth2.load(std::memory_order_relaxed);
        if (s2Abs > pS2) peakSynth2.store(s2Abs, std::memory_order_relaxed);
        else peakSynth2.store(pS2 * 0.9994f, std::memory_order_relaxed);

        float chAbs = std::abs(ch);
        float pCh = peakChaos.load(std::memory_order_relaxed);
        if (chAbs > pCh) peakChaos.store(chAbs, std::memory_order_relaxed);
        else peakChaos.store(pCh * 0.9994f, std::memory_order_relaxed);

        float mAbs = std::abs(output);
        float pM = peakMaster.load(std::memory_order_relaxed);
        if (mAbs > pM) peakMaster.store(mAbs, std::memory_order_relaxed);
        else peakMaster.store(pM * 0.9994f, std::memory_order_relaxed);

        return std::clamp(output, -1.0f, 1.0f);
    }
};
