#pragma once

#include "audio/engines/EngineBase.h"
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <cstring>

/**
 * Acid32: Classic & Tekno Acid Synthesizer with Integrated Delay & Reverb.
 *
 * Signal Flow:
 *   [OSC]  -> Morphing Wave (Tri -> Saw -> Sq -> Fold) + Unison + Sub-Osc
 *   [FILT] -> 4-Pole 24dB Ladder (Cutoff, Reso, EnvMod, Accent, HPF)
 *   [ENV]  -> Filter Decay, Accent, Pitch Envelope, Amp Decay & Sustain
 *   [FX]   -> Pre-Drive, Post-Distortion, Stereo Ping-Pong / Tape Delay, Reverb
 *
 * Exactly 32 Parameters arranged in 4 cards (4 cols x 2 rows = 8 pots each):
 *   Block 0: OSCILLATOR (8 pots)
 *   Block 1: LADDER FILTER (8 pots)
 *   Block 2: ENVELOPES (8 pots)
 *   Block 3: DRIVE & SPATIAL FX (8 pots: Drive, Dist, DistType, DelayTime, DelayFdbk, DelayMix, RevDecay, RevMix)
 */
class Acid32 : public EngineBase<Acid32> {
public:
    static constexpr int PARAM_COUNT = 32;
    Param params[PARAM_COUNT];

    // ==========================================
    // BLOCK 0: OSCILLATOR & TONE (Pots 0..7)
    // ==========================================
    Param& tune       = addParam({ .key = "tune",       .label = "Tune",       .unit = "st", .value = 0.0f,  .min = -24.0f, .max = 24.0f,  .step = 1.0f });
    Param& fine       = addParam({ .key = "fine",       .label = "Fine",       .value = 0.0f,  .min = -1.0f,  .max = 1.0f,   .step = 0.01f });
    Param& color      = addParam({ .key = "color",      .label = "Wave",       .unit = "%",  .value = 33.0f, .min = 0.0f,   .max = 100.0f, .step = 1.0f });
    Param& unison     = addParam({ .key = "unison",     .label = "Unison",     .value = 1.0f,  .min = 1.0f,   .max = 3.0f,   .step = 1.0f });
    Param& detune     = addParam({ .key = "detune",     .label = "Detune",     .unit = "ct", .value = 15.0f, .min = 0.0f,   .max = 60.0f,  .step = 0.5f });
    Param& sub        = addParam({ .key = "sub",        .label = "Sub Level",  .unit = "%",  .value = 0.0f,  .min = 0.0f,   .max = 100.0f, .step = 1.0f });
    Param& subOct     = addParam({ .key = "subOct",     .label = "Sub Oct",    .value = 1.0f,  .min = 1.0f,   .max = 2.0f,   .step = 1.0f });
    Param& glide      = addParam({ .key = "glide",      .label = "Slide",      .unit = "ms", .value = 55.0f, .min = 5.0f,   .max = 500.0f, .step = 1.0f });

    // ==========================================
    // BLOCK 1: LADDER FILTER (Pots 8..15)
    // ==========================================
    Param& cutoff     = addParam({ .key = "cutoff",     .label = "Cutoff",     .unit = "%",  .value = 45.0f, .min = 0.0f,   .max = 100.0f, .step = 1.0f });
    Param& reso       = addParam({ .key = "reso",       .label = "Reso",       .unit = "%",  .value = 65.0f, .min = 0.0f,   .max = 100.0f, .step = 1.0f });
    Param& envMod     = addParam({ .key = "envMod",     .label = "Env Mod",    .unit = "%",  .value = 60.0f, .min = -100.0f,.max = 100.0f, .step = 1.0f });
    Param& accent     = addParam({ .key = "accent",     .label = "Accent",     .unit = "%",  .value = 75.0f, .min = 0.0f,   .max = 100.0f, .step = 1.0f });
    Param& hpfCut     = addParam({ .key = "hpfCut",     .label = "HP Filter",  .unit = "Hz", .value = 25.0f, .min = 20.0f,  .max = 1000.0f,.step = 5.0f });
    Param& lfoRate    = addParam({ .key = "lfoRate",    .label = "LFO Rate",   .unit = "Hz", .value = 2.0f,  .min = 0.1f,   .max = 25.0f,  .step = 0.1f });
    Param& lfoToCut   = addParam({ .key = "lfoToCut",   .label = "LFO>Cut",    .unit = "%",  .value = 0.0f,  .min = -100.0f,.max = 100.0f, .step = 1.0f });
    Param& lfoToWave  = addParam({ .key = "lfoToWave",  .label = "LFO>Wave",   .unit = "%",  .value = 0.0f,  .min = -100.0f,.max = 100.0f, .step = 1.0f });

