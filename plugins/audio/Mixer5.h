/** Description:
This code defines a specialized component designed for use within audio software, often called a plugin. Think of it as creating a specific type of digital soundboard.

**1. The Overall Goal (The Blueprint):**
The primary purpose is to define a specific type of "Mixer." A mixer is responsible for blending or combining multiple sound signals into one output, much like a physical mixer console used in music production.

**2. The Specific Role (Mixer5):**
This component is explicitly named `Mixer5`. The number 5 is the key feature: it signals that this mixer is specifically configured to handle exactly five distinct audio inputs or channels. It is a specialized version of a general, flexible mixing component defined elsewhere.

**3. How It Works (Inheritance):**
Instead of writing all the complex blending logic from scratch, `Mixer5` takes advantage of a pre-existing general design (the `Mixer` blueprint). It inherits all the necessary features and functionalities—like volume control, balancing, and routing—from that general blueprint.

When this `Mixer5` component is activated within the audio program, it initializes itself using the core mixing logic but ensures it is ready to manage exactly five sources based on the instructions (properties and configuration) provided by the host software.

In summary, this file is the definition for a 5-channel audio mixing unit, leveraging a standardized design for efficiency and predictability.

sha: 21019fcbe0d9dd65c91f50a75d9c1415e2462cf3ec1b4786699b1cd206ddfa77 
*/
#pragma once

#include "Mixer.h"

class Mixer5 : public Mixer<5> {
public:
    Mixer5(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mixer<5>(props, config)
    {
    }
};

