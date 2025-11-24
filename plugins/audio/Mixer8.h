/** Description:
This C++ header file acts as a specialized blueprint for a specific type of digital audio tool called `Mixer8`.

The fundamental purpose of this code is to create a ready-to-use version of a generic audio component. Imagine a standard "Mixer" toolkit—this file defines a version of that toolkit specifically pre-configured for exactly **8** inputs or channels.

**How it Works:**

1.  **Specialization:** It builds upon an existing, more general `Mixer` component (likely designed to handle any number of channels). By naming it `Mixer8` and specifying the number 8 internally, the code ensures that this specific object is permanently set up to manage eight separate audio streams.
2.  **Configuration:** When the `Mixer8` object is initialized (or constructed), it immediately takes necessary setup information (like audio properties and configuration settings) and hands that information directly up to the foundational 8-channel mixer it inherits from.
3.  **Result:** This results in a highly focused, fixed-size mixer—reliable for tasks requiring precisely eight sources to be combined, such as blending eight instrument tracks in an audio application.

sha: 04e5957ec886f3689a424ad01d7e3b33ef3d4033128399c2648378e394ad4944 
*/
#pragma once

#include "Mixer.h"

class Mixer8 : public Mixer<8> {
public:
    Mixer8(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mixer<8>(props, config)
    {
    }
};