    // ==========================================
    // BLOCK 2: ENVELOPES (Pots 16..23)
    // ==========================================
    Param& fDecay     = addParam({ .key = "fDecay",     .label = "F.Decay",    .unit = "ms", .value = 260.0f,.min = 20.0f,  .max = 2500.0f,.step = 5.0f });
    Param& pDepth     = addParam({ .key = "pDepth",     .label = "P.Depth",    .unit = "st", .value = 0.0f,  .min = -24.0f, .max = 24.0f,  .step = 0.5f });
    Param& pDecay     = addParam({ .key = "pDecay",     .label = "P.Decay",    .unit = "ms", .value = 25.0f, .min = 5.0f,   .max = 300.0f, .step = 1.0f });
    Param& attack     = addParam({ .key = "attack",     .label = "Attack",     .unit = "ms", .value = 2.0f,  .min = 1.0f,   .max = 500.0f, .step = 1.0f });
    Param& decay      = addParam({ .key = "decay",      .label = "Decay",      .unit = "ms", .value = 350.0f,.min = 20.0f,  .max = 3000.0f,.step = 5.0f });
    Param& sustain    = addParam({ .key = "sustain",    .label = "Sustain",    .unit = "%",  .value = 70.0f, .min = 0.0f,   .max = 100.0f, .step = 1.0f });
    Param& release    = addParam({ .key = "release",    .label = "Release",    .unit = "ms", .value = 50.0f, .min = 5.0f,   .max = 2000.0f,.step = 5.0f });
    Param& drive      = addParam({ .key = "drive",      .label = "Drive",      .unit = "%",  .value = 0.0f,  .min = 0.0f,   .max = 100.0f, .step = 1.0f });

    // ==========================================
    // BLOCK 3: TEKNO DRIVE & SPATIAL FX (Pots 24..31)
    // ==========================================
    Param& dist       = addParam({ .key = "dist",       .label = "Distortion", .unit = "%",  .value = 0.0f,  .min = 0.0f,   .max = 100.0f, .step = 1.0f });
    Param& distType   = addParam({ .key = "distType",   .label = "Dist Type",  .value = 0.0f,  .min = 0.0f,   .max = 3.0f,   .step = 1.0f }); // Soft, Hard, Fold, Fuzz
    Param& delayTime  = addParam({ .key = "delayTime",  .label = "Delay Time", .unit = "ms", .value = 250.0f,.min = 20.0f,  .max = 800.0f, .step = 5.0f });
    Param& delayFdbk  = addParam({ .key = "delayFdbk",  .label = "Delay Fdbk", .unit = "%",  .value = 40.0f, .min = 0.0f,   .max = 95.0f,  .step = 1.0f });
    Param& delayMix   = addParam({ .key = "delayMix",   .label = "Delay Mix",  .unit = "%",  .value = 0.0f,  .min = 0.0f,   .max = 100.0f, .step = 1.0f });
    Param& revDecay   = addParam({ .key = "revDecay",   .label = "Rev Size",   .unit = "%",  .value = 50.0f, .min = 10.0f,  .max = 98.0f,  .step = 1.0f });
    Param& revDamp    = addParam({ .key = "revDamp",    .label = "Rev Damp",   .unit = "%",  .value = 35.0f, .min = 0.0f,   .max = 90.0f,  .step = 1.0f });
    Param& revMix     = addParam({ .key = "revMix",     .label = "Reverb Mix", .unit = "%",  .value = 0.0f,  .min = 0.0f,   .max = 100.0f, .step = 1.0f });

