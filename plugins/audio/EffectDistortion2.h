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
    double shapeTable[TABLESIZE];
    double mMakeupGain = 1.0;

    float (EffectDistortion2::*samplePtr)(float) = &EffectDistortion2::processSample;
    float (EffectDistortion2::*sampleDrive2Ptr)(float) = &EffectDistortion2::skipSample;

    float skipSample(float buf)
    {
        return buf;
    }

    float processDrive2Sample(float buf)
    {
        return (1 + shape) * buf / (1 + shape * fabsf(buf));
    }

    float processSample(float buf)
    {
        if (buf == 0.0) {
            return buf;
        }

        buf *= 1 + drive.pct() * 5;

        int index = std::floor(buf * STEPS) + STEPS;
        index = range(index, 0, 2 * STEPS - 1);
        double xOffset = ((1 + buf) * STEPS) - index;
        xOffset = range(xOffset, 0.0, 1.0);

        float out = shapeTable[index] + (shapeTable[index + 1] - shapeTable[index]) * xOffset;

        float gainRatioPct = gainRatio.pct();
        out = out * ((1 - gainRatioPct) + (mMakeupGain * gainRatioPct));

        return (this->*sampleDrive2Ptr)(out);
    }

    void hardClip(float thresholdDb)
    {
        const double threshold = DB_TO_LINEAR(thresholdDb);
        mMakeupGain = 15.0 / threshold;

        double lowThresh = 1 - threshold;
        double highThresh = 1 + threshold;

        for (int n = 0; n < TABLESIZE; n++) {
            if (n < (STEPS * lowThresh))
                shapeTable[n] = -threshold;
            else if (n > (STEPS * highThresh))
                shapeTable[n] = threshold;
            else
                shapeTable[n] = n / (double)STEPS - 1;
        }
        // printf("gain %f threshold %f lowThresh %f highThresh %f\n", mMakeupGain, threshold, lowThresh, highThresh);
    }

public:
    /*md **Values**: */
    /*md - `DRIVE` to set drive. */
    Val& drive = val(0.0, "DRIVE", { "Drive" });
    /*md - `DRIVE2` to set drive2. */
    Val& drive2 = val(0.0, "DRIVE2", { "Drive2" }, [&](auto p) { setDrive2(p.value); });
    /*md - `GAIN_RATIO` to set makeup gain. */
    Val& gainRatio = val(0.0, "GAIN_RATIO", { "Makeup Gain" });
    /*md - `DB` to set the clipping level threshold. */
    Val& db = val(0.0, "DB", { "Clipping level", .min = -100.0, .max = 0.0, .unit = "dB" }, [&](auto p) {
        db.setFloat(p.value);
        hardClip(db.get());
    });
    /*md - `ENABLE` to enable the effect. */
    Val& enable = val(1.0, "ENABLE", { "Enable", .max = 1.0 }, [&](auto p) {
        enable.setFloat(p.value);
        if (enable.get() > 0.0) {
            samplePtr = &EffectDistortion2::processSample;
        } else {
            samplePtr = &EffectDistortion2::skipSample;
        }
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

    void setDrive2(float value)
    {
        drive2.setFloat(value);
        if (drive2.get() > 0) {
            float pct = drive2.get() / 101.0; // Dividing by 101 instead of 100 to have max 0.99
            shape = 2 * pct / (1 - pct);
            sampleDrive2Ptr = &EffectDistortion2::processDrive2Sample;
        } else {
            sampleDrive2Ptr = &EffectDistortion2::skipSample;
        }
    }
};

#endif
