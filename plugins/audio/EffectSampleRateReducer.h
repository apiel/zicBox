/** Description:
This code defines an audio effect component, the `EffectSampleRateReducer`, which is designed to intentionally degrade or alter the sound quality of digital audio by simulating a reduction in the effective sampling rate. This technique is often used to achieve vintage digital or specific distortion sounds.

**How the Effect Works:**

The fundamental principle of this effect is based on repetition rather than skipping. Instead of processing every incoming piece of audio data (sample), the system utilizes a main control parameter called `SAMPLE_STEP`.

1.  **Setting the Step:** The user sets the `SAMPLE_STEP` value. This value dictates how frequently the effect should capture a *new* input sample.
2.  **Holding the Sample:** When the effect is active, it captures an input sample and holds onto that exact value. It then repeats this held value for a number of cycles equal to the defined step.
3.  **The Result:** If the step value is high (e.g., 50), the output audio will use the same audio value 50 times before updating to the next unique value from the input stream. This infrequent updating of the audio data creates the signature "sample rate reduction" effect, introducing digital artifacts and aliasing.

The component is engineered for efficiency. If the `SAMPLE_STEP` parameter is set to zero, the system uses an internal switch to bypass the reduction logic entirely, simply passing the audio through unchanged and ensuring maximum performance when the effect is disabled.

sha: 509c33d03f0a61c5cbc2afe6124db9d7ef00c99d7b8ec52fbb52612f8a39cb00 
*/
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
