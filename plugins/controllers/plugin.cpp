#ifndef PLUGIN_NAME
#define PLUGIN_NAME OscController
#endif

#ifndef PLUGIN_INCLUDE
#define PLUGIN_INCLUDE "OscController.h" 
#endif

#ifndef PLUGIN_PROPS
#define PLUGIN_PROPS ControllerInterface::Props
#endif

#include PLUGIN_INCLUDE 

extern "C"
{
	PLUGIN_NAME *allocator(PLUGIN_PROPS& props, uint16_t id)
	{
		return new PLUGIN_NAME(props, id);
	}

	void deleter(PLUGIN_NAME *ptr)
	{
		delete ptr;
	}
}
