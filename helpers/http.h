/** Description:
This C/C++ header defines a utility function specifically designed to process data received over the internet, typically from web forms or URL query strings.

### Function Overview

The core purpose of this code is to act as a **data translator**. It takes a single, long string of text that is formatted in a standard way known as "Form URL Encoding" and converts it into a highly organized, usable structure.

Imagine this function receiving a continuous stream of data like: `item1=valueA&item2=valueB&item3=valueC`.

### How it Works (The Translation Process)

1.  **Input Reading:** The function treats the entire input string as a stream of text to be processed.
2.  **Pair Separation:** It first scans the input and separates the data into distinct pairs wherever it encounters the ampersand character (`&`). This splits the string into individual `key=value` segments.
3.  **Key-Value Splitting:** For each segment, it searches for the equals sign (`=`). Everything before the equals sign is identified as the **Key** (the name of the data field), and everything after it is identified as the **Value** (the content submitted).
4.  **Storage:** The function stores these extracted Key and Value pairs into an easy-to-access internal list, similar to a digital dictionary. If a segment is missing the equals sign, it is ignored to prevent errors.

### The Output

The result is a structured list of data where you can quickly look up any field name (Key) and immediately retrieve the associated submitted content (Value). This structured format is essential for any program that needs to reliably read and act upon user-submitted web data.

sha: cda60c899ad4553cd577374878248cb210ea4889889fe4a76f46c7e9511836f4 
*/
#pragma once

#include <sstream>
#include <map>

std::map<std::string, std::string> parseFormUrlEncoded(const std::string &body) {
    std::map<std::string, std::string> result;
    std::istringstream ss(body);
    std::string pair;

    while (std::getline(ss, pair, '&')) {
        auto pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string val = pair.substr(pos + 1);
            result[key] = val;
        }
    }
    return result;
}