/** Description:
## Blueprint Analysis: Specialized Audio Mixer (Mixer4)

This code snippet serves as a specialized blueprint for creating an audio processing component, likely within a larger audio plugin or digital signal processing framework.

### Identity and Purpose

The core focus is defining a new structure called `Mixer4`. This structure is not a general-purpose mixer; it is specifically fixed to handle **four distinct input signals**.

### How It Works (The Basic Idea)

1.  **Reusing Logic:** Instead of writing new code for blending the four signals, `Mixer4` efficiently inherits all its mixing functionality from a pre-existing, versatile structure called `Mixer`. The `<4>` notation ensures the system knows this particular mixer only deals with four channels.
2.  **Initialization:** The code includes a standard setup routine (the constructor). When a `Mixer4` component is built, this routine takes the necessary configuration details (like volume levels or internal settings) and immediately passes them to the underlying, reusable `Mixer` logic.
3.  **Efficiency:** This approach saves time and reduces errors because developers only needed to define the size (four inputs); all the complex algorithms for balancing and combining the audio signals are reused from the base `Mixer` definition.

In essence, this file defines a ready-to-use, four-channel audio combiner built upon a robust, general-purpose mixing engine.

sha: a07f6ae4828bc3ea4d366bb92a4932ec2bfe3577006b89b78842e4c1b0f08fac 
*/
#pragma once

#include "Mixer.h"

class Mixer4 : public Mixer<4> {
public:
    Mixer4(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mixer<4>(props, config)
    {
    }
};