    // Live display metrics
    float livePre = 0.0f;
    float livePost = 0.0f;
    float liveCutoffHz = 440.0f;
    float liveReso = 0.0f;
    float liveEnvAmp = 0.0f;
    float liveEnvFilter = 0.0f;
    float liveLfoVal = 0.0f;
    int currentMidiNote = 36;

    explicit Acid32(const float sampleRate = 44100.0f)
        : EngineBase(Synth, "Acid32", params)
        , sampleRate(sampleRate)
    {
        initEffects();
        reset();
    }

    void reset()
    {
        gate = false;
        idle = true;
        isAccent = false;
        curPitchSt = targetPitchSt = 0.0f;
        pitchEnv = 0.0f;
        fEnvVal = 0.0f;
        ampEnvVal = 0.0f;
        ampStage = 3;
        controlCountdown = 0;
        phase[0] = phase[1] = phase[2] = 0.0f;
        subPhase = 0.0f;
        lfoPhase = 0.0f;
        s1 = s2 = s3 = s4 = 0.0f;
        hpS1 = 0.0f;
        inc0 = inc1 = 0.0f;
        snap = true;
    }

    void noteOn(uint8_t note, float velocity = 1.0f, bool slide = false, bool acc = false)
    {
        bool legato = slide && gate && !idle;
        currentMidiNote = note;
        targetPitchSt = (float)(note - 69) + tune.value + fine.value;
        isAccent = acc || (velocity > 0.85f);

        if (!legato) {
            if (idle) {
                s1 = s2 = s3 = s4 = 0.0f;
                hpS1 = 0.0f;
            }
            curPitchSt = targetPitchSt;
            pitchEnv = pDepth.value;
            fEnvVal = 1.0f; // Snappy 303 filter envelope starts at 1
            ampStage = 0;   // Amp Attack
            controlCountdown = 0;
            snap = true;
        }

        gate = true;
        idle = false;
    }

    void noteOnImpl(uint8_t note, float velocity = 1.0f)
    {
        noteOn(note, velocity, false, false);
    }

    void noteOff(uint8_t note = 0)
    {
        (void)note;
        gate = false;
        ampStage = 2; // Release
    }

    void noteOffImpl(uint8_t note = 0)
    {
        noteOff(note);
    }

