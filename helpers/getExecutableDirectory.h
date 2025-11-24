/** Description:
This C++ utility code provides a simple, dedicated tool for finding the location of the running application on the computer’s file system.

### I. Identity and Purpose

This is a specialized utility function designed to determine the precise folder (directory) where the program executable resides. This is crucial for applications that need to load configuration files, resources, or assets stored relative to their own installation location, regardless of where the user launched the program from.

### II. Core Functionality

The single core function, `getExecutableDirectory`, executes a system check and returns a text string containing the full folder path of the program.

### III. Basic Idea of Operation

The function employs a low-level, operating system-specific trick (common in Linux/Unix environments).

1.  **Locating the Pointer:** The running program queries a special, internal system reference (a hidden link) that the operating system maintains, which constantly points to the program's absolute file location.
2.  **Reading the Full Path:** It safely reads this full path (which includes both the folder and the file name).
3.  **Extracting the Directory:** Once the full path is obtained, a separate utility strips away the actual file name of the executable, leaving only the surrounding folder path.

If the system cannot successfully read this crucial location pointer, the code reports an error and returns an empty result, ensuring the calling program knows the operation failed.

### IV. Dependencies

The code relies on standard operating system interfaces for reading file paths and managing large path buffers, as well as basic C++ features for handling text strings and reporting errors.

sha: 32ce9d4aa49961a9f7d559d75aedb2c446c598579fb4761eb9300ff6f4e64e88 
*/
#pragma once

#include <iostream>
#include <limits.h>
#include <libgen.h>
#include <unistd.h>

std::string getExecutableDirectory() {
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        char *dir = dirname(buffer);
        return std::string(dir);
    } else {
        std::cerr << "Error getting executable path" << std::endl;
        return "";
    }
}
