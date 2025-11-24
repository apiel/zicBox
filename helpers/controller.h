/** Description:
This C/C++ header file acts as a universal dictionary or translation tool for keyboard inputs.

**Purpose and Mechanism**

The core functionality is provided by a function designed to map keyboard key names—such as "shift," "enter," or "F1"—to standardized numerical identifiers. Computers and operating systems rely on these specific numbers, rather than the text names, to recognize and process keyboard events.

When a program needs to know which key to use (for example, in a script simulating key presses), it feeds the text name into this function. The function then checks the input against a comprehensive list of known keys.

**Key Translation Scope**

The translation covers a wide array of standard keys:
1.  **Modifier Keys:** Such as "shift," "enter," and "alt."
2.  **Function Keys:** All keys from F1 through F12.
3.  **Navigation Keys:** The "up," "down," "left," and "right" arrows.
4.  **Standard Characters:** It handles letters (A-Z) and numbers (0-9) by calculating their code based on their position relative to the start of the alphabet/number line.

**Error Handling**

If a key name is provided but cannot be found in the known list, the code will log a warning message to alert the user. It also attempts to check if the user accidentally provided a number instead of a key name, trying to interpret that number as the required key code directly. This ensures that the program has a robust way to convert human language instructions into the specific digital codes required by the system.

sha: 3eb4cfd3510ebeefac7e5fd336416140290564eb009806ff893c3ec57e6811d6 
*/
#pragma once

#include <cstdint>
#include <string>

#include "log.h"

uint8_t getKeyCode(std::string keyStr)
{
    if (keyStr == "shift") {
        return 229;
    }
    if (keyStr == "enter") {
        return 40;
    }
    if (keyStr == "space") {
        return 44;
    }
    if (keyStr == "altleft") {
        return 226;
    }
    if (keyStr == "altright") {
        return 230;
    }
    if (keyStr == "F1") {
        return 58;
    }
    if (keyStr == "F2") {
        return 59;
    }
    if (keyStr == "F3") {
        return 60;
    }
    if (keyStr == "F4") {
        return 61;
    }
    if (keyStr == "F5") {
        return 62;
    }
    if (keyStr == "F6") {
        return 63;
    }
    if (keyStr == "F7") {
        return 64;
    }
    if (keyStr == "F8") {
        return 65;
    }
    if (keyStr == "F9") {
        return 66;
    }
    if (keyStr == "F10") {
        return 67;
    }
    if (keyStr == "F11") {
        return 68;
    }
    if (keyStr == "F12") {
        return 69;
    }
    if (keyStr == "up") {
        return 82;
    }
    if (keyStr == "down") {
        return 81;
    }
    if (keyStr == "left") {
        return 80;
    }
    if (keyStr == "right") {
        return 79;
    }
    if (keyStr[0] == '\'' && keyStr[2] == '\'') {
        // A is 4 and Z is 29
        if (keyStr[1] >= 97 && keyStr[1] <= 123) {
            return keyStr[1] - 97 + 4;
        }

        // a is 4 and z is 29
        if (keyStr[1] >= 65 && keyStr[1] <= 90) {
            return keyStr[1] - 65 + 4;
        }

        // 0 is 39
        if (keyStr[1] == '0') {
            return 39;
        }

        if (keyStr[1] >= 49 && keyStr[1] <= 57) {
            return keyStr[1] - 49 + 30;
        }

        // - is 45
        if (keyStr[1] == '-') {
            return 45;
        }

        // = is 46
        if (keyStr[1] == '=') {
            return 46;
        }

        // [ is 47
        if (keyStr[1] == '[') {
            return 47;
        }

        // ] is 48
        if (keyStr[1] == ']') {
            return 48;
        }

        // \ is 49
        if (keyStr[1] == '\\') {
            return 49;
        }

        // ; is 51
        if (keyStr[1] == ';') {
            return 51;
        }

        // ' is 52
        if (keyStr[1] == '\'') {
            return 52;
        }

        logWarn("Unsupported key: " + keyStr + ". Supported keys: a-z, A-Z, 0-9. and - = [ ] \\ ; ' For other special char, use scancode values.");
        return -1; // 255
    }

    return atoi(keyStr.c_str());
}
