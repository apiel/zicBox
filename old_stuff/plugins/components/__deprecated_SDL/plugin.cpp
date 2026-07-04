/** Description:
This C++ code acts as a specialized setup file, serving as a critical bridge for creating a plug-in or dynamic software component—specifically, likely a "Button Component."

### Core Purpose: The Universal Plug-in Factory

The primary role of this file is to define the exact steps required for an external program (the "host application") to load, use, and properly discard this specific component. Instead of making the host application understand complex C++ rules, this file provides simple, universal entry points.

### Key Mechanisms

1.  **Dynamic Naming:** The code first sets up temporary labels (macros) for the component's name, the header file it needs, and the type of setup instructions (properties) it requires. This makes the code highly flexible; the same setup can be reused for many different components by simply changing the labels during compilation.

2.  **The Universal Translator (`extern "C"`):** The most important feature is the use of a "universal translator" mechanism. C++ objects are complex, but the older C language standard is universally understood by almost all operating systems and programming languages. By wrapping the core functions in this mechanism, we guarantee that the external host application, regardless of what language it is written in, can successfully communicate with and load the C++ component.

3.  **Creation and Destruction:** Two essential functions are defined:
    *   **The Creator (`allocator`):** This function is responsible for manufacturing the component. When the host application needs a new "Button," it calls this creator, providing any necessary configuration settings. The function then builds a new instance of the component and safely returns it to the host.
    *   **The Destroyer (`deleter`):** This function ensures orderly shutdown. When the host is finished using the component, it calls the destroyer, which correctly cleans up the component's memory and resources, preventing crashes or memory leaks.

In essence, this file provides the required handshake protocols for dynamic software module loading.

sha: d71f778ce45dc0e7ff0d5cbcbca28dfea3e7e1fab8ea9e5159aec53a4e21fcd8 
*/
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
