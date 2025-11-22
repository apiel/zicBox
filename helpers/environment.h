#pragma once

#include <cstdlib>

#include "log.h"
#include "plugins/components/baseInterface.h"

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
