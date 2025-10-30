#pragma once

#define SKIP_SNDFILE

#include "audio/EnvelopDrumAmp.h"
#include "audio/KickEnvTableGenerator.h"
#include "audio/KickTransientTableGenerator.h"
#include "audio/WavetableGenerator2.h"
#include "audio/effects/applySample.h"
#include "audio/filterArray.h"
#include "audio/lookupTable.h"

#include "audio/engines/DrumClapEngine.h"
#include "audio/engines/DrumToneEngine.h"
#include "audio/engines/DrumStringEngine.h"

#include <string>
class Audio {
protected:
    LookupTable lookupTable;

    float velocity = 1.0f;

    Audio()
        : tone(sampleRate, &lookupTable)
        , clap(sampleRate, lookupTable)
        , drumString(sampleRate)
    {
    }


public:
    // Tone
    DrumToneEngine tone;
    float toneVolume = 1.0f; // 0.00 to 1.00

    // Clap
    DrumClapEngine clap;
    float clapVolume = 0.2f;

    // String
DrumStringEngine drumString;
    float stringVolume = 0.5f;

  
    // const static int sampleRate = 48000;
    const static int sampleRate = 44100;
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
        float sumVolume = toneVolume + clapVolume + stringVolume;
        if (sumVolume == 0.0f) {
            return 0.0f;
        }

        float out = 0.0f;
        if (toneVolume > 0.0f) {
            out += tone.sample() * toneVolume;
        }

        if (clapVolume > 0.0f) {
            out += clap.sample() * clapVolume;
        }

        if (stringVolume > 0.0f) {
            out += drumString.sample() * stringVolume * stringVolume;
        }

        // add modulation that could turn into FM --> might use page switch on same button
        //       ----> maybe instead add a second layer that could act as FM modulation
        // add fx and f2 using page switch on same button
        //    ----> add master volume, and then could be 3 multi effect

        out = (out * velocity) / sumVolume;
        out = CLAMP(out, -1.0f, 1.0f);
        return out;
    }

    void noteOn(uint8_t note, float _velocity)
    {
        velocity = _velocity;

        tone.noteOn(note);
        clap.noteOn(note);

        if (stringVolume > 0.0f) {
            drumString.noteOn(note);
        }
    }

    void noteOff(uint8_t note) { }
};

Audio* Audio::instance = NULL;