    float sampleImpl()
    {
        if (idle && !hasFxTail()) {
            livePre = livePost = 0.0f;
            return 0.0f;
        }

        if (controlCountdown <= 0) {
            updateControl();
            controlCountdown = CTRL_RATE;
        }
        controlCountdown--;

        float stepFrac = (float)(CTRL_RATE - controlCountdown) * (1.0f / (float)CTRL_RATE);
        float baseInc = inc0 + (inc1 - inc0) * stepFrac;

        // Render Oscillator (PolyBLEP + Unison)
        int nVoices = (int)std::clamp(unison.value, 1.0f, 3.0f);
        float waveNorm = std::clamp(color.value * 0.01f + liveLfoVal * (lfoToWave.value * 0.005f), 0.0f, 1.0f);
        float oscMix = 0.0f;

        for (int v = 0; v < nVoices; ++v) {
            float vRatio = voiceRatios[v];
            float dt = baseInc * vRatio;
            phase[v] += dt;
            if (phase[v] >= 1.0f) phase[v] -= 1.0f;
            oscMix += renderOscWave(phase[v], dt, waveNorm);
        }

        if (nVoices > 1) {
            oscMix *= (1.0f / std::sqrt((float)nVoices));
        }

        // Sub Oscillator
        float subGain = sub.value * 0.01f;
        if (subGain > 0.001f) {
            float subDiv = (subOct.value >= 1.5f) ? 0.25f : 0.5f;
            subPhase += baseInc * subDiv;
            if (subPhase >= 1.0f) subPhase -= 1.0f;
            oscMix += subGain * std::sin(subPhase * 6.2831853f);
        }

        // Pre-Filter Warm Saturation
        float driveVal = drive.value * 0.01f;
        float u = oscMix * 0.85f;
        livePre = u;
        if (driveVal > 0.001f) {
            float driveG = 1.0f + 8.0f * driveVal * driveVal;
            float normVal = 1.0f / std::tanh(driveG);
            u = std::tanh(u * driveG) * normVal;
        }

        // 4-Pole ZDF Ladder Filter
        float omg = 1.0f - G;
        float sg1 = omg * s1;
        float sg2 = omg * s2;
        float sg3 = omg * s3;
        float sg4 = omg * s4;
        float sum = G3 * sg1 + G2 * sg2 + G * sg3 + sg4;
        float y4 = (G4 * u + sum) * rdiv;
        float x = u - kk * y4;
        float xs = std::tanh(x);

        float v_in = G * (xs - s1); float y = v_in + s1; s1 = y + v_in;
        v_in = G * (y - s2);  y = v_in + s2; s2 = y + v_in;
        v_in = G * (y - s3);  y = v_in + s3; s3 = y + v_in;
        v_in = G * (y - s4);  y = v_in + s4; s4 = y + v_in;

        s1 = std::clamp(s1, -8.0f, 8.0f);
        s2 = std::clamp(s2, -8.0f, 8.0f);
        s3 = std::clamp(s3, -8.0f, 8.0f);
        s4 = std::clamp(s4, -8.0f, 8.0f);

        // Acid resonance bass compensation
        float filtered = std::clamp(y + y * kk * 0.5f, -8.0f, 8.0f);

        // Secondary High-Pass Filter (tight sub / screech)
        float hpIn = filtered;
        float vHp = hpG * (hpIn - hpS1);
        float hpOut = hpIn - (vHp + hpS1);
        hpS1 = vHp + hpS1;
        filtered = hpOut;

        advanceEnvelopes();

        // Accent boost
        float outGain = isAccent ? (1.0f + 0.7f * (accent.value * 0.01f)) : 1.0f;
        float ym = std::tanh(filtered * 2.5f * outGain);

        // Post-Filter Distortion
        float distAmt = dist.value * 0.01f;
        if (distAmt > 0.001f) {
            float distG = 1.0f + 16.0f * distAmt * distAmt;
            float distWet = distAmt >= 0.15f ? 1.0f : (distAmt * 6.66f);
            float w = 0.0f;
            int type = (int)distType.value;
            if (type == 1) { // Hard Clip
                w = std::clamp(ym * distG, -1.0f, 1.0f);
            } else if (type == 2) { // Wavefold
                w = std::sin(ym * distG * 1.5707963f);
            } else if (type == 3) { // Fuzz
                float asym = (ym > 0.0f) ? (ym * 1.3f) : (ym * 0.7f);
                w = std::tanh(asym * distG * 1.5f);
            } else { // Soft Tanh
                w = std::tanh(ym * distG) / std::tanh(distG);
            }
            ym += (w - ym) * distWet;
        }

        float drySignal = ym * ampEnvVal * 1.25f;
        liveEnvAmp = ampEnvVal;
        liveEnvFilter = fEnvVal;

        if (ampStage == 3 && ampEnvVal < 1e-4f && !gate) {
            idle = true;
            ampEnvVal = 0.0f;
        }

        // Process Delay & Reverb (Mental Tribe Space)
        float processed = processSpatialFx(drySignal);
        livePost = processed;

        return processed;
    }

private:
    static constexpr int CTRL_RATE = 8;
    float sampleRate = 44100.0f;

    bool gate = false;
    bool idle = true;
    bool isAccent = false;
    bool snap = true;

