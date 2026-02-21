#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/WavetableGenerator2.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyReverb.h"
#include "audio/effects/applyWaveshape.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterArray.h"
#include "audio/utils/math.h"
#include "helpers/clamp.h"

class DrumBass : public EngineBase<DrumBass> {
public:
    EnvelopDrumAmp envelopAmp;

protected:
    const float sampleRate;
    EffectFilterArray<2> filter;
    
    WavetableGenerator waveform;
    WavetableInterface* wave = nullptr;

    float* reverbBuffer = nullptr;
    int reverbIndex = 0;

    float velocity = 1.0f;
    float sampleIndex = 0.0f;
    float currentFreq = 1.0f;
    int currentSampleCount = 0;
    int totalSamples = 0;

    void updateFilter() {
        // Initial setup for filter params (resonance)
        float res = 0.95f * (1.0f - Math::pow(1.0f - pct(resonance), 2.0f));
        filter.setResonance(res);
    }

    void updateWaveform() {
        int idx = static_cast<int>(waveformType.value);
        waveform.setType(static_cast<WavetableGenerator::Type>(idx));
        waveform.setMorph(pct(shape));
        wave = &waveform;
    }

public:
    Param params[12] = {
        { .label = "Duration", .unit = "ms", .value = 800.0f, .min = 50.0f, .max = 3000.0f, .step = 10.0f },
        { .label = "Amp. Env.", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float val) { static_cast<DrumBass*>(ctx)->envelopAmp.morph(val * 0.01f); } },
        { .label = "Pitch", .value = 0.0f, .min = -24.0f, .max = 24.0f, .step = 1.0f, .type = VALUE_CENTERED },
        { .label = "Bend", .unit = "%", .value = 40.0f },
        { .label = "Cutoff", .unit = "%", .value = 50.0f },
        { .label = "Resonance", .unit = "%", .value = 20.0f, .onUpdate = [](void* ctx, float val) { static_cast<DrumBass*>(ctx)->updateFilter(); } },
        { .label = "Waveform", .value = 0.0f, .min = 0.0f, .max = 5.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) { static_cast<DrumBass*>(ctx)->updateWaveform(); } },
        { .label = "Shape", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float val) { static_cast<DrumBass*>(ctx)->updateWaveform(); } },
        { .label = "Clipping", .unit = "%", .value = 0.0f },
        { .label = "Drive", .unit = "%", .value = 0.0f },
        { .label = "Compression", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED },
        { .label = "Reverb", .unit = "%", .value = 10.0f }
    };

    Param& duration = params[0];
    Param& ampEnv = params[1];
    Param& pitch = params[2];
    Param& bend = params[3];
    Param& cutoff = params[4];
    Param& resonance = params[5];
    Param& waveformType = params[6];
    Param& shape = params[7];
    Param& clipping = params[8];
    Param& drive = params[9];
    Param& compression = params[10];
    Param& reverb = params[11];

    DrumBass(const float sampleRate, float* rvBuffer, LookupTable* lut)
        : EngineBase(Drum, "Bass", params)
        , sampleRate(sampleRate)
        , waveform(lut, sampleRate)
        , reverbBuffer(rvBuffer)
    {
        init();
        updateWaveform();
        updateFilter();
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        sampleIndex = 0.0f;
        currentSampleCount = 0;
        
        // Base note 72 (60 + 12) per original logic
        currentFreq = Math::pow(2.0f, (static_cast<float>(note) - 72.0f + pitch.value) / 12.0f);
        
        totalSamples = static_cast<int>(sampleRate * (duration.value * 0.001f));
        envelopAmp.reset(totalSamples);
    }

    float sampleImpl()
    {
        float envAmp = envelopAmp.next();
        
        // Process reverb tail even if envelope is finished
        if (envAmp < 0.001f) {
            return applyReverb(0.0f, pct(reverb), reverbBuffer, reverbIndex);
        }

        // 1. Pitch Bend Logic
        float t = static_cast<float>(currentSampleCount) / static_cast<float>(totalSamples);
        float bendedFreq = currentFreq * (1.0f - pct(bend) * t);
        
        // 2. Wave Generation
        float out = 0.0f;
        if (wave) {
            out = *wave->sample(&sampleIndex, bendedFreq);
        }
        out *= velocity * envAmp;

        // 3. Filtering
        float dynamicCutoff = 0.85f * pct(cutoff) * envAmp + 0.1f;
        filter.setCutoff(dynamicCutoff);
        filter.setSampleData(out, 0);
        filter.setSampleData(filter.lp[0], 1);
        out = filter.lp[1];

        // 4. Clipping
        if (clipping.value > 0.0f) {
            out = CLAMP(out + out * pct(clipping) * 8.0f, -1.0f, 1.0f);
        }

        // 5. FX Chain
        out = applyReverb(out, pct(reverb), reverbBuffer, reverbIndex);
        
        // Compression / Waveshape centered logic
        if (compression.value > 0.0f) {
            out = applyCompression(out, pct(compression));
        } else if (compression.value < 0.0f) {
            out = applyWaveshape2(out, -pct(compression));
        }

        if (drive.value > 0.0f) {
            out = applyDrive(out, pct(drive));
        }

        currentSampleCount++;
        return out * envAmp;
    }
};