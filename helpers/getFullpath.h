#ifndef _GET_FULLPATH_H
#define _GET_FULLPATH_H

#include <string.h>

char *getFullpath(char *path, const char *parentFilename, char *fullpathBuffer)
{
    if (path[0] == '/')
    {
        return path;
    }

    strcpy(fullpathBuffer, parentFilename);
    char *lastSlash = strrchr(fullpathBuffer, '/');
    if (lastSlash)
    {
        *lastSlash = '\0';
    } else {
        fullpathBuffer[0] = '\0';
    }
    strcat(fullpathBuffer, "/");
    strcat(fullpathBuffer, path);
    return fullpathBuffer;
}

#endif