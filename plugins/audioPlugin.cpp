#ifndef PLUGIN_NAME
#define PLUGIN_NAME EffectGainVolume
#endif

#ifndef PLUGIN_INCLUDE
#define PLUGIN_INCLUDE "EffectGainVolume.h" 
#endif

#include PLUGIN_INCLUDE

extern "C"
{
	PLUGIN_NAME *allocator(AudioPlugin::Props& props, char * name)
	{
		return new PLUGIN_NAME(props, name);
	}

	void deleter(PLUGIN_NAME *ptr)
	{
		delete ptr;
	}
}
