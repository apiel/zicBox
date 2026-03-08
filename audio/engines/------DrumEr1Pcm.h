#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/MultiFx.h"
#include "audio/TransientGenerator.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/applySampleGain.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"
#include <cstring>
#include <sndfile.h>

class DrumEr1Pcm : public EngineBase<DrumEr1Pcm> {
public:
    EnvelopDrumAmp envelopAmp;
    TransientGenerator transient;
    MultiFx multiFx1;
    MultiFx multiFx2;

    static const uint64_t bufferSize = 48000 * 3;
    float sampleData[bufferSize];
    uint64_t sampleCount = 0;

protected:
    const float sampleRate;
    float velocity = 1.0f;
    float index = 0.0f;
    float modPhase = 0.0f;
    float transientIndex = 0.0f;

    // Mod S&H state
    int samplesPerHold = 1;
    int lastHoldSample = -1;
    float heldValue = 0.0f;

    char fx1Name[24] = "Off";
    char fx2Name[24] = "Off";
    char modTypeName[24] = "Sine";

public:
    Param params[12] = {
        { .label = "Duration", .unit = "ms", .value = 1000.0f, .min = 50.0f, .max = 3000.0f },
        { .label = "Amp. Env.", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float v) {
             ((DrumEr1Pcm*)ctx)->envelopAmp.morph(v * 0.01f);
         } },
        { .label = "Waveform", .value = 1.0f },
        { .label = "Pitch", .unit = "st", .value = 0.0f, .min = -24.0f, .max = 24.0f, .type = VALUE_CENTERED },
        { .label = "Mod Depth", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED },
        { .label = "Mod Speed", .unit = "Hz", .value = 100.0f, .min = 10.0f, .max = 5000.0f },
        { .label = "Mod Type", .string = modTypeName, .value = 0.0f, .max = 8.0f, .onUpdate = [](void* ctx, float v) {
             auto e = (DrumEr1Pcm*)ctx;
             const char* names[] = { "Sine", "Saw Down", "Square", "Triangle", "S&H", "Noise", "Env Expo", "Env Bounce", "Env Asym" };
             strncpy(e->modTypeName, names[(int)v], 23);
         } },
        { .label = "Transient", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float v) {
             ((DrumEr1Pcm*)ctx)->transient.morphType(v * 0.01f);
         } },
        { .label = "FX1 Type", .string = fx1Name, .value = 0.0f, .max = (float)MultiFx::FX_COUNT - 1, .onUpdate = [](void* ctx, float v) {
             auto e = (DrumEr1Pcm*)ctx;
             e->multiFx1.setEffect((int)v);
             strncpy(e->fx1Name, e->multiFx1.getEffectName(), 23);
         } },
        { .label = "FX1 Edit", .unit = "%", .value = 0.0f },
        { .label = "FX2 Type", .string = fx2Name, .value = 0.0f, .max = (float)MultiFx::FX_COUNT - 1, .onUpdate = [](void* ctx, float v) {
             auto e = (DrumEr1Pcm*)ctx;
             e->multiFx2.setEffect((int)v);
             strncpy(e->fx2Name, e->multiFx2.getEffectName(), 23);
         } },
        { .label = "FX2 Edit", .unit = "%", .value = 0.0f },
    };

    // Reference Helpers
    Param& pitchParam = params[2];
    Param& modDepth = params[3];
    Param& modSpeed = params[4];
    Param& modType = params[5];
    Param& transMorph = params[6];
    Param& fx1Amt = params[8];
    Param& fx2Amt = params[10];

    DrumEr1Pcm(const float sampleRate, float* fxBuf1, float* fxBuf2)
        : EngineBase(Drum, "ER-1 PCM", params)
        , sampleRate(sampleRate)
        , transient(sampleRate, 50)
        , multiFx1(sampleRate, fxBuf1)
        , multiFx2(sampleRate, fxBuf2)
    {
        init();
    }

    void loadSample(const std::string& path)
    {
        SF_INFO sfinfo;
        SNDFILE* file = sf_open(path.c_str(), SFM_READ, &sfinfo);
        if (file) {
            sampleCount = sf_read_float(file, sampleData, bufferSize);
            sf_close(file);
            applySampleGain(sampleData, sampleCount);
        }
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        index = 0.0f;
        modPhase = 0.0f;
        transientIndex = 0.0f;
        lastHoldSample = -1;
        samplesPerHold = std::max(1, (int)(sampleRate / modSpeed.value * 10.0f));

        int totalSamples = (int)(sampleRate * (params[0].value * 0.001f));
        envelopAmp.reset(totalSamples);
    }

    float getModulation(int sc, int ts)
    {
        float depth = pct(modDepth) * 2.0f - 1.0f;
        float x = (float)sc / ts;
        float speedPct = pct(modSpeed);

        switch ((int)modType.value) {
        case 0:
            return 1.0f + Math::sin(modPhase) * depth;
        case 1:
            return 1.0f + (1.0f - (fmodf(modPhase / PI_X2, 1.0f) * 2.0f)) * depth;
        case 2:
            return 1.0f + (Math::sin(modPhase) >= 0 ? 1.0f : -1.0f) * depth;
        case 3: {
            float p = fmodf(modPhase / PI_X2, 1.0f);
            return 1.0f + (p < 0.5f ? (p * 4.0f - 1.0f) : (3.0f - p * 4.0f)) * depth;
        }
        case 4: { // S&H
            if (sc / samplesPerHold != lastHoldSample) {
                heldValue = Noise::sample();
                lastHoldSample = sc / samplesPerHold;
            }
            return 1.0f + heldValue * depth;
        }
        case 5:
            return 1.0f + Noise::sample() * depth;
        case 6:
            return 1.0f + Math::exp(-(10.0f + 100.0f * Math::pow(1.0f - speedPct, 4.0f)) * x) * depth;
        case 7: { // Bounce
            float freq = 2.0f + 10.0f * (1.0f - speedPct);
            return 1.0f + std::abs(Math::sin(M_PI * x * freq)) * Math::exp(-(4.0f + 8.0f * speedPct) * x) * depth;
        }
        case 8: { // Asym
            float rise = 2.0f + 4.0f * (1.0f - speedPct);
            float drop = 10.0f + 50.0f * (1.0f - speedPct);
            float val = (x < 0.5f) ? Math::pow(x * 2.0f, 1.0f / rise) : 1.0f - Math::exp(-drop * (x - 0.5f) * 2.0f);
            return 1.0f + val * depth;
        }
        default:
            return 1.0f;
        }
    }

    float sampleImpl()
    {
        float envAmp = envelopAmp.next();
        if (envAmp < 0.001f) return multiFx1.apply(multiFx2.apply(0.0f, pct(fx2Amt)), pct(fx1Amt));

        int sc = envelopAmp.getSampleCounter();
        int ts = envelopAmp.getTotalSamples();

        float mod = getModulation(sc, ts);
        modPhase += pct(modSpeed) * 0.01f;
        if (modPhase > PI_X2) modPhase -= PI_X2;

        float out = 0.0f;
        float pitchShift = Math::pow(2.0f, pitchParam.value / 12.0f);

        if ((uint64_t)index < sampleCount) {
            out = sampleData[(uint64_t)index] * mod;
            index += pitchShift;
        }

        // Transient
        if ((float)sc / ts < 0.01f && transMorph.value > 0.0f) {
            out += transient.next(&transientIndex);
        }

        out = multiFx1.apply(out, pct(fx1Amt));
        out = multiFx2.apply(out, pct(fx2Amt));

        return out * envAmp * velocity;
    }
};