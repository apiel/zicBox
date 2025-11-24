/** Description:
This C++ code serves as a standardized "factory" or entry blueprint for a software plugin. It is specifically designed to allow a larger application to dynamically load, use, and manage a component without having its code built directly into the main program.

Before defining the functions, the code uses predefined labels to customize the plugin's name (which defaults to `RectComponent`) and the necessary header file. This setup ensures the blueprint is reusable for creating different components simply by changing these labels.

The core of the code defines two external functions, often critical when dealing with shared libraries or dynamic loading:

1.  **The Allocator:** This function acts as the component creator. When the main application decides it needs the plugin, it calls the allocator. The allocator safely constructs a new instance of the component and provides it to the main program for immediate use.
2.  **The Deleter:** When the application is finished, it calls the deleter. This function guarantees the component is safely removed from the computer's memory, which is essential for preventing slowdowns or resource leaks.

The special notation around these functions ensures that they are easily discoverable and accessible by the operating system and the main application, regardless of whether that application is written strictly in C++. This structure is foundational for building extensible, modular software.

sha: d5fd50a62d2f6b5febb1eb2ad335da3c813f26d5318aea0bc068be38cfcb7c7f 
*/
#ifndef PLUGIN_NAME
#define PLUGIN_NAME RectComponent
#endif

#ifndef PLUGIN_INCLUDE
#define PLUGIN_INCLUDE "RectComponent.h" 
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
