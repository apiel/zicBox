/** Description:
This file is a fundamental **utility toolkit** designed for quick and efficient modification of text data, commonly referred to as "strings" in programming. As a header file, it serves as a library of specialized functions that other programs can easily access and use.

The primary goal of these utilities is to perform "in-place" text changes, meaning they modify the original data directly rather than creating new copies.

### Core Functionality Breakdown

1.  **Removing Specific Characters:**
    This function processes a piece of text and completely removes all instances of a single, specified, unwanted character. It achieves this by scanning the text and rewriting it, omitting the targeted character as it goes.

2.  **Replacing Characters:**
    This tool systematically scans the text from start to finish. If it finds a designated "search" character, it immediately swaps it out for a new, specified "replacement" character.

3.  **Removing File Extensions:**
    This highly specialized utility is often used for cleaning up filenames. It searches for the last period (`.`) in the text (which typically marks the start of a file extension like `.txt` or `.jpg`). When found, it cuts off everything from that last period onward, leaving only the base filename.

In summary, this toolkit provides essential functions necessary for cleaning, sanitizing, and standardizing text inputs, making data manipulation simpler and faster for developers.

sha: ff90b0e7f4b7cc98b5a9df84a58ddd29deddf3a8454ae0194966c3d800bf198c 
*/
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