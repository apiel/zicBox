#ifndef _GET_FULLPATH_H
#define _GET_FULLPATH_H

#include <string.h>

// TODO use string...

// paste path into parentFilename folder and output is fullpathBuffer
char* getFullpath(char* path, const char* parentFilename, char* fullpathBuffer)
{
    if (path[0] == '/') {
        return path;
    }

    strcpy(fullpathBuffer, parentFilename);
    char* lastSlash = strrchr(fullpathBuffer, '/');
    if (lastSlash) {
        *lastSlash = '\0';
        strcat(fullpathBuffer, "/");
    } else {
        fullpathBuffer[0] = '\0';
    }
    strcat(fullpathBuffer, path);
    return fullpathBuffer;
}

#endif