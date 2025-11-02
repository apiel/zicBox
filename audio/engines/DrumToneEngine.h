#pragma once

#include "audio/engines/Engine.h"
#include "audio/EnvelopDrumAmp.h"
#include "audio/KickEnvTableGenerator.h"
#include "audio/KickTransientTableGenerator.h"
#include "audio/WavetableGenerator2.h"
#include "audio/filterArray.h"
#include "audio/lookupTable.h"

class DrumToneEngine: public Engine {
protected:
    int sampleRate;

    EffectFilterArray<3> filter;
    float resonance = 0.0f; // 0.0 to 1.0

    float freq = 1.0f;

    float applyFilter(float out, float envAmp)
    {
        if (filterCutoff < 0) {
            float amount = -filterCutoff;
            filter.setCutoff(0.85 * amount * envAmp + 0.1);
            filter.setSampleData(out, 0);
            filter.setSampleData(filter.hp[0], 1);
            filter.setSampleData(filter.hp[1], 2);
            // if (amount < 0.5f) { // Soft transition between LP and HP
            //     float ratio = amount / 0.5f;
            //     out = filter.lp[0] * (1.0f - ratio) + filter.hp[2] * ratio;
            // } else {
            //     out = filter.hp[2];
            // }
            // Let's just always mix both of them
            out = filter.lp[0] * (1.0f - amount) + filter.hp[2] * amount;
        } else {
            filter.setCutoff(0.85 * filterCutoff * envAmp + 0.1);
            filter.setSampleData(out, 0);
            filter.setSampleData(filter.lp[0], 1);
            filter.setSampleData(filter.lp[1], 2);
            out = filter.lp[2];
        }
        return out;
    }

public:
    WavetableGenerator waveform;
    EnvelopDrumAmp envelopAmp;
    KickEnvTableGenerator envelopFreq;
    KickTransientTableGenerator transient;

    int duration = 1000; // 50 to 3000
    int8_t pitch = -8; // -36 to 36

    // Filter
    float filterCutoff = 0.0f; // -1.0 to 1.0

    void hydrate(std::vector<KeyValue> values) override {
        for (auto& kv : values) {
            if (kv.key == "duration") duration = std::get<float>(kv.value);
            else if (kv.key == "pitch") pitch = std::get<float>(kv.value);
            else if (kv.key == "filterCutoff") filterCutoff = std::get<float>(kv.value);
            else if (kv.key == "waveformType") waveform.setType(std::get<std::string>(kv.value));
            else if (kv.key == "waveformMorph") waveform.setMorph(std::get<float>(kv.value));
            else if (kv.key == "envelopAmp") envelopAmp.morph(std::get<float>(kv.value));
            else if (kv.key == "envelopFreq") envelopFreq.setMorph(std::get<float>(kv.value));
            else if (kv.key == "transient") transient.setMorph(std::get<float>(kv.value));
        }
    }
    std::vector<KeyValue> serialize() override { return {
        { "duration", (float)duration },
        { "pitch", (float)pitch },
        { "filterCutoff", filterCutoff },
        { "waveformType", waveform.getTypeName() },
        { "waveformMorph", waveform.getMorph() },
        { "envelopAmp", envelopAmp.getMorph() },
        { "envelopFreq", envelopFreq.getMorph() },
        { "transient", transient.getMorph() },
    }; }



    // void hydrate(std::vector<KeyValue> values) override {
    //     for (auto& kv : values) {
    //         if (kv.key == "burstSpacing") burstSpacing = std::get<float>(kv.value);
    //         else if (kv.key == "decay") decay = std::get<float>(kv.value);
    //         else if (kv.key == "burstCount") burstCount = std::get<float>(kv.value);
    //         else if (kv.key == "noiseColor") noiseColor = std::get<float>(kv.value);
    //         else if (kv.key == "punch") punch = std::get<float>(kv.value);
    //         else if (kv.key == "filter") filter = std::get<float>(kv.value);
    //         else if (kv.key == "resonance") resonance = std::get<float>(kv.value);
    //         else if (kv.key == "envelopAmp") envelopAmp.morph(std::get<float>(kv.value));
    //     }
    // }
    // std::vector<KeyValue> serialize() override { return {
    //     { "burstSpacing", burstSpacing },
    //     { "decay", decay },
    //     { "burstCount", (float)burstCount },
    //     { "noiseColor", noiseColor },
    //     { "punch", punch },
    //     { "filter", filter },
    //     { "resonance", resonance },
    //     { "envelopAmp", envelopAmp.getMorph() },
    // }; }

    DrumToneEngine(int sampleRate, LookupTable* lookupTable)
        : Engine(Engine::Type::Drum, "Tone", "Tone")
        , sampleRate(sampleRate)
        , waveform(lookupTable, sampleRate)
    {
        waveform.setType(WavetableGenerator::Type::Saw);
        envelopAmp.morph(0.2f);
    }

    void setDuration(int value) { duration = CLAMP(value, 50, 3000); }
    void setPitch(int value) { pitch = CLAMP(value, -36, 36); }
    void setFilterCutoff(float value) { filterCutoff = CLAMP(value, -1.0f, 1.0f); }

    void setResonance(float value)
    {
        resonance = CLAMP(value, 0.0f, 1.0f); 
        filter.setResonance(0.95 * (1.0 - std::pow(1.0 - resonance, 2)));
    }

    float getResonance() { return resonance; }

    // Higher base note is, lower pitch will be
    //
    // Usualy our base note is 60
    // but since we want a kick sound we want bass and we remove one octave (12 semitones)
    // So we send note 60, we will play note 48...
    const uint8_t baseNote = 60 + 12;

    void noteOn(uint8_t note) override
    {
        freq = pow(2, ((note - baseNote + pitch) / 12.0));

        totalSamples = static_cast<int>(sampleRate * (duration / 1000.0f));
        envelopAmp.reset(totalSamples);
        sampleCounter = 0;
        sampleIndex = 0.0f;
    }

protected:
    int totalSamples = 0;
    int sampleCounter = 0;
    float sampleIndex = 0.0f;

public:
    float sample() override
    {
        if (sampleCounter < totalSamples) {
            float t = float(sampleCounter) / totalSamples;
            sampleCounter++;

            float envAmp = envelopAmp.next();
            float envFreq = envelopFreq.next(t);

            float modulatedFreq = freq + envFreq;
            float out = waveform.sample(&sampleIndex, modulatedFreq) * 0.75f;

            if (t < 0.01f && transient.getMorph() > 0.0f) {
                out = out + transient.next(t);
            }

            out = applyFilter(out, envAmp);

            return out * envAmp;
        }
        return 0.0f;
    }
};