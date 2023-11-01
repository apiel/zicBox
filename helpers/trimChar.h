#ifndef _TRIMCHAR_H_
#define _TRIMCHAR_H_

#include <cstring>

char* trimChar(char* str, char c = '\n')
{
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (str[i] == c) {
            str[i] = '\0';
            break;
        }
    }

    // for (int i = strlen(str); i > 0; i--) {
    //     if (str[i] != c) {
    //         str[i] = '\0';
    //         break;
    //     }
    // }
    return str;
}

std::string rtrim(std::string s, const char c)
{
    size_t end = s.find_last_not_of(c);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string rtrim(std::string s, const char *c)
{
    size_t n = sizeof(c) / sizeof(c[0]);
    for (size_t i = 0; i < n; i++) {
        s = rtrim(s, c[i]);
    }
    return s;
}

std::string ltrim(std::string s, const char c)
{
    size_t start = s.find_first_not_of(c);
    // return (start == std::string::npos) ? "" : s.substr(start);
    return s.substr(start);
}

#endif
