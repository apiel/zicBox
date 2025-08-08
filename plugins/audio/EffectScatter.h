#pragma once

#include "audioPlugin.h"
#include "log.h"
#include "mapping.h"
#include "plugins/audio/utils/effects/applyReverb.h"
#include "plugins/audio/utils/effects/applySampleReducer.h"

#define MAX_SCATTER_EFFECTS 5

class EffectScatter : public Mapping {
protected:
    float velocity = 0.0f;
    bool effectActive[MAX_SCATTER_EFFECTS] = { false };

public:
    EffectScatter(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    }

    void sample(float* buf)
    {
        float out = buf[track];
        if (effectActive[0]) {
            out = fxSampleReducer(out, 0.5f * velocity);
        }
        if (effectActive[1]) {
            out = fxReverb(out, velocity);
        }
        if (effectActive[2]) {
            out = fxDelay(out, velocity);
        }
        

        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        if (_velocity == 0.0f){
            return noteOff(note, _velocity);
        }
        logDebug("EffectScatter noteOn: %d %f\n", note, _velocity);
        effectActive[note % MAX_SCATTER_EFFECTS] = true;
        velocity = _velocity;
    }

    void noteOff(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        logDebug("EffectScatter noteOff: %d %f\n", note, _velocity);
        effectActive[note % MAX_SCATTER_EFFECTS] = false;
    }

protected:
    float sampleSqueeze;
    int samplePosition = 0;
    float fxSampleReducer(float input, float amount)
    {
        return applySampleReducer(input, amount, sampleSqueeze, samplePosition);
    }

    static constexpr int REVERB_BUFFER_SIZE = 48000; // 1 second buffer at 48kHz
    float buffer[REVERB_BUFFER_SIZE] = { 0.0f };
    int bufferIndex = 0;
    float fxReverb(float signal, float amount)
    {
        float reverbAmount = amount;
        return applyReverb(signal, reverbAmount, buffer, bufferIndex, REVERB_BUFFER_SIZE);
    }

    static constexpr int DELAY_BUFFER_SIZE = REVERB_BUFFER_SIZE * 3; // 3 second
    float buffer2[DELAY_BUFFER_SIZE] = { 0.0f };
    int bufferIndex2 = 0;
    float fxDelay(float input, float amount)
    {
        return applyDelay(input, amount, buffer2, bufferIndex2, DELAY_BUFFER_SIZE);
    }

};
