#ifndef _EFFECT_SAMPLE_RATE_REDUCER_H_
#define _EFFECT_SAMPLE_RATE_REDUCER_H_

#include "audioPlugin.h"
#include "mapping.h"

#include <math.h>

class EffectSampleRateReducer : public Mapping {
protected:
    float sampleSqueeze;
    int samplePosition = 0;
    int sampleStepInt = 0; // Could get rid of this...

    float (EffectSampleRateReducer::*samplePtr)(float) = &EffectSampleRateReducer::skipSample;

    float skipSample(float buf)
    {
        return buf;
    }

    float processSample(float buf)
    {
        if (samplePosition == 0) {
            sampleSqueeze = buf;
        }

        if (samplePosition < sampleStepInt) {
            samplePosition++;
        } else {
            samplePosition = 0;
        }

        return sampleSqueeze;
    }

public:
    Val& sampleStep = val(0.0f, "SAMPLE_STEP", { "Step Reducer", .max = 256.0, .unit = "steps" }, [&](auto p) { setSampleStep(p.value); });

    EffectSampleRateReducer(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    };

    EffectSampleRateReducer& setSampleStep(float value)
    {
        sampleStep.setFloat(value);
        sampleStepInt = sampleStep.get();
        if (sampleStepInt == 0) {
            samplePtr = &EffectSampleRateReducer::skipSample;
            debug("SampleRateReducer: disabled\n");
        } else {
            samplePtr = &EffectSampleRateReducer::processSample;
            debug("SampleRateReducer: sampleStep=%d\n", sampleStepInt);
        }
        return *this;
    }

    void sample(float* buf)
    {
        buf[track] = (this->*samplePtr)(buf[track]);
    }
};

#endif
