#ifndef _UTILS_H_
#define _UTILS_H_

#include <string.h>

void removeChar(char* str, char garbage)
{

    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage)
            dst++;
    }
    *dst = '\0';
}

void replaceChar(char* str, char search, char replace)
{
    for (char* src = str; *src != '\0'; src++) {
        if (*src == search) {
            *src = replace;
        }
    }
}

void removeExtension(char* str)
{
    char* dot = strrchr(str, '.');
    if (dot) {
        *dot = '\0';
    }
}

#endif