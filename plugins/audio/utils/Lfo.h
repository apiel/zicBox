#pragma once

#include <stdlib.h>

class LFO {
public:
    enum Waveform {
        TRIANGLE,
        SQUARE,
        SAWTOOTH,
        WAVEFORM_COUNT,
    };

    LFO(float sampleRate, float rate = 1.0f, Waveform wave = SQUARE)
        : sampleRate(sampleRate)
        , rate(rate)
        , phase(0.0f)
    {
        setWaveform(wave);
        updatePhaseIncrement();
    }

    void setRate(float r)
    {
        rate = r;
        updatePhaseIncrement();
    }

    void setWaveform(Waveform wave)
    {
        waveform = wave;
        switch (wave) {
        case SQUARE:
            processFunc = &LFO::processSquare;
            break;
        case TRIANGLE:
            processFunc = &LFO::processTriangle;
            break;
        case SAWTOOTH:
            processFunc = &LFO::processSawtooth;
            break;
        }
    }

    float process()
    {
        return (this->*processFunc)(); // Call the selected function
    }

private:
    float sampleRate;
    float rate;
    Waveform waveform;
    float phase;
    float phaseIncrement;
    float (LFO::*processFunc)() = nullptr;

    void updatePhaseIncrement()
    {
        phaseIncrement = rate / sampleRate;
    }

    float processSquare()
    {
        float value = (phase < 0.5f) ? 1.0f : -1.0f;
        phase += phaseIncrement;
        if (phase >= 1.0f)
            phase -= 1.0f;
        return value;
    }

    float processTriangle()
    {
        float value = 2.0f * abs(2.0f * phase - 1.0f) - 1.0f;
        phase += phaseIncrement;
        if (phase >= 1.0f)
            phase -= 1.0f;
        return value;
    }

    float processSawtooth()
    {
        float value = 2.0f * phase - 1.0f;
        phase += phaseIncrement;
        if (phase >= 1.0f)
            phase -= 1.0f;
        return value;
    }
};