    int ampStage = 3; // 0: Attack, 1: Decay, 2: Release, 3: Idle
    int controlCountdown = 0;

    float curPitchSt = 0.0f;
    float targetPitchSt = 0.0f;
    float pitchEnv = 0.0f;
    float fEnvVal = 0.0f;
    float ampEnvVal = 0.0f;

    float inc0 = 0.0f, inc1 = 0.0f;
    float phase[3] = { 0.0f, 0.0f, 0.0f };
    float voiceRatios[3] = { 1.0f, 1.0f, 1.0f };
    float subPhase = 0.0f;
    float lfoPhase = 0.0f;

    // Filter states
    float G = 0.0f, G2 = 0.0f, G3 = 0.0f, G4 = 0.0f, kk = 0.0f, rdiv = 1.0f;
    float s1 = 0.0f, s2 = 0.0f, s3 = 0.0f, s4 = 0.0f;
    float hpG = 0.0f, hpS1 = 0.0f;

    // Rates
    float glideCoeff = 0.1f;
    float penvCoeff = 0.9f;
    float fenvDecMul = 0.99f;
    float ampAttStep = 0.01f;
    float ampDecMul = 0.999f;
    float ampRelMul = 0.999f;

    // Spatial FX: Tape/Stereo Feedback Delay Line (up to 1 sec)
    static constexpr int MAX_DELAY = 48000;
    float delayBuf[MAX_DELAY] = {};
    int delayWritePos = 0;

    // Spatial FX: 4 Parallel Comb Filters + 2 All-Pass Reverb (Schroeder / Freeverb style)
    static constexpr int COMB1_L = 1557, COMB2_L = 1617, COMB3_L = 1491, COMB4_L = 1422;
    static constexpr int AP1_L = 225, AP2_L = 556;
    float combBuf1[COMB1_L] = {}, combBuf2[COMB2_L] = {}, combBuf3[COMB3_L] = {}, combBuf4[COMB4_L] = {};
    float apBuf1[AP1_L] = {}, apBuf2[AP2_L] = {};
    int cPos1 = 0, cPos2 = 0, cPos3 = 0, cPos4 = 0;
    int apPos1 = 0, apPos2 = 0;
    float combFilter1 = 0.0f, combFilter2 = 0.0f, combFilter3 = 0.0f, combFilter4 = 0.0f;

    void initEffects()
    {
        std::memset(delayBuf, 0, sizeof(delayBuf));
        std::memset(combBuf1, 0, sizeof(combBuf1));
        std::memset(combBuf2, 0, sizeof(combBuf2));
        std::memset(combBuf3, 0, sizeof(combBuf3));
        std::memset(combBuf4, 0, sizeof(combBuf4));
        std::memset(apBuf1, 0, sizeof(apBuf1));
        std::memset(apBuf2, 0, sizeof(apBuf2));
    }

    bool hasFxTail() const
    {
        return (delayMix.value > 1.0f || revMix.value > 1.0f);
    }

