/** Description:
This code defines a specialized blueprint for a specific type of audio device known as a `Mixer6`.

The purpose of this file is to create a component that leverages a powerful, general-purpose audio system called `Mixer`, but limits and specifies its functionality.

### How It Works

1.  **Specialization:** The number '6' in `Mixer6` is crucial. It signals that this specific device is designed exclusively to handle six distinct audio inputs or channels (like six instruments or microphones).
2.  **Inheritance:** `Mixer6` is built directly on top of the underlying `Mixer` framework, meaning it automatically gains all the general mixing capabilities without having to rewrite them. It’s like creating a specific model of car (a sedan) based on a general automotive platform.
3.  **Initialization:** When this `Mixer6` is created (or instantiated), it receives essential starting instructions, such as audio configuration settings and preferences. It immediately passes these setup details to the core `Mixer` system, ensuring the powerful mixing engine is correctly configured right away to manage exactly six channels.

In essence, this code provides a simple, dedicated way to use the generic mixing library specifically as a 6-channel audio processor within a larger application, ensuring consistency and proper setup.

sha: 5210446999992e7a06a110f4229f0d6731fd3ddc572d51b8be2dd79e9ffcd6ba 
*/
#pragma once

#include "Mixer.h"

class Mixer6 : public Mixer<6> {
public:
    Mixer6(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mixer<6>(props, config)
    {
    }
};
