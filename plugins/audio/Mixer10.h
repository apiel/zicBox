/** Description:
This code acts as a specialized blueprint for a component within an audio application or plugin. Its entire purpose is to create a ready-to-use audio mixer designed for a very specific task.

Think of it like manufacturing a particular appliance model based on a flexible factory design.

1.  **The Core Function:** This blueprint is based on a general, reusable design called a "Mixer." This general design handles the complex process of combining multiple audio signals into one output.
2.  **Specialization:** The new blueprint, named `Mixer10`, specifies that the underlying general mixer must be configured to handle **exactly 10 separate audio inputs** simultaneously. It doesn't add new features; it simply locks in the capacity to ten channels.
3.  **Initialization:** When this specialized `Mixer10` is created (or "turned on"), it requires essential setup information, such as configuration settings and property values. It takes this initial data and immediately hands it off to the core 10-channel mixing engine underneath, ensuring the mixer is properly initialized and ready to process sound without needing any further setup steps.

In short, this code is a convenient way to instantiate a fully configured, 10-channel audio mixing module using an existing, robust mixing framework.

sha: 179a6c6df6ad3954269a0eccaac763b8ba44e44808ba5abbb45bed7d396430c3 
*/
#pragma once

#include "Mixer.h"

class Mixer10 : public Mixer<10> {
public:
    Mixer10(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mixer<10>(props, config)
    {
    }
};
