#pragma once

#include "audioBuffer.h"
#include "audioPlugin.h"
#include "log.h"
#include "mapping.h"
#include "plugins/audio/utils/effects/applySampleReducer.h"

#define MAX_SCATTER_EFFECTS 5

class EffectScatter : public Mapping {
protected:
    AudioBuffer<> buffer;

    uint8_t activeEffectIndex = 255; // No effect
    float velocity = 0.0f;
    uint8_t playedNote = 0;

    // ---- State for effects ----
    uint64_t repeatStart = 0; // For Beat Repeat
    uint64_t repeatLength = 2000; // samples
    uint64_t repeatPos = 0;

    uint64_t sprayCounter = 0; // For Granular Spray
    uint64_t sprayGrainSize = 200; // samples
    uint64_t sprayIndex = 0;

    float pitchIncrement = 1.0f; // For Pitch Change
    float pitchPos = 0.0f;

    // static constexpr int REVERB_BUFFER_SIZE = 48000; // 1 second buffer at 48kHz
    // static constexpr int DELAY_BUFFER_SIZE = REVERB_BUFFER_SIZE * 3; // 3 second
    // float buffer[DELAY_BUFFER_SIZE] = { 0.0f };
    // int bufferIndex = 0;

    // bool effectActive[MAX_SCATTER_EFFECTS] = { false };

public:
    EffectScatter(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    }

    void sample(float* buf)
    {
        buffer.addSample(buf[track]);

        if (activeEffectIndex < MAX_SCATTER_EFFECTS) {
            switch (activeEffectIndex) {
            case 0:
                fxSampleReducer(buf, 0.5f);
                break;
            case 1:
                processPitchChange(buf);
                break;
            case 2:
                processGate(buf);
                break;
                // add more effects here
            case 3:
                processBeatRepeat(buf);
                break;
            case 4:
                processGranularSpray(buf);
                break;
            }
        }
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        if (_velocity == 0.0f)
            return noteOff(note, _velocity);

        activeEffectIndex = note % MAX_SCATTER_EFFECTS;
        playedNote = note;
        velocity = _velocity;
        // logDebug("Scatter noteOn: %d %f activeEffectIndex: %d\n", note, velocity, activeEffectIndex);
        resetEffectState();
    }

    void noteOff(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        if (note % MAX_SCATTER_EFFECTS == activeEffectIndex) {
            // logDebug("Scatter noteOff: %d %f activeEffectIndex: %d\n", note, velocity, activeEffectIndex);
            activeEffectIndex = 255;
        }
    }

protected:
    void processGranularSpray(float* buf)
    {
        if (sprayCounter == 0) {
            // Pick random start position in buffer
            sprayIndex = (buffer.index + (rand() % buffer.size)) % buffer.size;
            sprayCounter = sprayGrainSize;
        }
        *buf = buffer.samples[sprayIndex] * velocity;
        sprayIndex = (sprayIndex + 1) % buffer.size;
        sprayCounter--;
    }

    void processBeatRepeat(float* buf)
    {
        if (repeatPos == 0) {
            repeatStart = buffer.index;
        }
        uint64_t playIndex = (repeatStart + repeatPos) % buffer.size;
        buf[track] = buffer.samples[playIndex] * velocity;

        repeatPos++;
        if (repeatPos >= repeatLength) {
            repeatPos = 0; // restart loop
        }
    }

    void processPitchChange(float* buf)
    {
        float input = buf[track];
        if (input > 0.10 || input < -0.10) {
            buf[track] = -input;
        }
    }

    float sampleSqueeze;
    int samplePosition = 0;
    void fxSampleReducer(float* buf, float amount)
    {
        buf[track] = applySampleReducer(buf[track], amount, sampleSqueeze, samplePosition);
    }

    void processGate(float* buf)
    {
        static uint64_t counter = 0;
        static uint64_t gateLength = 100;
        static bool gateOpen = true;

        if (counter++ >= gateLength) {
            gateOpen = !gateOpen;
            counter = 0;
        }
        buf[track] = gateOpen ? buffer.samples[buffer.index % buffer.size] * velocity : 0.0f;
    }

    void resetEffectState()
    {
        // Reset static vars for each effect
        // (Needed so old state doesn’t carry over between triggers)
    }
};
