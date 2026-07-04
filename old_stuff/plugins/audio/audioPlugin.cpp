/** Description:
This C++ code acts as the essential interface or "doorway" for an audio software plugin, likely a volume or gain control effect named `EffectGainVolume`.

Its primary function is to provide a standardized, universal way for a host application (like a digital audio workstation) to interact with the plugin, regardless of internal programming complexities.

The code defines two critical, publicly accessible functions necessary for managing the plugin's life cycle:

1.  **The Allocator (The Builder):** This function is called by the host application when the audio effect is needed. It creates a new instance of the `EffectGainVolume` plugin in the computer's memory, giving it necessary setup information (properties and configuration data).
2.  **The Deleter (The Destroyer):** When the host application is finished using the effect, this function is called to safely and cleanly remove the plugin instance from memory, preventing performance issues.

The special `extern "C"` block is crucial because it ensures these two management functions are easily discoverable and usable by the host program, even if the host is written in a different programming language or uses different naming conventions than the C++ plugin core. This structure is standard practice for creating loadable software components like audio effects.

sha: 53fb604ceaab4933c4a764fd9852cec58a3e2d399f7d865f3828e88be0326589 
*/
#ifndef PLUGIN_NAME
#define PLUGIN_NAME EffectGainVolume
#endif

#ifndef PLUGIN_INCLUDE
#define PLUGIN_INCLUDE "EffectGainVolume.h" 
#endif

#include PLUGIN_INCLUDE

extern "C"
{
	PLUGIN_NAME *allocator(AudioPlugin::Props& props, AudioPlugin::Config& config)
	{
		return new PLUGIN_NAME(props, config);
	}

	void deleter(PLUGIN_NAME *ptr)
	{
		delete ptr;
	}
}
