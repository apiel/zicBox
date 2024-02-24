#ifndef _GET_FULLPATH_H
#define _GET_FULLPATH_H

#include <string.h>

// TODO use string...

// paste path into parentFilename folder and output is fullpathBuffer
char* getFullpath(char* newPath, const char* parentFilename, char* fullpathBuffer)
{
    char* lastSlash;
    strcpy(fullpathBuffer, parentFilename);
    if (newPath[0] == '/' || (lastSlash = strrchr(fullpathBuffer, '/')) == NULL) {
        fullpathBuffer[0] = '\0';
    } else {
        *lastSlash = '\0';
        strcat(fullpathBuffer, "/");
    }
    strcat(fullpathBuffer, newPath);
    return fullpathBuffer;
}

#endif