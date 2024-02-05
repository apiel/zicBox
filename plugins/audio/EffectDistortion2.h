#ifndef _EFFECT_DISTORTION2_H_
#define _EFFECT_DISTORTION2_H_

#include "../../helpers/range.h"
#include "audioPlugin.h"
#include "mapping.h"

#include <math.h>

#define STEPS 1024 // number of +ve or -ve steps in lookup tabe
#define TABLESIZE 2049 // size of lookup table (steps * 2 + 1)
#define DB_TO_LINEAR(x) (pow(10.0, (x) / 20.0))

/*md
## EffectDistortion2

EffectDistortion2 plugin is used to apply distortion effect on audio buffer.
*/
class EffectDistortion2 : public Mapping {
protected:
    float shape;

    float (EffectDistortion2::*samplePtr)(float) = &EffectDistortion2::skipSample;

    float skipSample(float buf)
    {
        return buf;
    }

    int mTableChoiceIndx = 0;
    double mThreshold_dB = -12.0;
    double mNoiseFloor = -70.0;
    double mParam1 = 50.0;
    double mParam2 = 80.0;
    double mTable[TABLESIZE];
    double mMakeupGain = 1.0;

    float processSample(float buf)
    {
        if (buf == 0.0) {
            return buf;
        }

        double amount = mParam1 / 100.0;
        buf *= 1 + amount * 5;

        int index = std::floor(buf * STEPS) + STEPS;
        index = range(index, 0, 2 * STEPS - 1);
        double xOffset = ((1 + buf) * STEPS) - index;
        xOffset = range(xOffset, 0.0, 1.0);

        float out = mTable[index] + (mTable[index + 1] - mTable[index]) * xOffset;

        double p2 = mParam2 / 100.0;
        out = out * ((1 - p2) + (mMakeupGain * p2));

        return out;
    }

    void hardClip()
    {
        const double threshold = DB_TO_LINEAR(mThreshold_dB);
        mMakeupGain = 15.0 / threshold;

        double lowThresh = 1 - threshold;
        double highThresh = 1 + threshold;

        for (int n = 0; n < TABLESIZE; n++) {
            if (n < (STEPS * lowThresh))
                mTable[n] = -threshold;
            else if (n > (STEPS * highThresh))
                mTable[n] = threshold;
            else
                mTable[n] = n / (double)STEPS - 1;
        }

        printf("gain %f threshold %f lowThresh %f highThresh %f\n", mMakeupGain, threshold, lowThresh, highThresh);
    }

public:
    /*md **Values**: */
    /*md - `DRIVE` to set drive. */
    Val& drive = val(0.0, "DRIVE", { "Distortion" }, [&](auto p) { setDrive(p.value); });

    Val& P1 = val(0.0, "P1", { "Drive" }, [&](auto p) { P1.setFloat(p.value); mParam1 = P1.get(); });
    Val& P2 = val(0.0, "P2", { "Makeup Gain" }, [&](auto p) { P2.setFloat(p.value); mParam2 = P2.get(); });
    /*md - `DB` to set the clipping level threshold. */
    Val& DB = val(0.0, "DB", { "Clipping level", .min = -100.0, .max = 0.0, .unit = "dB" }, [&](auto p) {
        DB.setFloat(p.value);
        mThreshold_dB = DB.get();
        samplePtr = &EffectDistortion2::processSample;
        hardClip();
    });

    EffectDistortion2(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    };

    float sample(float buf)
    {
        return (this->*samplePtr)(buf);
    }

    void sample(float* buf)
    {
        buf[track] = sample(buf[track]);
    }

    void setDrive(float value)
    {
        drive.setFloat(value);
        if (drive.get() == 0.0) {
            samplePtr = &EffectDistortion2::skipSample;
            debug("Distortion: disabled\n");
        } else {
            // float pct = drive.get() / 101.0; // Dividing by 101 instead of 100 to have max 0.99
            // samplePtr = &EffectDistortion2::processSample;
            // shape = 2 * pct / (1 - pct);
            // debug("Distortion: drive=%f shape=%f\n", drive.get(), shape);

            samplePtr = &EffectDistortion2::processSample;
            hardClip();
        }
    }
};

#endif
