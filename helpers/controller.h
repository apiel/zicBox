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
        return 229;
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

        // 1 is 30 and 9 is 38
        if (keyStr[1] >= 48 && keyStr[1] <= 57) {
            return keyStr[1] - 48 + 29;
        }

        // 0 is 39
        if (keyStr[1] == '0') {
            return 39;
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
