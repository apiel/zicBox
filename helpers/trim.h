#ifndef _TRIMCHAR_H_
#define _TRIMCHAR_H_

#include <string>
#include <cstring>

uint16_t countLeadingChar(char* str, char c)
{
    uint16_t count = 0;
    while (str[0] == c) {
        count++;
        str++;
    }
    return count;
}

char* ltrim(char* str, char c)
{
    uint16_t count = countLeadingChar(str, c);
    return str + count;
}

uint16_t countTrailingChar(char* str, char c)
{
    uint16_t count = 0;
    char * end = str + strlen(str);
    while (end[-1] == c) {
        count++;
        end--;
    }
    return count;
}

char * rtrim(char* str, char c)
{
    uint16_t count = countTrailingChar(str, c);
    str[strlen(str) - count] = '\0';
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
