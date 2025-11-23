/** Description:
This header file defines a specialized digital audio control mechanism known as the `AdsrEnvelop` (Attack, Decay, Sustain, Release). This type of envelope is fundamental in synthesizers and digital sound design, acting as a blueprint for how a sound’s volume, pitch, or other characteristics change over time.

The class inherits basic functionality from a parent `Envelop` structure, customizing it specifically for the four ADSR phases.

**How it Works:**

The envelope manages timing using a crucial setting called the `sampleRate` (typically 48,000 samples per second). The internal logic automatically converts user-friendly time settings (provided in milliseconds) into precise sample counts required by the digital audio system.

The provided functions allow external systems to define the shape of this sound curve:

1.  **Set Attack, Decay, and Release:** These methods specify the duration (in milliseconds) of the rising phase (Attack), the initial drop (Decay), and the final fade-out (Release).
2.  **Set Sustain:** This method sets the specific level or amplitude the sound should maintain while a trigger (like a key press) is active.

In essence, this module serves as a configurable timer and level controller, allowing developers to precisely sculpt the dynamics of a digital sound from its onset to its fade.

sha: 81ff3309a170450b8d56bef9b94875d8739649a39c8fee6a25947b02efa529e2 
*/
#pragma once

#include "Envelop.h"

class AdsrEnvelop : public Envelop {
public:
    unsigned int sampleRate = 48000;

    AdsrEnvelop()
        : Envelop({ { 0.0f, 50 }, { 1.0f, 20 }, { 0.5f, 0 }, { 0.0f, 50 }, { 0.0f, 0 } })
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
