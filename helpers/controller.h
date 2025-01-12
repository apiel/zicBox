#ifndef _HELPER_CONTROLLER_H_
#define _HELPER_CONTROLLER_H_

#include <cstdint>
#include <string>

#include "log.h"

uint8_t getKeyCode(std::string keyStr)
{
    if (keyStr[0] == '\'' && keyStr[2] == '\'') {
        // A is 4 and Z is 29
        if (keyStr[1] >= 97 && keyStr[1] <= 123) {
            return keyStr[1] - 97 + 4;
        }

        // a is 4 and z is 29
        if (keyStr[1] >= 65 && keyStr[1] <= 90) {
            return keyStr[1] - 65 + 4;
        }

        // 0 is 30 and 9 is 39
        if (keyStr[1] >= 48 && keyStr[1] <= 57) {
            return keyStr[1] - 48 + 29;
        }

        logWarn("Unsupported key: " + keyStr + ". Supported keys: a-z, A-Z, 0-9. For other special char, use scancode values.");
        return -1; // 255
    }

    return atoi(keyStr.c_str());
}

#endif
