/** Description:
This code defines a highly specific building block used within an audio processing application or framework.

### Explanation of the Mixer12 Component

**The Purpose:**
This component creates a specialized version of an audio mixer. Think of a physical soundboard or mixing console. Instead of building the entire mixing architecture from scratch every time, this code relies on an existing, universal design called the "Mixer."

**How it Works (Simplified Idea):**
The key function of this small piece of code is to enforce a precise configuration: it explicitly tells the system, "Use the standard mixing blueprint, but specifically configure it to handle exactly 12 input channels."

The component is named `Mixer12` and it inherits all the complex mixing functionality from the generic `Mixer` design. The number 12 specifies the channel count.

**Initialization:**
When this `Mixer12` is created (or instantiated), it receives essential setup information—like properties and configurations—from the main audio plugin it belongs to. It immediately forwards this information to the underlying, core 12-channel mixing engine. This ensures the mixer starts up correctly and is ready to blend or combine those 12 separate audio signals into a single output stream without requiring new, complex code just for the setup.

In summary, this code acts as a streamlined way to instantly produce a specialized, ready-to-use 12-channel mixer based on a pre-existing, robust architectural design.

sha: 843c1f069708552c54889e7fafbce241e50d821b5939fb9e30b7c24c2819b494 
*/
#pragma once

#include "Mixer.h"

class Mixer12 : public Mixer<12> {
public:
    Mixer12(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mixer<12>(props, config)
    {
    }
};
