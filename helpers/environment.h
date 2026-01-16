/** Description:
This file serves as a utility tool designed to configure the initial appearance and dimensions of a program’s window based on external settings. It is essentially a configuration reader.

The core idea is to check system-level parameters, known as environment variables, which can be set by the user or the system before the program starts.

1.  **Window Position Retrieval:** The first function checks for a variable named `WINDOW_POSITION`. If this setting is found (expected to be in a format like "X,Y"), the code reads the coordinates, logs them for debugging, and returns them to the application so the window can be positioned precisely on the screen. If the setting is missing, the function returns a neutral value, signaling the application to use its standard default location.

2.  **Window Size Retrieval:** Similarly, the second function checks for the `WINDOW_SIZE` variable. It parses the string into Width and Height values. This allows users to externally define how large the program window should be upon launch.

In both cases, the code ensures the configuration values are present and valid before converting them from text into numerical data that the application can use for drawing the window. This approach ensures maximum flexibility for deployment and user customization.

sha: 40ecc311e6822f1d4dc63db4da678c5e85718274756c2d142da31f5cf46184b9 
*/
#pragma once

#include <cstdlib>

#include "log.h"
#include "draw/baseInterface.h"

Point getEnvWindowPosition()
{
    if (getenv("WINDOW_POSITION") && getenv("WINDOW_POSITION")[0] != '\0') {
        logInfo("Env variable window position: %s", getenv("WINDOW_POSITION"));
        std::string position = getenv("WINDOW_POSITION");
        std::string x = position.substr(0, position.find(","));
        std::string y = position.substr(position.find(",") + 1);
        return { atoi(x.c_str()), atoi(y.c_str()) };
    }

    return { -1, -1 };
}

Size getEnvWindowSize()
{
    if (getenv("WINDOW_SIZE") && getenv("WINDOW_SIZE")[0] != '\0') {
        logInfo("Env variable window size: %s", getenv("WINDOW_SIZE"));
        std::string size = getenv("WINDOW_SIZE");
        std::string w = size.substr(0, size.find(","));
        std::string h = size.substr(size.find(",") + 1);
        return { atoi(w.c_str()), atoi(h.c_str()) };
    }

    return { -1, -1 };
}