    float processSpatialFx(float in)
    {
        float out = in;

        // 1. Tape Feedback Delay
        float dMix = delayMix.value * 0.01f;
        if (dMix > 0.001f) {
            int dLen = (int)(delayTime.value * 0.001f * sampleRate);
            dLen = std::clamp(dLen, 100, MAX_DELAY - 1);
            int readPos = delayWritePos - dLen;
            if (readPos < 0) readPos += MAX_DELAY;

            float dOut = delayBuf[readPos];
            float fdbk = delayFdbk.value * 0.01f;
            delayBuf[delayWritePos] = in + dOut * fdbk;
            delayWritePos = (delayWritePos + 1) % MAX_DELAY;

            out = in * (1.0f - dMix * 0.5f) + dOut * dMix;
        }

        // 2. High-Density Mental Tribe Reverb
        float rMix = revMix.value * 0.01f;
        if (rMix > 0.001f) {
            float revFeedback = revDecay.value * 0.01f;
            float damp = revDamp.value * 0.01f;

            // Comb 1
            float cOut1 = combBuf1[cPos1];
            combFilter1 = cOut1 * (1.0f - damp) + combFilter1 * damp;
            combBuf1[cPos1] = out + combFilter1 * revFeedback;
            cPos1 = (cPos1 + 1) % COMB1_L;

            // Comb 2
            float cOut2 = combBuf2[cPos2];
            combFilter2 = cOut2 * (1.0f - damp) + combFilter2 * damp;
            combBuf2[cPos2] = out + combFilter2 * revFeedback;
            cPos2 = (cPos2 + 1) % COMB2_L;

            // Comb 3
            float cOut3 = combBuf3[cPos3];
            combFilter3 = cOut3 * (1.0f - damp) + combFilter3 * damp;
            combBuf3[cPos3] = out + combFilter3 * revFeedback;
            cPos3 = (cPos3 + 1) % COMB3_L;

            // Comb 4
            float cOut4 = combBuf4[cPos4];
            combFilter4 = cOut4 * (1.0f - damp) + combFilter4 * damp;
            combBuf4[cPos4] = out + combFilter4 * revFeedback;
            cPos4 = (cPos4 + 1) % COMB4_L;

            float combSum = (cOut1 + cOut2 + cOut3 + cOut4) * 0.25f;

            // All-Pass 1
            float apIn1 = combSum;
            float apBufOut1 = apBuf1[apPos1];
            float apOut1 = -apIn1 + apBufOut1;
            apBuf1[apPos1] = apIn1 + apBufOut1 * 0.5f;
            apPos1 = (apPos1 + 1) % AP1_L;

            // All-Pass 2
            float apIn2 = apOut1;
            float apBufOut2 = apBuf2[apPos2];
            float apOut2 = -apIn2 + apBufOut2;
            apBuf2[apPos2] = apIn2 + apBufOut2 * 0.5f;
            apPos2 = (apPos2 + 1) % AP2_L;

            out = out * (1.0f - rMix * 0.4f) + apOut2 * rMix;
        }

        return out;
    }

    static inline float blep(float t, float dt)
    {
        if (t < dt) {
            float x = t / dt;
            return x + x - x * x - 1.0f;
        }
        if (t > 1.0f - dt) {
            float x = (t - 1.0f) / dt;
            return x * x + x + x + 1.0f;
        }
        return 0.0f;
    }

    float renderOscWave(float p, float dt, float c)
    {
        float saw = 2.0f * p - 1.0f - blep(p, dt);

        float p2 = p + 0.5f;
        if (p2 >= 1.0f) p2 -= 1.0f;
        float sq = (p < 0.5f ? 1.0f : -1.0f) + blep(p, dt) - blep(p2, dt);

        float tri = 2.0f * std::fabs(saw) - 1.0f;

        float p0, p1, t;
        if (c < 0.3333f) {
            p0 = tri;
            p1 = saw;
            t = c * 3.0f;
        } else if (c < 0.6666f) {
            p0 = saw;
            p1 = sq;
            t = (c - 0.3333f) * 3.0f;
        } else {
            p0 = sq;
            p1 = std::tanh(2.0f * saw + sq);
            t = (c - 0.6666f) * 3.0f;
        }
        t = std::clamp(t, 0.0f, 1.0f);
        return p0 + (p1 - p0) * t;
    }

    void updateVoiceRatios()
    {
        int n = (int)std::clamp(unison.value, 1.0f, 3.0f);
        float detuneCents = detune.value;
        for (int i = 0; i < 3; ++i) {
            if (i < n && n > 1) {
                float spread = ((float)i / (float)(n - 1)) * 2.0f - 1.0f;
                voiceRatios[i] = std::exp2(spread * detuneCents * (1.0f / 1200.0f));
            } else {
                voiceRatios[i] = 1.0f;
            }
        }
    }

