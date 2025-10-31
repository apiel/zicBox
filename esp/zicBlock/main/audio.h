#pragma once

#include "audio/filterArray.h"
#include "audio/lookupTable.h"
#include "audio/effects/tinyReverb.h"

#include "audio/engines/DrumClapEngine.h"
#include "audio/engines/DrumMetalicEngine.h"
#include "audio/engines/DrumStringEngine.h"
#include "audio/engines/DrumToneEngine.h"
#include "audio/engines/DrumSnareHatEngine.h"

// add modulation that could turn into FM --> might use page switch on same button
//       ----> maybe instead add a second layer that could act as FM modulation
// add fx and f2 using page switch on same button
//    ----> add master volume, and then could be 3 multi effect

class Audio {
protected:
    LookupTable lookupTable;

    TINY_REVERB_BUFFER

    float velocity = 1.0f;

    Audio()
        : tone(sampleRate, &lookupTable)
        , clap(sampleRate, lookupTable)
        , drumString(sampleRate, tinyReverbBuffer)
        , metalic(sampleRate, tinyReverbBuffer)
        , snareHat(sampleRate, lookupTable)
    {
    }

public:
    DrumToneEngine tone;
    DrumClapEngine clap;
    DrumStringEngine drumString;
    DrumMetalicEngine metalic;
    DrumSnareHatEngine snareHat;

    float volume = 1.0f;

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
        float out = 0.0f;
        // out = tone.sample() * volume;
        out = clap.sample() * volume;
        // out = drumString.sample() * volume * volume;
        // out = metalic.sample() * volume;
        // out = snareHat.sample() * volume;
        return CLAMP(out, -1.0f, 1.0f);
    }

    void noteOn(uint8_t note, float _velocity)
    {
        velocity = _velocity;

        // tone.noteOn(note);
        clap.noteOn(note);
        // drumString.noteOn(note);
        // metalic.noteOn(note);
        // snareHat.noteOn(note);
    }

    void noteOff(uint8_t note) { }
};

Audio* Audio::instance = NULL;
