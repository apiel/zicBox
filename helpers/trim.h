/** Description:
This header file functions as a specialized utility for "trimming" or cleaning up text strings by removing specific unwanted characters from their edges—either the beginning (left side) or the end (right side).

### Purpose and Core Functionality

The primary goal is to provide a comprehensive text cleaning toolkit usable in different programming contexts. It contains specialized functions tailored to handle two common forms of text data: older, raw memory strings (C-style), and modern, robust string objects (C++ standard strings).

### Handling Raw Text (C-Style)

For raw text buffers, the functions modify the data directly or adjust where the program looks for the start of the text.

1.  **Counting:** Separate helper functions first count exactly how many times a designated character appears consecutively at the start or end of the string.
2.  **Left Trim:** This function doesn't change the actual characters; instead, it provides a new starting point, effectively skipping over any unwanted leading characters.
3.  **Right Trim:** This function physically terminates the string just before the first unwanted trailing character, thereby permanently cutting off the excess characters from the end.

### Handling Modern Strings (C++ Style)

For standard C++ string objects, the methods are safer and more flexible. Instead of altering the original string, the trim functions analyze the string and return a brand new, clean version. These functions can handle the removal of a single character (like a space) or efficiently process a whole list of unwanted characters from the edges of the text.

In essence, this file provides necessary logic for sanitizing data by ensuring text fields do not contain superfluous characters (like extra spaces or tabs) at their margins, making data processing more reliable.

sha: 097e97bf1e459ce84f34eeb37c7478c27443c71d42692df7aa8f3cc722fc1d03 
*/
#ifndef _TRIMCHAR_H_
#define _TRIMCHAR_H_

#include <cstdint>
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
