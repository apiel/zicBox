#pragma once

#include "audioBuffer.h"
#include "audioPlugin.h"
#include "mapping.h"
#include "log.h"

#define MAX_SCATTER_EFFECTS 3

class EffectScatter : public Mapping {
protected:
    AudioBuffer<> buffer;

    uint8_t activeEffectIndex = 255; // No effect
    float velocity = 0.0f;
    uint8_t playedNote = 0;

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
                processStutter(buf);
                break;
            case 1:
                processReverse(buf);
                break;
            case 2:
                processGate(buf);
                break;
                // add more effects here
            }
        }
    }

    /* ---------- Example Effect Methods ---------- */
    void processStutter(float* buf)
    {
        static uint64_t stutterLength = 48000;
        static uint64_t idx = 0;
        static float cachedSample = 0.0f;

        if (idx % stutterLength == 0) {
            cachedSample = buffer.samples[buffer.index % buffer.size];
        }
        *buf = cachedSample * velocity;
        idx++;
    }

    void processReverse(float* buf)
    {
        static int64_t windowSize = 48000;
        static int64_t offset = 0;

        int64_t reverseIndex = (int64_t)buffer.index - offset;
        if (reverseIndex < 0)
            reverseIndex += buffer.size;
        *buf = buffer.samples[reverseIndex % buffer.size] * velocity;

        offset = (offset + 1) % windowSize;
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
        *buf = gateOpen ? buffer.samples[buffer.index % buffer.size] * velocity : 0.0f;
    }

    /* ---------- MIDI Handling ---------- */
    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        if (_velocity == 0.0f)
            return noteOff(note, _velocity);

        activeEffectIndex = note % MAX_SCATTER_EFFECTS;
        playedNote = note;
        velocity = _velocity;
        logDebug("Scatter noteOn: %d %f activeEffectIndex: %d\n", note, velocity, activeEffectIndex);
        resetEffectState();
    }

    void noteOff(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        if (note % MAX_SCATTER_EFFECTS == activeEffectIndex) {
            logDebug("Scatter noteOff: %d %f activeEffectIndex: %d\n", note, velocity, activeEffectIndex);
            activeEffectIndex = 255;
        }
    }

    void resetEffectState()
    {
        // Reset static vars for each effect
        // (Needed so old state doesn’t carry over between triggers)
    }
};
