#ifndef _GET_FULLPATH_H
#define _GET_FULLPATH_H

#include <string.h>

std::string getFullpath(std::string newPath, std::string parentFilename)
{
    if (newPath[0] == '@') {
        std::string newPathCopy = newPath;
        newPathCopy.replace(0, 1, ".");
        return newPathCopy;
    }

    size_t lastSlash;
    if (newPath[0] == '/' || (lastSlash = parentFilename.find_last_of("/")) == std::string::npos) {
        return newPath;
    }
    std::string parentFolder = parentFilename.substr(0, lastSlash);
    return parentFolder + "/" + newPath;
}

#endif
