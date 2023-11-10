#ifndef PLUGIN_NAME
#define PLUGIN_NAME ButtonComponent
#endif

#ifndef PLUGIN_INCLUDE
#define PLUGIN_INCLUDE "ButtonComponent.h" 
#endif

#ifndef PLUGIN_PROPS
#define PLUGIN_PROPS ComponentInterface::Props
#endif

#include PLUGIN_INCLUDE 

extern "C"
{
	PLUGIN_NAME *allocator(PLUGIN_PROPS props)
	{
		return new PLUGIN_NAME(props);
	}

	void deleter(PLUGIN_NAME *ptr)
	{
		delete ptr;
	}
}
