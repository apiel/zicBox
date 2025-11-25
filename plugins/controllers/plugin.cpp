/** Description:
This C++ code acts as a standard gateway or loading dock for a software component, often called a plugin, such as an audio controller (`OscController`). Its primary purpose is to create a reliable, standardized mechanism for a main application (the "host") to start up and shut down this specialized tool.

The initial lines define key names using placeholders—identifying the plugin's name, the required configuration details, and the file needed to access it. This template approach allows developers to reuse this wrapper code easily for many different plugins.

The core functionality resides in two essential public functions:

1.  **The `allocator`:** This function is the "factory." When the host application decides it needs the `OscController` component, it calls the `allocator`. This function instantly creates a brand-new, active copy of the controller, initializing it with necessary settings and identification details.
2.  **The `deleter`:** This function is the "cleanup crew." When the host application is done using the component, it calls the `deleter` to safely shut down and completely remove the active controller copy from the computer's memory.

In essence, this code ensures that the external plugin can be reliably brought into existence, configured, and then properly retired by the host software, maintaining stability and resource efficiency.

sha: c4fcac315118656ce21a0831dbbb942dacdbd08633ea4678903427b165487c6d 
*/
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
