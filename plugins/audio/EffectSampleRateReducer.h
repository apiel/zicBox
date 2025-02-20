#pragma once

#include "audioPlugin.h"
#include "mapping.h"

#include <math.h>

/*md
## EffectSampleRateReducer

EffectSampleRateReducer plugin is used to reduce sample rate on audio buffer.
*/
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
    /*md **Values**: */
    /*md - `SAMPLE_STEP` to set sample step reduction. */
    Val& sampleStep = val(0.0f, "SAMPLE_STEP", { "Step Reducer", .max = 256.0, .unit = "steps" }, [&](auto p) { setSampleStep(p.value); });

    EffectSampleRateReducer(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    };

    EffectSampleRateReducer& setSampleStep(float value)
    {
        sampleStep.setFloat(value);
        sampleStepInt = sampleStep.get();
        if (sampleStepInt == 0) {
            samplePtr = &EffectSampleRateReducer::skipSample;
            // debug("SampleRateReducer: disabled\n");
        } else {
            samplePtr = &EffectSampleRateReducer::processSample;
            // debug("SampleRateReducer: sampleStep=%d\n", sampleStepInt);
        }
        return *this;
    }

    void sample(float* buf)
    {
        buf[track] = (this->*samplePtr)(buf[track]);
    }
};
