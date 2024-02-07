#ifndef _ZIC_ADSR_ENVELOP_H_
#define _ZIC_ADSR_ENVELOP_H_

#include "Envelop.h"

class AdsrEnvelop : public Envelop {
public:
    unsigned int sampleRate = 48000;

    AdsrEnvelop()
        : Envelop({ { 0.0f, 50 }, { 1.0f, 20 }, { 0.5f, 0 }, { 0.0f, 50 } })
    {
    }

    void setSampleRate(unsigned int rate)
    {
        sampleRate = rate;
    }

    void setAttack(unsigned int ms)
    {
        data[0].sampleCount = Envelop::msToSampleCount(ms, sampleRate);
    }

    void setDecay(unsigned int ms)
    {
        data[1].sampleCount = Envelop::msToSampleCount(ms, sampleRate);
    }

    void setSustain(float value)
    {
        data[2].modulation = value;
        data[3].modulation = value;
    }

    void setRelease(unsigned int ms)
    {
        data[3].sampleCount = Envelop::msToSampleCount(ms, sampleRate);
    }
};

#endif
