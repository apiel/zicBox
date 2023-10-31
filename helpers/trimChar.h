#ifndef _TRIMCHAR_H_
#define _TRIMCHAR_H_

#include <cstring>

char *trimChar(char *str, char c = '\n')
{
    int len = strlen(str);
    for (int i = 0; i < len; i++)
    {
        if (str[i] == c)
        {
            str[i] = '\0';
            break;
        }
    }
    return str;
}

#endif
