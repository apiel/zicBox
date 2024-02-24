#ifndef _GET_FULLPATH_H
#define _GET_FULLPATH_H

#include <string.h>

std::string getFullpath(std::string newPath, std::string parentFilename)
{
    size_t lastSlash;
    if (newPath[0] == '/' || (lastSlash = parentFilename.find_last_of("/")) == std::string::npos) {
        return newPath;
    }
    std::string parentFolder = parentFilename.substr(0, lastSlash);
    return parentFolder + "/" + newPath;
}

#endif
