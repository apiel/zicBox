#pragma once

#define USE_LUT_AND_FAST_MATH

#include "audio/Clock.h"
#include "audio/Sequencer.h"
#include "audio/effects/fx.h"
#include "audio/effects/tinyReverb.h"
#include "audio/filterArray.h"
#include "audio/lookupTable.h"

#include "audio/engines/DrumClapEngine.h"
#include "audio/engines/DrumMetalicEngine.h"
#include "audio/engines/DrumSnareHatEngine.h"
#include "audio/engines/DrumStringEngine.h"
#include "audio/engines/DrumToneEngine.h"

class Audio {
protected:
    LookupTable lookupTable;
    Clock clock;

    TINY_REVERB_BUFFER

    float velocity = 1.0f;

    Audio()
        : clock(sampleRate, channels)
        , seq(lookupTable, [&](auto& step) { noteOn(step.note, step.velocity); }, [&](auto& step) { noteOff(step.note); })
        , fx1(sampleRate, &lookupTable)
        , fx2(sampleRate, &lookupTable)
        , fx3(sampleRate, &lookupTable)
        , tone(sampleRate, &lookupTable)
        , clap(sampleRate, lookupTable)
        , drumString(sampleRate, lookupTable, tinyReverbBuffer)
        , metalic(sampleRate, lookupTable, tinyReverbBuffer)
        , snareHat(sampleRate, lookupTable)
    {
        fx1.set(8);
        fx1Amount = 0.5f;
        fx2.set(3);
        fx2Amount = 0.70;

        // seq.steps = {
        //     { 0, 60 },
        //     { 8, 60, 1.0f, 1, 0.85f },
        //     { 16, 60 },
        //     { 24, 60, 1.0f, 1, 0.85f },
        //     { 32, 60 },
        //     { 40, 60, 1.0f, 1, 0.85f },
        //     { 48, 60 },
        //     { 56, 60, 1.0f, 1, 0.85f },
        // };
    }

public:
    Sequencer seq;
    Fx fx1;
    float fx1Amount = 0.0f;
    Fx fx2;
    float fx2Amount = 0.0f;
    Fx fx3;
    float fx3Amount = 0.0f;

    DrumToneEngine tone;
    DrumClapEngine clap;
    DrumStringEngine drumString;
    DrumMetalicEngine metalic;
    DrumSnareHatEngine snareHat;

    Engine* engine = &tone;

    float volume = 1.0f;
    bool isPlaying = false;

    const static int sampleRate = 22050; // 👈 22.05 kHz
    // const static int sampleRate = 44100;
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
        if (isPlaying) {
            uint32_t clockValue = clock.getClock();
            if (clockValue != 0) {
                if (clockValue % 6 == 0) {
                    seq.onStep();
                }
                // on clock
            }
        }

        float out = 0.0f;
        out = engine->sample();

        out = fx1.apply(out, fx1Amount);
        out = fx2.apply(out, fx2Amount);
        out = fx3.apply(out, fx3Amount);

        out = out * volume * volume * velocity;
        return CLAMP(out, -1.0f, 1.0f);
    }

    void noteOn(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        engine->noteOn(note);
    }

    void noteOff(uint8_t note) { }

    void setEngine(Engine* e) { engine = e; }
    void setFx1Amount(float a) { fx1Amount = CLAMP(a, 0.0f, 1.0f); }
    void setFx2Amount(float a) { fx2Amount = CLAMP(a, 0.0f, 1.0f); }
    void setFx3Amount(float a) { fx3Amount = CLAMP(a, 0.0f, 1.0f); }

    void play() { isPlaying = true; }
    void pause() { isPlaying = false; }
    void stop()
    {
        isPlaying = false;
        seq.reset();
    }
};

Audio* Audio::instance = NULL;
