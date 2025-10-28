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

    int totalSamples = 0;
    int sampleCounter = 0;
    float sampleIndex = 0.0f;
    float freq = 1.0f;
    float baseFreq = 440.0f;
    float resonatorState = 0.0f;
    float velocity = 1.0f;

    // Higher base note is, lower pitch will be
    //
    // Usualy our base note is 60
    // but since we want a kick sound we want bass and we remove one octave (12 semitones)
    // So we send note 60, we will play note 48...
    uint8_t baseNote = 60 + 12;

    Audio()
        : waveform(&lookupTable, sampleRate)
    {
        waveform.setType(WavetableGenerator::Type::Saw);
        envelopAmp.morph(0.2f);
    }

    float applyResonator(float input)
    {
        // Advance "state" (acts like a time accumulator)
        resonatorState += 1.0f / sampleRate;

        // Compute output = input * e^(-decay * t) * sin(2π f t)
        float output = input * expf(-0.02f * resonatorState)
            * sinf(2.0f * M_PI * baseFreq * resonatorState * resonator);

        // Optional loudness compensation so higher freq = less drop in volume
        float compensation = sqrtf(220.0f / std::max(baseFreq, 1.0f));
        output *= compensation;

        float amount = resonator / 1.5f;
        return (output * amount) + (input * (1.0f - amount));
    }

public:
    WavetableGenerator waveform;
    EnvelopDrumAmp envelopAmp;
    KickEnvTableGenerator envelopFreq;

    int duration = 1000; // 50 to 3000
    int8_t pitch = -8; // -36 to 36
    float resonator = 0.0f; // 0.00 to 1.50
    float timbre = 0.0f; // 0.00 to 1.00

    const static int sampleRate = 48000;
    const static uint8_t channels = 2;

    static Audio* instance;
    static Audio& get()
    {
        if (!instance) {
            instance = new Audio();
        }
        return *instance;
    }

    float sample()
    {
        float out = 0.0f;
        if (sampleCounter < totalSamples) {
            float t = float(sampleCounter) / totalSamples;
            float envAmp = envelopAmp.next();
            float envFreq = envelopFreq.next(t);

            float modulatedFreq = freq + envFreq;
            out = waveform.sample(&sampleIndex, modulatedFreq) * 0.75f;

            if (resonator > 0.0f) {
                out = applyResonator(out);
            }

            if (timbre > 0.0f) {
                // Adjust timbre by filtering harmonics dynamically
                out *= (1.0f - timbre) + timbre * sinf(2.0f * M_PI * baseFreq * 0.5f * t);
            }

            // TODO add resonator and fm

            // and then on another page clap and noise...

            // and then string

            out = out * envAmp * velocity;
            out = CLAMP(out, -1.0f, 1.0f);
            sampleCounter++;
        }
        return out;
    }

    void noteOn(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        totalSamples = static_cast<int>(sampleRate * (duration / 1000.0f));
        envelopAmp.reset(totalSamples);
        sampleCounter = 0;
        sampleIndex = 0.0f;
        freq = pow(2, ((note - baseNote + pitch) / 12.0));
        baseFreq = freq * 440.0f;
    }

    void noteOff(uint8_t note) { }
};

Audio* Audio::instance = NULL;
