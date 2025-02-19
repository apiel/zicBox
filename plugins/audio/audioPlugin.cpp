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