    float computeLfo()
    {
        float lfoInc = lfoRate.value * (float)CTRL_RATE / sampleRate;
        lfoPhase += lfoInc;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
        float val = std::sin(lfoPhase * 6.2831853f);
        liveLfoVal = val;
        return val;
    }

    void updateControl()
    {
        updateVoiceRatios();
        float lfo = computeLfo();

        float slideSec = std::max(0.005f, glide.value * 0.001f);
        glideCoeff = 1.0f - std::exp(-((float)CTRL_RATE / sampleRate) / slideSec);
        curPitchSt += (targetPitchSt - curPitchSt) * glideCoeff;

        float pDecSec = std::max(0.005f, pDecay.value * 0.001f);
        penvCoeff = std::exp(-((float)CTRL_RATE / sampleRate) / pDecSec);
        pitchEnv *= penvCoeff;

        float effectivePitchSt = curPitchSt + pitchEnv;

        inc0 = snap ? 0.0f : inc1;
        inc1 = 440.0f * std::exp2(effectivePitchSt * (1.0f / 12.0f)) * (1.0f / sampleRate);
        inc1 = std::clamp(inc1, 0.0001f, 0.45f);
        if (snap) {
            inc0 = inc1;
            snap = false;
        }

        // Filter Envelope Decay (Classic 303 decay style)
        float fDecSec = std::max(0.020f, fDecay.value * 0.001f);
        fenvDecMul = std::exp(-((float)CTRL_RATE / sampleRate) / fDecSec);
        fEnvVal *= fenvDecMul;

        // Cutoff & Envelope Modulation
        float baseCutSt = -43.0f + 100.0f * (cutoff.value * 0.01f);
        float envModDepth = 60.0f * (envMod.value * 0.01f) * (isAccent ? (1.0f + 0.6f * (accent.value * 0.01f)) : 1.0f);
        float lfoCutSt = lfo * (lfoToCut.value * 0.35f);
        float totalCutSt = std::clamp(baseCutSt + fEnvVal * envModDepth + lfoCutSt, -43.0f, 62.0f);

        float fc = 440.0f * std::exp2(totalCutSt * (1.0f / 12.0f));
        liveCutoffHz = fc;
        liveReso = reso.value * 0.01f;

        float x = (float)M_PI * fc / sampleRate;
        x = std::clamp(x, 0.0001f, 1.55f);
        float g = std::tan(x);
        g = g / (1.0f + g);
        float g2 = g * g;

        kk = 4.2f * liveReso;
        rdiv = 1.0f / (1.0f + kk * g2 * g2);
        G = g;
        G2 = g2;
        G3 = g2 * g;
        G4 = g2 * g2;

        float hpX = (float)M_PI * std::clamp(hpfCut.value, 15.0f, 2000.0f) / sampleRate;
        float hpTan = std::tan(hpX);
        hpG = hpTan / (1.0f + hpTan);

        float aAttSec = std::max(0.001f, attack.value * 0.001f);
        ampAttStep = 1.0f / (aAttSec * sampleRate);

        float aDecSec = std::max(0.020f, decay.value * 0.001f);
        ampDecMul = std::exp(-1.0f / (aDecSec * sampleRate));

        float aRelSec = std::max(0.005f, release.value * 0.001f);
        ampRelMul = std::exp(-1.0f / (aRelSec * sampleRate));
    }

    void advanceEnvelopes()
    {
        float aSust = sustain.value * 0.01f;
        if (ampStage == 0) {
            ampEnvVal += ampAttStep;
            if (ampEnvVal >= 1.0f) {
                ampEnvVal = 1.0f;
                ampStage = 1;
            }
        } else if (ampStage == 1) {
            ampEnvVal = aSust + (ampEnvVal - aSust) * ampDecMul;
        } else if (ampStage == 2) {
            ampEnvVal *= ampRelMul;
            if (ampEnvVal < 1e-4f) {
                ampEnvVal = 0.0f;
                ampStage = 3;
            }
        }
    }
};
