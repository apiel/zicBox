/** Description:
This code snippet is an organizational structure, commonly known as a header file, designed to manage dependencies within a larger C or C++ software project, likely related to digital audio processing.

It acts primarily as a manifest, importing essential components needed elsewhere. Its main purpose is to link tools required for handling two major parts of the system:
1. Definitions related to **audio plugins** (external extensions or modules).
2. Definitions specifying the standard method (interface) for managing and setting **values** (like volume levels, equalization settings, or other parameters).

A built-in safety mechanism is used to ensure that the file’s contents are only processed once by the compiler, regardless of how many times the file is referenced across the program. This prevents errors and maintains compilation efficiency.

Finally, the presence of a developer note (`FIXME`) indicates that the programmer responsible for the code intends to review or potentially modify this section in the future, suggesting that the current structure may be temporary or require optimization.

sha: 5d36c56ec7e02d6380cfe58c00c279cb7587519439013e795b2c4a935bbc1bbc 
*/
#ifndef _IMPORT_VALUE_INTERFACE_H_
#define _IMPORT_VALUE_INTERFACE_H_

// FIXME
#include "../audio/audioPlugin.h"
#include "../audio/valueInterface.h"

#endif