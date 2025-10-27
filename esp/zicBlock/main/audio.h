#pragma once

#define SKIP_SNDFILE

#include "plugins/audio/utils/EnvelopDrumAmp.h"
#include "plugins/audio/utils/KickEnvTableGenerator.h"
#include "plugins/audio/utils/WavetableGenerator2.h"
#include "plugins/audio/utils/lookupTable.h"

#include <cmath>
#include <string>

class Audio {
protected:
    LookupTable lookupTable;
    WavetableGenerator waveform;
    EnvelopDrumAmp envelopAmp;
    KickEnvTableGenerator envelopFreq;

    int duration = 1000; // 50 to 3000
    int8_t pitch = 0; // -36 to 36

    int totalSamples = 0;
    int sampleCounter = 0;
    float sampleIndex = 0.0f;
    float freq = 1.0f;

    // Higher base note is, lower pitch will be
    //
    // Usualy our base note is 60
    // but since we want a kick sound we want bass and we remove one octave (12 semitones)
    // So we send note 60, we will play note 48...
    uint8_t baseNote = 60 + 12;

public:
    const static int sampleRate = 48000;
    const static uint8_t channels = 2;

    Audio()
        : waveform(&lookupTable, sampleRate)
    {
        waveform.setType(WavetableGenerator::Type::Saw);
        envelopAmp.morph(0.2f);
    }

    float sample()
    {
        float out = 0.0f;
        if (sampleCounter < totalSamples) {
            float t = float(sampleCounter) / totalSamples;
            float envAmp = envelopAmp.next();
            float envFreq = envelopFreq.next(t);

            float modulatedFreq = freq + envFreq;
            out = waveform.sample(&sampleIndex, modulatedFreq) * envAmp;

            sampleCounter++;
        }
        return out;
    }

    void noteOn(uint8_t note, float _velocity)
    {
        totalSamples = static_cast<int>(sampleRate * (duration / 1000.0f));
        envelopAmp.reset(totalSamples);
        sampleCounter = 0;
        sampleIndex = 0.0f;
        freq = pow(2, ((note - baseNote + pitch) / 12.0));
    }

    void noteOff(uint8_t note) { }
};