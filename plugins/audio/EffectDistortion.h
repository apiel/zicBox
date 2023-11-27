#ifndef _EFFECT_DISTORTION_H_
#define _EFFECT_DISTORTION_H_

#include "../../helpers/range.h"
#include "audioPlugin.h"
#include "mapping.h"

#include <math.h>

class EffectDistortion : public Mapping {
protected:
    float shape;

    float (EffectDistortion::*samplePtr)(float) = &EffectDistortion::skipSample;

    float skipSample(float buf)
    {
        return buf;
    }

    float processSample(float buf)
    {
        if (buf == 0.0) {
            return buf;
        }

        return (1 + shape) * buf / (1 + shape * fabsf(buf));
    }

public:
    Val& drive = val(0.0, "DRIVE", [&](float value) { setDrive(value); }, { "Distortion" });

    EffectDistortion(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        setDrive(drive.get());
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
            samplePtr = &EffectDistortion::skipSample;
            debug("Distortion: disabled\n");
        } else {
            float pct = drive.get() / 101.0; // Dividing by 101 instead of 100 to have max 0.99
            samplePtr = &EffectDistortion::processSample;
            shape = 2 * pct / (1 - pct);
            debug("Distortion: drive=%f shape=%f\n", drive.get(), shape);
        }
    }
};

#endif
